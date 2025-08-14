#include "LXMysql.h"

#include <format>
#include <mysql.h>
#include <iostream>
#include <sstream>
#include <fstream>
#ifdef _WIN32
#include <windows.h>
#else
#include <iconv.h>
#endif

//////////////////////////////////工具函数///////////////////////////////////
auto join(const std::vector<std::string> &strings, const std::string &delimiter) -> std::string
{
    std::ostringstream oss;

    for (size_t i = 0; i < strings.size(); ++i)
    {
        oss << strings[i];
        if (i < strings.size() - 1)
        { /// 在元素之间添加分隔符
            oss << delimiter;
        }
    }

    return oss.str(); /// 返回连接后的字符串
}

#ifndef _WIN32
static size_t convert(char *from_cha, char *to_cha, char *in, size_t inlen, char *out, size_t outlen)
{
    /// 转换上下文
    iconv_t cd;
    cd = iconv_open(to_cha, from_cha);
    if (cd == 0)
        return -1;
    memset(out, 0, outlen);
    char **pin  = &in;
    char **pout = &out;
    // std::cout << "in = " << in << std::endl;
    // std::cout << "inlen = " << inlen << std::endl;
    // std::cout << "outlen = " << outlen << std::endl;
    //返回转换字节数的数量，但是转GBK时经常不正确 >=0就成功
    size_t re = iconv(cd, pin, &inlen, pout, &outlen);
    iconv_close(cd);
    // std::cout << "result = " << (int)result << std::endl;
    return re;
}
#endif
/////////////////////////////////////////////////////////////////////////////

class LXMysql::PImpl
{
public:
    PImpl(LXMysql *owenr);
    ~PImpl() = default;

public:
    LXMysql   *owenr_  = nullptr;
    MYSQL     *mysql_  = nullptr;
    MYSQL_RES *result_ = nullptr;
};

LXMysql::PImpl::PImpl(LXMysql *owenr) : owenr_(owenr)
{
}

LXData::LXData()
{
}

LXData::LXData(const char *data)
{
    if (!data)
        return;
    this->data = data;
    this->size = static_cast<unsigned long>(strlen(data));
    this->type = LX_DATA_TYPE::LXD_TYPE_STRING;
}

LXData::LXData(const char *data, int size, const LX_DATA_TYPE &type)
{
    if (!data || size <= 0)
        return;
    this->data = data;
    this->size = size;
    this->type = type;
}

LXData::LXData(const int *d)
{
    if (!d)
        return;
    this->type = LX_DATA_TYPE::LXD_TYPE_LONG;
    this->data = reinterpret_cast<const char *>(d);
    this->size = sizeof(int);
}

auto LXData::loadFile(const char *fileName) -> bool
{
    if (!fileName)
        return false;
    std::fstream in(fileName, std::ios::in | std::ios::binary);
    if (!in.is_open())
    {
        std::cerr << "LoadFile " << fileName << " failed!" << std::endl;
        return false;
    }
    /// 文件大小
    in.seekg(0, std::ios::end);
    size = in.tellg();
    in.seekg(0, std::ios::beg);
    if (size <= 0)
    {
        return false;
    }
    data       = new char[size];
    int readed = 0;
    while (!in.eof())
    {
        in.read(const_cast<char *>(data) + readed, size - readed);
        if (in.gcount() > 0)
            readed += in.gcount();
        else
            break;
    }
    in.close();
    this->type = LXD_TYPE_BLOB;
    return true;
}

auto LXData::saveFile(const char *fileName) -> bool
{
    if (!data || size <= 0)
        return false;

    if (!fileName)
        return false;
    std::fstream out(fileName, std::ios::out | std::ios::binary);
    if (!out.is_open())
    {
        std::cerr << "SaveFile " << fileName << " failed!" << std::endl;
        return false;
    }
    out.write(data, size);
    out.close();
    return true;
}

