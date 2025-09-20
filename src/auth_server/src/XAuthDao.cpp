#include "XAuthDao.h"

#include "XMsgCom.pb.h"

#include <XTools.h>
#include <LXMysql.h>

constexpr auto table_name      = "xms_auth";
constexpr auto col_id          = "id";
constexpr auto col_user_name   = "xms_username";
constexpr auto col_user_passed = "xms_password";
constexpr auto col_user_role   = "xms_rolename";
constexpr auto chart           = "gbk";

constexpr auto table_name_token = "xms_token";
constexpr auto col_token        = "token";
constexpr auto col_expired_time = "expired_time";

static std::mutex auth_mutex;

class XAuthDao::PImpl
{
public:
    PImpl(XAuthDao *owenr);
    ~PImpl() = default;

public:
    XAuthDao *owenr_ = nullptr;
    LXMysql  *mysql_ = nullptr;
};

XAuthDao::PImpl::PImpl(XAuthDao *owenr) : owenr_(owenr)
{
}

XAuthDao *XAuthDao::get()
{
    static XAuthDao dao;
    return &dao;
}

XAuthDao::~XAuthDao() = default;

auto XAuthDao::init() -> bool
{
    XMutex mux(&auth_mutex);
    if (!impl_->mysql_)
        impl_->mysql_ = new LXMysql();
    if (!impl_->mysql_->init())
    {
        LOGDEBUG("my_->Init() failed!");
        return false;
    }

    impl_->mysql_->setReconnect(true);
    impl_->mysql_->setConnectTimeout(3);

    if (!impl_->mysql_->inputDBConfig())
    {
        LOGDEBUG("my_->inputDBConfig failed!");
        return false;
    }
    LOGDEBUG("my_->inputDBConfig success!");
    return impl_->mysql_->query(std::format("SET NAMES {}", chart).c_str());
}

auto XAuthDao::install() -> bool
{
    LOGDEBUG("XAuthDao::install()");

    XMutex mux(&auth_mutex);
    if (!impl_->mysql_)
    {
        LOGERROR("mysql not init");
        return false;
    }

    XFIELDS fields = {
        { .name              = col_id,
          .type              = LX_DATA_TYPE::LXD_TYPE_INT24,
          .length            = 0,
          .is_key            = true,
          .is_auto_increment = true }, /// id
        { .name      = col_user_name,
          .type      = LX_DATA_TYPE::LXD_TYPE_STRING,
          .length    = 128,
          .is_unique = true },                                                              /// 登录用户名称
        { .name = col_user_passed, .type = LX_DATA_TYPE::LXD_TYPE_STRING, .length = 1024 }, /// 登录用户密码
        { .name = col_user_role, .type = LX_DATA_TYPE::LXD_TYPE_STRING, .length = 128 },    /// 用户角色名称
    };

    if (!impl_->mysql_->createTable(table_name, fields, true))
    {
        LOGINFO("CREATE TABLE xms_auth failed!");
        return false;
    }

    XFIELDS token_fields = {
        { .name              = col_id,
          .type              = LX_DATA_TYPE::LXD_TYPE_INT24,
          .length            = 0,
          .is_key            = true,
          .is_auto_increment = true },                                                    /// id
        { .name = col_user_name, .type = LX_DATA_TYPE::LXD_TYPE_STRING, .length = 1024 }, /// 登录用户名称
        { .name = col_user_role, .type = LX_DATA_TYPE::LXD_TYPE_STRING, .length = 128 },  /// 用户角色名称
        { .name = col_token, .type = LX_DATA_TYPE::LXD_TYPE_STRING, .length = 64 },       /// token
        { .name = col_expired_time, .type = LX_DATA_TYPE::LXD_TYPE_INT24 },               /// 过期时间
    };

    if (!impl_->mysql_->createTable(table_name_token, token_fields, true))
    {
        LOGINFO("CREATE TABLE xms_token failed!");
        return false;
    }

    return true;
}

auto XAuthDao::addUser(xmsg::XAddUserReq *user) -> bool
{
    if (!user || !impl_->mysql_)
        return false;

    XMutex mux(&auth_mutex);
    XDATA  data;
    data[col_user_name]   = user->username().c_str();
    data[col_user_passed] = user->password().c_str();
    data[col_user_role]   = user->rolename().c_str();
    if (!impl_->mysql_->insert(data, table_name))
    {
        return false;
    }
    return true;
}

auto XAuthDao::login(const xmsg::XLoginReq *user_req, xmsg::XLoginRes *user_res, int timeout_sec) -> bool
{
    if (!user_req || !user_res || !impl_->mysql_)
        return false;

    /// 注入攻击
    std::string username = user_req->username();
    std::string password = user_req->password();
    if (username.find(';') != username.npos || username.find('\'') != username.npos)
    {
        return false;
    }


    auto rows = impl_->mysql_->getRows(table_name, { col_user_name, col_user_role },
                                       { { col_user_name, username }, { col_user_passed, password } });
    if (rows.empty())
    {
        user_res->set_restype(xmsg::XLoginRes::XRT_ERR);
        user_res->set_token("username or password error!");
        return false;
    }
    std::string rolename = "";
    if (rows[0][1].data)
        rolename = rows[0][1].data;
    user_res->set_rolename(rolename);
    user_res->set_username(username);

    /// 2 生成token
    XDATA data;
    int   now              = time(0);
    int   expired_time     = now + timeout_sec;
    auto  expired_time_str = std::to_string(expired_time);

    data[col_token]        = "@UUID()";
    data[col_user_name]    = username.c_str();
    data[col_user_role]    = rolename.c_str();
    data[col_expired_time] = expired_time_str.c_str();

    if (!impl_->mysql_->insert(data, table_name_token))
    {
        user_res->set_restype(xmsg::XLoginRes::XRT_ERR);
        user_res->set_token("1 make token error!");
        return false;
    }

    /// 根据id获取token
    int id = impl_->mysql_->getInSqlInId();
    rows   = impl_->mysql_->getRows(table_name_token, col_token, { col_id, std::to_string(id) });
    if (rows.empty())
    {
        user_res->set_restype(xmsg::XLoginRes::XRT_ERR);
        user_res->set_token("2 make token error!!");
        return false;
    }
    std::string token = rows[0][0].data;
    user_res->set_restype(xmsg::XLoginRes::XRT_OK);
    user_res->set_token(token);
    user_res->set_expired_time(expired_time);

    /// 清理过期的登录信息
    impl_->mysql_->remove(table_name_token, { { col_expired_time, std::to_string(now) } }, LXMysql::LX_C_LT);
    return true;
}

XAuthDao::XAuthDao()
{
    impl_ = std::make_unique<PImpl>(this);
}