auto LXData::gbkToUtf8() const -> std::string
{
    std::string result = "";
#ifdef _WIN32
    /// GBK转unicode

    /// 1.1 统计转换后字节数
    int len = MultiByteToWideChar(CP_ACP, /// 转换的格式
                                  0,      /// 默认的转换方式
                                  data,   /// 输入的字节
                                  -1,     /// 输入的字符串大小 -1 找\0
                                  0,      /// 输出
                                  0       /// 输出的空间大小
    );
    if (len <= 0)
        return result;
    std::wstring udata;
    udata.resize(len);
    MultiByteToWideChar(CP_ACP, 0, data, -1, (wchar_t *)udata.data(), len);

    /// 2 unicode 转utf-8
    len = WideCharToMultiByte(CP_UTF8, 0, (wchar_t *)udata.data(), -1, 0, 0,
                              0, /// 失败默认替代字符
                              0  /// s是否使用默认替代
    );
    if (len <= 0)
        return result;
    result.resize(len);
    WideCharToMultiByte(CP_UTF8, 0, (wchar_t *)udata.data(), -1, (char *)result.data(), len, 0, 0);
#else
    result.resize(1024);
    int inlen = strlen(data);
    convert((char *)"gbk", (char *)"utf-8", (char *)data, inlen, (char *)result.data(), result.size());
    int outlen = strlen(result.data());
    // std::cout << "outlen = " << outlen << std::endl;
    result.resize(outlen);
#endif
    return result;
}

auto LXData::utf8ToGbk() const -> std::string
{
    std::string result = "";
#ifdef _WIN32
    /// 1 UFT8 转为unicode win utf16

    /// 1.1 统计转换后字节数
    int len = MultiByteToWideChar(CP_UTF8, /// 转换的格式
                                  0,       /// 默认的转换方式
                                  data,    /// 输入的字节
                                  -1,      /// 输入的字符串大小 -1 找\0
                                  0,       /// 输出
                                  0        /// 输出的空间大小
    );
    if (len <= 0)
        return result;
    std::wstring udata;
    udata.resize(len);
    MultiByteToWideChar(CP_UTF8, 0, data, -1, (wchar_t *)udata.data(), len);

    /// 2 unicode 转GBK
    len = WideCharToMultiByte(CP_ACP, 0, (wchar_t *)udata.data(), -1, 0, 0,
                              0, /// 失败默认替代字符
                              0  /// s是否使用默认替代
    );
    if (len <= 0)
        return result;
    result.resize(len);
    WideCharToMultiByte(CP_ACP, 0, (wchar_t *)udata.data(), -1, (char *)result.data(), len, 0, 0);
#else
    result.resize(1024);
    int inlen = strlen(data);
    // std::cout << "inlen=" << inlen << std::endl;
    convert((char *)"utf-8", (char *)"gbk", (char *)data, inlen, (char *)result.data(), result.size());
    int outlen = strlen(result.data());
    //std::cout << "outlen = " << outlen << std::endl;
    result.resize(outlen);
#endif
    return result;
}

auto LXData::drop() -> void
{
    delete data;
    data = nullptr;
}

LXMysql::LXMysql()
{
    std::cout << "LXMysql::LXMysql()" << std::endl;
    impl_ = std::make_unique<PImpl>(this);
}

LXMysql::~LXMysql()
{
    std::cout << "LXMysql::~LXMysql()" << std::endl;
}

auto LXMysql::init() -> bool
{
    close();
    std::cout << "LXMysql::init()" << std::endl;
    /// 新创建一个MYSQL 对象
    impl_->mysql_ = mysql_init(nullptr);
    if (!impl_->mysql_)
    {
        std::cerr << "mysql_init failed!" << std::endl;
        return false;
    }
    return true;
}

auto LXMysql::close() -> void
{
    if (impl_->mysql_)
    {
        mysql_close(impl_->mysql_);
        impl_->mysql_ = nullptr;
    }
    std::cout << "LXMysql::close()" << std::endl;
}

auto LXMysql::connect(const char *host, const char *user, const char *pass, const char *db, unsigned short port,
                      unsigned long flag, bool is_check_database) -> bool
{
    if (impl_->mysql_ == nullptr && !init())
    {
        std::cerr << "Mysql connect failed! msyql is not init!" << std::endl;
        return false;
    }
    if (is_check_database)
    {
        if (!mysql_real_connect(impl_->mysql_, host, user, pass, nullptr, port, nullptr, flag))
        {
            std::cerr << "Mysql connect failed!" << mysql_error(impl_->mysql_) << std::endl;
            return false;
        }
        if (!query(std::format("CREATE DATABASE IF NOT EXISTS {};", db).c_str()))
        {
            std::cerr << "Mysql create database failed!" << mysql_error(impl_->mysql_) << std::endl;
            return false;
        }
    }
    else
    {
        if (!mysql_real_connect(impl_->mysql_, host, user, pass, db, port, nullptr, flag))
        {
            std::cerr << "Mysql connect failed!" << mysql_error(impl_->mysql_) << std::endl;
            return false;
        }
    }

    if (!query(std::format("USE {}", db).c_str()))
    {
        std::cerr << "Mysql use database failed!" << mysql_error(impl_->mysql_) << std::endl;
        return true;
    }
    std::cout << "mysql connect success!" << std::endl;
    return true;
}

auto LXMysql::createTable(const std::string &table_name, const XFIELDS &fileds, bool is_check_exist) -> bool
{
    if (!impl_->mysql_)
    {
        std::cerr << "Mysql createTable failed! msyql is not init!!!" << std::endl;
        return false;
    }
    if (table_name.empty() || fileds.empty())
    {
        std::cerr << "Mysql createTable failed! table_name or fileds is empty!!!" << std::endl;
        return false;
    }

    std::string              key_string = "";
    std::vector<std::string> fields;
    for (const auto field : fileds)
    {
        std::string tmp = "`" + field.name + "`";
        switch (field.type)
        {
            case LX_DATA_TYPE::LXD_TYPE_STRING:
            case LX_DATA_TYPE::LXD_TYPE_VARCHAR:
                {
                    if (field.length > 0)
                    {
                        tmp += " VARCHAR(" + std::to_string(field.length) + ")";
                    }
                    else
                    {
                        tmp += " VARCHAR(" + std::to_string(field.name.size()) + ")";
                    }
                    break;
                }
            case LX_DATA_TYPE::LXD_TYPE_INT24:
                tmp += " INT";
                break;
            case LX_DATA_TYPE::LXD_TYPE_LONG:
                tmp += " INT";
                break;
            case LX_DATA_TYPE::LXD_TYPE_BLOB:
                tmp += " BLOB";
                break;
            default:
                break;
        }

        if (field.is_key)
            key_string = std::format(" PRIMARY KEY (`{}`)", field.name);
        if (field.is_auto_increment)
            tmp += " AUTO_INCREMENT";
        if (field.is_not_null)
            tmp += " NOT NULL";
        fields.emplace_back(tmp);
    }

    /// 根据操作系统设置字符集
    std::string charset;
#ifdef _WIN32
    charset = "gbk";
#else
    charset = "utf8";
#endif

    const std::string &field_str = join(fields, ",");
    const std::string &sql = std::format("CREATE TABLE IF NOT EXISTS `{0}` ({1},{2}) CHARACTER SET {3};", table_name,
                                         field_str, key_string, charset);

    if (!query(sql.c_str()))
    {
        std::cerr << "Mysql createTable failed!" << mysql_error(impl_->mysql_) << std::endl;
        return false;
    }
    return true;
}

auto LXMysql::query(const char *sql, unsigned long sql_len) -> bool
{
    if (!impl_->mysql_)
    {
        std::cerr << "Mysql query failed! msyql is not init!!!" << std::endl;
        return false;
    }
    if (!sql)
    {
        std::cerr << "sql is null!!!" << std::endl;
        return false;
    }
    if (sql_len <= 0)
        sql_len = static_cast<unsigned long>(strlen(sql));

    if (sql_len <= 0)
    {
        std::cerr << "Query sql is empty or wrong format!!!" << std::endl;
        return false;
    }

    int re = mysql_real_query(impl_->mysql_, sql, sql_len);
    if (re != 0)
    {
        std::cerr << "Mysql query failed!!!" << mysql_error(impl_->mysql_) << std::endl;
        return false;
    }
    return true;
}

auto LXMysql::option(const LX_OPT &opt, const void *arg) -> bool
{
    if (!impl_->mysql_)
    {
        std::cerr << "Mysql option failed! msyql is not init!!!" << std::endl;
        return false;
    }
    int re = mysql_options(impl_->mysql_, static_cast<mysql_option>(opt), arg);
    if (re != 0)
    {
        std::cerr << "mysql_options failed!" << mysql_error(impl_->mysql_) << std::endl;
        return false;
    }
    return true;
}

auto LXMysql::setConnectTimeout(unsigned int sec) -> bool
{
    if (!impl_->mysql_)
    {
        std::cerr << "Mysql setConnectTimeout failed! msyql is not init!!!" << std::endl;
        return false;
    }
    if (sec <= 0)
    {
        std::cerr << "timeout is wrong!!!" << std::endl;
        return false;
    }
    return option(LX_OPT_CONNECT_TIMEOUT, &sec);
}

auto LXMysql::setReconnect(bool bRe) -> bool
{
    if (!impl_->mysql_)
    {
        std::cerr << "Mysql setReconnect failed! msyql is not init!!!" << std::endl;
        return false;
    }
    return option(LX_OPT_RECONNECT, &bRe);
}

auto LXMysql::ping() -> bool
{
    if (!impl_->mysql_)
    {
        std::cerr << "Mysql ping failed! msyql is not init!!!" << std::endl;
        return false;
    }
    int re = mysql_ping(impl_->mysql_);
    if (re == 0)
    {
        std::cout << "mysql ping success!" << std::endl;
        return true;
    }
    else
    {
        std::cerr << "mysql ping failed! " << mysql_error(impl_->mysql_) << std::endl;
        return false;
    }
}

auto LXMysql::storeResult() -> bool
{
    if (!impl_->mysql_)
    {
        std::cerr << "Mysql storeResult failed! msyql is not init!!!" << std::endl;
        return false;
    }
    freeResult();
    impl_->result_ = mysql_store_result(impl_->mysql_);
    if (!impl_->result_)
    {
        std::cerr << "mysql_store_result failed!" << mysql_error(impl_->mysql_) << std::endl;
        return false;
    }
    return true;
}

auto LXMysql::useResult() -> bool
{
    if (!impl_->mysql_)
    {
        std::cerr << "Mysql useResult failed! msyql is not init!!!" << std::endl;
        return false;
    }
    freeResult();
    impl_->result_ = mysql_use_result(impl_->mysql_);
    if (!impl_->result_)
    {
        std::cerr << "mysql_use_result failed!" << mysql_error(impl_->mysql_) << std::endl;
        return false;
    }
    return true;
}

auto LXMysql::freeResult() -> void
{
    if (impl_->result_)
    {
        mysql_free_result(impl_->result_);
        impl_->result_ = nullptr;
    }
}

auto LXMysql::fetchRow() -> std::vector<LXData>
{
    std::vector<LXData> row;
    if (!impl_->result_)
    {
        // std::cerr << "Mysql fetchRow failed! result is null!!!" << std::endl;
        return row;
    }
    MYSQL_ROW r = mysql_fetch_row(impl_->result_);
    if (!r)
    {
        // std::cerr << "Mysql fetchRow failed! row is null!!!" << std::endl;
        return row;
    }
    unsigned long *lengths = mysql_fetch_lengths(impl_->result_);
    if (!lengths)
    {
        std::cerr << "Mysql fetchRow failed! lengths is null!!!" << std::endl;
        return row;
    }
    MYSQL_FIELD *fields = mysql_fetch_fields(impl_->result_);
    if (!fields)
    {
        std::cerr << "Mysql fetchRow failed! fields is null!!!" << std::endl;
        return row;
    }

    for (int i = 0; i < mysql_num_fields(impl_->result_); ++i)
    {
        int  iSize = lengths[i];
        auto type  = static_cast<LX_DATA_TYPE>(fields[i].type);
        row.emplace_back(r[i], iSize, type);
    }
    return row;
}

auto LXMysql::getInsertSql(const XDATA &kv, const std::string &table_name) -> std::string
{
    std::string sql;
    if (kv.empty() || table_name.empty())
    {
        return sql;
    }

    std::vector<std::string> keys;
    std::vector<std::string> values;
    for (const auto &[key, data] : kv)
    {
        auto tmp = "`" + key + "`";
        keys.emplace_back(tmp);
        if (data.data[0] == '@')
        {
            tmp = data.data;
            tmp = tmp.substr(1, tmp.size() - 1);
        }
        else
        {
            tmp = std::string("'") + data.data + std::string("'");
        }
        values.emplace_back(tmp);
    }

    const std::string &key_str = join(keys, ",");
    const std::string &val_str = join(values, ",");
    sql                        = std::format("INSERT INTO `{0}` ({1}) VALUES ({2});", table_name, key_str, val_str);

    return sql;
}

auto LXMysql::insert(const XDATA &kv, const std::string &table_name) -> bool
{
    if (!impl_->mysql_)
    {
        std::cerr << "Mysql insert failed! msyql is not init!!!" << std::endl;
        return false;
    }

    const std::string &sql = getInsertSql(kv, table_name);
    if (sql.empty())
    {
        std::cerr << "Mysql insert failed! sql is empty!!!" << std::endl;
        return false;
    }
    if (!query(sql.c_str()))
        return false;
    int num = mysql_affected_rows(impl_->mysql_);
    if (num <= 0)
        return false;
    return true;
}

auto LXMysql::insertBin(const XDATA &kv, const std::string &table_name) -> bool
{
    if (!impl_->mysql_)
    {
        std::cerr << "Mysql insertBin failed! msyql is not init!!!" << std::endl;
        return false;
    }

    if (kv.empty() || table_name.empty())
    {
        std::cerr << "Mysql insertBin failed! kv or table_name is empty!!!" << std::endl;
        return false;
    }

    std::vector<std::string> keys;
    std::vector<std::string> values;
    MYSQL_BIND               bind[256] = { 0 };
    int                      i         = 0;
    for (const auto &[key, data] : kv)
    {
        keys.emplace_back("`" + key + "`");
        values.emplace_back("?");
        bind[i].buffer        = const_cast<char *>(data.data);
        bind[i].buffer_length = data.size;
        bind[i].buffer_type   = static_cast<enum_field_types>(data.type);
        ++i;
    }

    const std::string &key_str = join(keys, ",");
    const std::string &val_str = join(values, ",");
    const std::string &sql     = std::format("INSERT INTO `{0}` ({1}) VALUES ({2});", table_name, key_str, val_str);

    /// 预处理SQL语句
    MYSQL_STMT *stmt = mysql_stmt_init(impl_->mysql_);
    if (!stmt)
    {
        std::cerr << "Mysql insertBin failed! mysql_stmt_init failed!!!" << std::endl;
        return false;
    }

    if (mysql_stmt_prepare(stmt, sql.c_str(), static_cast<unsigned long>(sql.size())) != 0)
    {
        mysql_stmt_close(stmt);
        std::cerr << "Mysql insertBin failed! mysql_stmt_prepare failed!!!" << std::endl;
        return false;
    }

    if (mysql_stmt_bind_param(stmt, bind) != 0)
    {
        mysql_stmt_close(stmt);
        std::cerr << "Mysql insertBin failed! mysql_stmt_bind_param failed!!!" << std::endl;
        return false;
    }

    if (mysql_stmt_execute(stmt) != 0)
    {
        mysql_stmt_close(stmt);
        std::cerr << "Mysql insertBin failed! mysql_stmt_execute failed!!!" << std::endl;
        std::cerr << "Mysql insertBin failed: " << mysql_stmt_error(stmt) << std::endl;
        return false;
    }

    mysql_stmt_close(stmt);
    return true;
}

auto LXMysql::getUpdateSql(const XDATA &kv, const std::string &table_name, std::string where) -> std::string
{
    std::string sql;
    if (kv.empty() || table_name.empty())
    {
        return sql;
    }

    std::vector<std::string> sets;
    for (const auto &[key, data] : kv)
    {
        auto tmp = "`" + key + "`";
        tmp += "=";
        tmp += std::string("'") + data.data + std::string("'");
        sets.emplace_back(tmp);
    }

    const std::string &set_str = join(sets, ",");
    sql                        = std::format("UPDATE `{0}` SET {1} WHERE {2};", table_name, set_str, where);

    return sql;
}

auto LXMysql::update(const XDATA &kv, const std::string &table_name, const std::string &where) -> int
{
    if (!impl_->mysql_)
    {
        std::cerr << "Mysql update failed! msyql is not init!!!" << std::endl;
        return -1;
    }

    const std::string &sql = getUpdateSql(kv, table_name, where);
    if (sql.empty())
    {
        std::cerr << "Mysql update failed! sql is empty!!!" << std::endl;
        return -1;
    }
    if (!query(sql.c_str()))
        return -1;
    return mysql_affected_rows(impl_->mysql_);
}

auto LXMysql::updateBin(const XDATA &kv, const std::string &table_name, const std::string &where) -> int
{
    if (!impl_->mysql_)
    {
        std::cerr << "Mysql updateBin failed! msyql is not init!!!" << std::endl;
        return -1;
    }

    if (kv.empty() || table_name.empty())
    {
        std::cerr << "Mysql updateBin failed! kv or table_name is empty!!!" << std::endl;
        return -1;
    }

    std::vector<std::string> sets;
    MYSQL_BIND               bind[256] = { 0 };
    int                      i         = 0;
    for (const auto &[key, data] : kv)
    {
        sets.emplace_back("`" + key + "`=?");
        bind[i].buffer        = const_cast<char *>(data.data);
        bind[i].buffer_length = data.size;
        bind[i].buffer_type   = static_cast<enum_field_types>(data.type);
        ++i;
    }

    const std::string &set_str = join(sets, ",");
    const std::string &sql     = std::format("UPDATE `{0}` SET {1} WHERE {2};", table_name, set_str, where);

    /// 预处理SQL语句
    MYSQL_STMT *stmt = mysql_stmt_init(impl_->mysql_);
    if (!stmt)
    {
        std::cerr << "Mysql updateBin failed! mysql_stmt_init failed!!!" << std::endl;
        return -1;
    }

    if (mysql_stmt_prepare(stmt, sql.c_str(), static_cast<unsigned long>(sql.size())) != 0)
    {
        mysql_stmt_close(stmt);
        std::cerr << "Mysql updateBin failed! mysql_stmt_prepare failed!!!" << std::endl;
        return -1;
    }

    if (mysql_stmt_bind_param(stmt, bind) != 0)
    {
        mysql_stmt_close(stmt);
        std::cerr << "Mysql updateBin failed! mysql_stmt_bind_param failed!!!" << std::endl;
        return -1;
    }

    if (mysql_stmt_execute(stmt) != 0)
    {
        mysql_stmt_close(stmt);
        std::cerr << "Mysql updateBin failed! mysql_stmt_execute failed!!!" << std::endl;
        return -1;
    }

    int count = mysql_stmt_affected_rows(stmt);
    mysql_stmt_close(stmt);
    return count;
}

int LXMysql::updateBin(const XDATA &kv, const std::string &table_name, const std::map<std::string, std::string> &wheres)
{
    if (!impl_->mysql_)
    {
        std::cerr << "Mysql updateBin failed! msyql is not init!!!" << std::endl;
        return -1;
    }

    if (kv.empty() || table_name.empty())
    {
        std::cerr << "Mysql updateBin failed! kv or table_name is empty!!!" << std::endl;
        return -1;
    }

    std::vector<std::string> sets;
    MYSQL_BIND               bind[256] = { 0 };
    int                      i         = 0;
    for (const auto &[key, data] : kv)
    {
        sets.emplace_back("`" + key + "`=?");
        bind[i].buffer        = const_cast<char *>(data.data);
        bind[i].buffer_length = data.size;
        bind[i].buffer_type   = static_cast<enum_field_types>(data.type);
        ++i;
    }

    std::vector<std::string> temps;
    temps.reserve(wheres.size());
    for (const auto &[key, value] : wheres)
    {
        temps.emplace_back(std::format("`{}`='{}'", key, value));
    }
    auto where = join(temps, " AND ");

    const std::string &set_str = join(sets, ",");
    const std::string &sql     = std::format("UPDATE `{0}` SET {1} WHERE {2};", table_name, set_str, where);

    /// 预处理SQL语句
    MYSQL_STMT *stmt = mysql_stmt_init(impl_->mysql_);
    if (!stmt)
    {
        std::cerr << "Mysql updateBin failed! mysql_stmt_init failed!!!" << std::endl;
        return -1;
    }

    if (mysql_stmt_prepare(stmt, sql.c_str(), static_cast<unsigned long>(sql.size())) != 0)
    {
        mysql_stmt_close(stmt);
        std::cerr << "Mysql updateBin failed! mysql_stmt_prepare failed!!!" << std::endl;
        return -1;
    }

    if (mysql_stmt_bind_param(stmt, bind) != 0)
    {
        mysql_stmt_close(stmt);
        std::cerr << "Mysql updateBin failed! mysql_stmt_bind_param failed!!!" << std::endl;
        return -1;
    }

    if (mysql_stmt_execute(stmt) != 0)
    {
        mysql_stmt_close(stmt);
        std::cerr << "Mysql updateBin failed! mysql_stmt_execute failed!!!" << std::endl;
        return -1;
    }

    int count = mysql_stmt_affected_rows(stmt);
    mysql_stmt_close(stmt);
    return count;
}

auto LXMysql::startTransaction() -> bool
{
    /// return query("set autocommit=0");
    if (!impl_->mysql_)
    {
        std::cerr << "Mysql startTransaction failed! msyql is not init!!!" << std::endl;
        return false;
    }
    if (mysql_autocommit(impl_->mysql_, false) != 0)
    {
        std::cerr << "Mysql startTransaction failed! mysql_autocommit failed!!!" << std::endl;
        return false;
    }
    return true;
}

auto LXMysql::commit() -> bool
{
    /// return query("commit");
    if (!impl_->mysql_)
    {
        std::cerr << "Mysql commit failed! msyql is not init!!!" << std::endl;
        return false;
    }
    if (mysql_commit(impl_->mysql_) != 0)
    {
        std::cerr << "Mysql commit failed! mysql_commit failed!!!" << std::endl;
        return false;
    }
    return true;
}

auto LXMysql::rollback() -> bool
{
    /// return query("rollback");
    if (!impl_->mysql_)
    {
        std::cerr << "Mysql rollback failed! msyql is not init!!!" << std::endl;
        return false;
    }
    if (mysql_rollback(impl_->mysql_) != 0)
    {
        std::cerr << "Mysql rollback failed! mysql_rollback failed!!!" << std::endl;
        return false;
    }
    return true;
}

auto LXMysql::stopTransaction() -> bool
{
    /// return query("set autocommit=1");
    if (!impl_->mysql_)
    {
        std::cerr << "Mysql stopTransaction failed! msyql is not init!!!" << std::endl;
        return false;
    }
    if (mysql_autocommit(impl_->mysql_, true) != 0)
    {
        std::cerr << "Mysql stopTransaction failed! mysql_commit failed!!!" << std::endl;
        return false;
    }
    return true;
}

auto LXMysql::getResult(const char *sql) -> XROWS
{
    freeResult();

    XROWS rows;
    if (!impl_->mysql_)
    {
        std::cerr << "Mysql getResult failed! msyql is not init!!!" << std::endl;
        return rows;
    }
    if (!sql)
    {
        std::cerr << "Mysql getResult failed! sql is null!!!" << std::endl;
        return rows;
    }
    if (!query(sql))
    {
        std::cerr << "Mysql getResult failed! query failed!!!" << std::endl;
        return rows;
    }
    if (!storeResult())
    {
        std::cerr << "Mysql getResult failed! storeResult failed!!!" << std::endl;
        return rows;
    }
    for (;;)
    {
        auto row = fetchRow();
        if (row.empty())
            break;
        rows.emplace_back(row);
    }
    return rows;
}

auto LXMysql::getColumns(const char *table_name) -> XCOLUMNS
{
    freeResult();

    XCOLUMNS xcolumns;
    if (!impl_->mysql_)
    {
        std::cerr << "Mysql getResult failed! msyql is not init!!!" << std::endl;
        return xcolumns;
    }
    const std::string &sql  = std::format("SHOW COLUMNS FROM {};", table_name);
    auto               rows = getResult(sql.c_str());
    for (auto row : rows)
    {
        xcolumns.emplace_back(row[0].data);
    }
    return xcolumns;
}

auto LXMysql::getRows(const char *table_name, const char *selectCol, const std::map<std::string, std::string> &wheres,
                      const std::pair<int, int> &limit, const XORDER &order) -> XROWS
{
    XROWS rows;
    if (!table_name || !selectCol)
        return rows;

    std::string sql = std::format("SELECT {} FROM {}", selectCol, table_name);

    if (!wheres.empty())
    {
        sql += " WHERE ";
        std::vector<std::string> temps;
        temps.reserve(wheres.size());
        for (const auto &[key, value] : wheres)
        {
            temps.emplace_back(std::format("`{}`='{}'", key, value));
        }
        sql += join(temps, " AND ");
    }

    auto [order_name, order_value] = order;
    if (!order_name.empty())
    {
        std::string str_order = order_value == LXD_ADESC ? "ASC" : "DESC";
        sql += std::format(" ORDER BY `{}` {}", order_name, str_order);
    }

    auto [start, end] = limit;
    if (start >= 0 && end > 0)
    {
        const auto str_start = std::to_string(start);
        const auto str_end   = std::to_string(end);

        sql += std::format(" LIMIT {}, {};", str_start, str_end);
    }

    return getResult(sql.c_str());
}

auto LXMysql::getRows(const char *table_name, const char *selectCol, const std::pair<std::string, std::string> &where,
                      const std::pair<int, int> &limit, const XORDER &order) -> XROWS
{
    XROWS rows;
    if (!table_name || !selectCol)
        return rows;

    std::string sql   = std::format("SELECT {} FROM {}", selectCol, table_name);
    auto [key, value] = where;
    if (!key.empty() && !value.empty())
    {
        sql += std::format(" WHERE `{}`='{}'", key, value);
    }

    auto [order_name, order_value] = order;
    if (!order_name.empty())
    {
        std::string str_order = order_value == LXD_ADESC ? "ASC" : "DESC";
        sql += std::format(" ORDER BY `{}` {}", order_name, str_order);
    }

    auto [start, end] = limit;
    if (start >= 0 && end > 0)
    {
        const auto str_start = std::to_string(start);
        const auto str_end   = std::to_string(end);

        sql += std::format(" LIMIT {}, {};", str_start, str_end);
    }

    return getResult(sql.c_str());
}

auto LXMysql::getRows(const char *table_name, const std::vector<std::string> &selectCols,
                      const std::pair<std::string, std::string> &where, const std::pair<int, int> &limit,
                      const XORDER &order) -> XROWS
{
    XROWS rows;
    if (!table_name)
        return rows;

    std::string selectCol;
    if (selectCol.empty())
        selectCol = "*";

    selectCol = join(selectCols, ",");


    std::string sql   = std::format("SELECT {} FROM {}", selectCol, table_name);
    auto [key, value] = where;
    if (!key.empty() && !value.empty())
    {
        sql += std::format(" WHERE `{}`='{}'", key, value);
    }

    auto [order_name, order_value] = order;
    if (!order_name.empty())
    {
        std::string str_order = order_value == LXD_ADESC ? "ASC" : "DESC";
        sql += std::format(" ORDER BY `{}` {}", order_name, str_order);
    }

    auto [start, end] = limit;
    if (start >= 0 && end > 0)
    {
        const auto str_start = std::to_string(start - 1);
        const auto str_end   = std::to_string(end);

        sql += std::format(" LIMIT {}, {};", str_start, str_end);
    }

    return getResult(sql.c_str());
}

auto LXMysql::getCount(const char *table_name, const std::pair<std::string, std::string> &where) -> int
{
    if (!table_name)
        return -1;

    std::string sql   = std::format("SELECT COUNT(*) FROM {}", table_name);
    auto [key, value] = where;
    if (!key.empty() && !value.empty())
    {
        sql += std::format(" WHERE `{}`='{}'", key, value);
    }
    auto rows = getResult(sql.c_str());
    if (rows.empty() || !rows[0][0].data)
        return -1;
    return atoi(rows[0][0].data);
}

auto LXMysql::getRemoveSql(const char *table_name, const std::map<std::string, std::string> &wheres) -> std::string
{
    std::string sql;
    if (!table_name)
        return sql;

    sql = std::format("DELETE FROM {}", table_name);

    if (!wheres.empty())
    {
        sql += " WHERE ";
        std::vector<std::string> temps;
        temps.reserve(wheres.size());
        for (const auto &[key, value] : wheres)
        {
            temps.emplace_back(std::format("`{}`='{}'", key, value));
        }
        sql += join(temps, " AND ");
    }

    return sql;
}

auto LXMysql::remove(const char *table_name, const std::map<std::string, std::string> &wheres) -> bool
{
    if (!impl_->mysql_)
    {
        std::cerr << "Mysql insert failed! msyql is not init!!!" << std::endl;
        return false;
    }

    const std::string &sql = getRemoveSql(table_name, wheres);
    if (sql.empty())
    {
        std::cerr << "Mysql insert failed! sql is empty!!!" << std::endl;
        return false;
    }
    if (!query(sql.c_str()))
        return false;
    int num = mysql_affected_rows(impl_->mysql_);
    if (num <= 0)
        return false;
    return true;
}
