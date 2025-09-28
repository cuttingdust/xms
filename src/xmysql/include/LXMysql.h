/**
 * @file   LXMysql.h
 * @brief  
 *
 * @details   
 *
 * @author 31667
 * @date   2024-12-06
 */

#ifndef LXMYSQL_H
#define LXMYSQL_H

#include "LXMysql_Global.h"

#include <map>
#include <memory>
#include <string>
#include <vector>

enum LX_DATA_TYPE
{
    LXD_TYPE_DECIMAL,
    LXD_TYPE_TINY,
    LXD_TYPE_SHORT,
    LXD_TYPE_LONG,
    LXD_TYPE_FLOAT,
    LXD_TYPE_DOUBLE,
    LXD_TYPE_NULL,
    LXD_TYPE_TIMESTAMP,
    LXD_TYPE_LONGLONG,
    LXD_TYPE_INT24,
    LXD_TYPE_DATE,
    LXD_TYPE_TIME,
    LXD_TYPE_DATETIME,
    LXD_TYPE_YEAR,
    LXD_TYPE_NEWDATE, /**< Internal to MySQL. Not used in protocol */
    LXD_TYPE_VARCHAR,
    LXD_TYPE_BIT,
    LXD_TYPE_TIMESTAMP2,
    LXD_TYPE_DATETIME2,   /**< Internal to MySQL. Not used in protocol */
    LXD_TYPE_TIME2,       /**< Internal to MySQL. Not used in protocol */
    LXD_TYPE_TYPED_ARRAY, /**< Used for replication only */
    LXD_TYPE_INVALID     = 243,
    LXD_TYPE_BOOL        = 244, /**< Currently just a placeholder */
    LXD_TYPE_JSON        = 245,
    LXD_TYPE_NEWDECIMAL  = 246,
    LXD_TYPE_ENUM        = 247,
    LXD_TYPE_SET         = 248,
    LXD_TYPE_TINY_BLOB   = 249,
    LXD_TYPE_MEDIUM_BLOB = 250,
    LXD_TYPE_LONG_BLOB   = 251,
    LXD_TYPE_BLOB        = 252,
    LXD_TYPE_VAR_STRING  = 253,
    LXD_TYPE_STRING      = 254,
    LXD_TYPE_GEOMETRY    = 255
};

enum LX_ORDER
{
    LXD_ADESC = 0, /// 升序
    LXD_DESC  = 1, /// 降序
};

struct LXM_EXPORT LXField
{
    std::string                         name;                                ///<<< 名称
    LX_DATA_TYPE                        type              = LXD_TYPE_STRING; ///<<< 类型
    int                                 length            = 0;               ///<<< 长度
    bool                                is_key            = false;           ///<<< 主键约束（PRIMARY KEY）
    bool                                is_auto_increment = false;           ///<<< 自增约束 (AUTO INCREMENT)
    bool                                is_not_null       = false;           ///<<< 非空约束（NOT NULL ）
    bool                                is_unique         = false;           ///<<< 唯一约束（UNIQUE）
    std::string                         default_value;                       ///<<< 默认值约束（DEFAULT ）
    std::string                         comment;                             ///<<< 字段注释
    std::pair<std::string, std::string> foreign;                             ///<<< 外键约束
};

struct LXM_EXPORT LXData
{
public:
    LXData();
    LXData(const char *data); /// 字符串 非二进制
    LXData(const char *data, int size, const LX_DATA_TYPE &type);
    LXData(const int *d);

public:
    const char  *data = nullptr;
    int          size = 0;
    LX_DATA_TYPE type = LX_DATA_TYPE::LXD_TYPE_INVALID;
    auto         loadFile(const char *fileName) -> bool;
    auto         saveFile(const char *fileName) -> bool;
    auto         gbkToUtf8() const -> std::string;
    auto         utf8ToGbk() const -> std::string;
    auto         drop() -> void;
};

using XDATA    = std::map<std::string, LXData>;
using XFIELDS  = std::vector<LXField>;
using XROWS    = std::vector<std::vector<LXData>>;
using XCOLUMNS = std::vector<std::string>;
using XORDER   = std::pair<std::string, LX_ORDER>;

class LXM_EXPORT LXMysql
{
public:
    LXMysql();
    virtual ~LXMysql();
    enum LX_OPT
    {
        LX_OPT_CONNECT_TIMEOUT,
        LX_OPT_COMPRESS,
        LX_OPT_NAMED_PIPE,
        LX_INIT_COMMAND,
        LX_READ_DEFAULT_FILE,
        LX_READ_DEFAULT_GROUP,
        LX_SET_CHARSET_DIR,
        LX_SET_CHARSET_NAME,
        LX_OPT_LOCAL_INFILE,
        LX_OPT_PROTOCOL,
        LX_SHARED_MEMORY_BASE_NAME,
        LX_OPT_READ_TIMEOUT,
        LX_OPT_WRITE_TIMEOUT,
        LX_OPT_USE_RESULT,
        LX_REPORT_DATA_TRUNCATION,
        LX_OPT_RECONNECT,
        LX_PLUGIN_DIR,
        LX_DEFAULT_AUTH,
        LX_OPT_BIND,
        LX_OPT_SSL_KEY,
        LX_OPT_SSL_CERT,
        LX_OPT_SSL_CA,
        LX_OPT_SSL_CAPATH,
        LX_OPT_SSL_CIPHER,
        LX_OPT_SSL_CRL,
        LX_OPT_SSL_CRLPATH,
        LX_OPT_CONNECT_ATTR_RESET,
        LX_OPT_CONNECT_ATTR_ADD,
        LX_OPT_CONNECT_ATTR_DELETE,
        LX_SERVER_PUBLIC_KEY,
        LX_ENABLE_CLEARTEXT_PLUGIN,
        LX_OPT_CAN_HANDLE_EXPIRED_PASSWORDS,
        LX_OPT_MAX_ALLOWED_PACKET,
        LX_OPT_NET_BUFFER_LENGTH,
        LX_OPT_TLS_VERSION,
        LX_OPT_SSL_MODE,
        LX_OPT_GET_SERVER_PUBLIC_KEY,
        LX_OPT_RETRY_COUNT,
        LX_OPT_OPTIONAL_RESULTSET_METADATA,
        LX_OPT_SSL_FIPS_MODE
    };

    enum lX_CONDICTION
    {
        LX_C_EQUAL  = 0, /// =
        LX_C_LIKE   = 1, /// like
        LX_C_IN     = 2, /// in
        LX_C_GT     = 3, /// >
        LX_C_LT     = 4, /// <
        LX_C_GE     = 5, /// >=
        LX_C_LE     = 6, /// <=
        LX_C_NE     = 7, /// <>
        LX_C_IS     = 8, /// is
        LX_C_IS_NOT = 9  /// is not
    };

public:
    /// \brief 初始化数据库
    /// \return
    auto init() -> bool;

    /// \brief 接收用户输入数据库配置
    /// \return
    auto inputDBConfig() -> bool;

    /// \brief 关闭数据库
    auto close() -> void;

    /// \brief 连接数据库
    /// \param host 主机地址
    /// \param user 用户名
    /// \param pass 密码
    /// \param db   数据库
    /// \param port 端口
    /// \param flag 连接标志
    /// \param is_check_database 是否检查数据库 如果为true则不存在则创建
    /// \return 是否连接成功
    auto connect(const char *host, const char *user, const char *pass, const char *db, unsigned short port = 3306,
                 unsigned long flag = 0, bool is_check_database = false) -> bool;

    /// \brief 创建表 (Note: 主键必须设置)
    /// \param table_name   表名
    /// \param fileds       字段
    /// \param is_check_exist 是否检查表是否存在
    /// \return
    auto createTable(const std::string &table_name, const XFIELDS &fileds, bool is_check_exist) -> bool;

    /// \brief 执行sql 语句
    /// \param sql sql 语句
    /// \param sql_len sql 语句长度
    /// \return
    auto query(const char *sql, unsigned long sql_len = 0) -> bool;

    /// \brief Mysql参数的设定
    /// \return
    auto option(const LX_OPT &opt, const void *arg) -> bool;

    /// \brief 设置数据库连接超时时间
    /// \param sec 超时时间
    /// \return 是否设置成功
    auto setConnectTimeout(unsigned int sec) -> bool;

    /// \brief 设置数据库自动重连
    /// \param bRe 是否自动重连
    /// \return 是否设置成功
    auto setReconnect(bool bRe) -> bool;

    /// \brief ping 数据库 测试连接
    /// \return 是否ping成功
    auto ping() -> bool;

    /// \brief 获取结果集
    /// \return
    auto storeResult() -> bool;

    /// \brief 开始接收结果，通过Fetch获取
    /// \return
    auto useResult() -> bool;

    /// \brief 释放结果集占用的空间
    auto freeResult() -> void;

    /// \brief 获取一行数据
    /// \return
    auto fetchRow() -> std::vector<LXData>;

    /// \brief 生成插入sql语句
    /// \param kv <字段名,字段值>
    /// \param table_name 表名
    /// \return sql 语句
    auto getInsertSql(const XDATA &kv, const std::string &table_name) -> std::string;

    /// \brief 插入数据库(非二进制数据)
    /// \param kv <字段名,字段值>
    /// \param table_name 表名
    /// \return 是否插入成功
    auto insert(const XDATA &kv, const std::string &table_name) -> bool;

    /// \brief 插入数据库(二进制数据)
    /// \param kv <字段名， 字段值>
    /// \param table_name 表名
    /// \return 是否插入成功
    auto insertBin(const XDATA &kv, const std::string &table_name) -> bool;

    /// \brief 生成修改Sql语句
    /// \param kv <字段名,字段值>
    /// \param table_name 表名
    /// \param where 修改选中条件
    /// \return 语句
    auto getUpdateSql(const XDATA &kv, const std::string &table_name, std::string where) -> std::string;

    /// \brief 修改数据库(非二进制数据)
    /// \param kv  <字段名,字段值>
    /// \param table_name 表名
    /// \param where 修改选中条件
    /// \return 返回更新数量，失败返回-1
    auto update(const XDATA &kv, const std::string &table_name, const std::string &where) -> int;

    /// \brief 修改数据库(二进制数据)
    /// \param kv  <字段名,字段值>
    /// \param table_name 表名
    /// \param where 修改选中条件
    /// \return 返回更新数量，失败返回-1
    int updateBin(const XDATA &kv, const std::string &table_name, const std::string &where);

    /// \brief 修改数据库(二进制数据)
    /// \param kv  <字段名,字段值>
    /// \param table_name  表名
    /// \param wheres 修改选中条件
    /// \return 返回更新数量，失败返回-1
    int updateBin(const XDATA &kv, const std::string &table_name, const std::map<std::string, std::string> &wheres);

    /// \brief 开启事务
    /// \return
    auto startTransaction() -> bool;

    /// \brief 提交
    /// \return
    auto commit() -> bool;

    /// \brief 回滚
    /// \return
    auto rollback() -> bool;

    /// \brief 关闭事务
    /// \return
    auto stopTransaction() -> bool;

    /// \brief 简易接口,返回select的数据结果，每次调用清理上一次的结果集
    /// \return
    auto getResult(const char *sql) -> XROWS;

    /// \brief获取表的字段
    /// \param table_name
    /// \return
    auto getColumns(const char *table_name) -> XCOLUMNS;

    /// \brief 获取条件数据
    /// \param table_name   表名
    /// \param selectCol    选择的列(单个)
    /// \param wheres       查询条件
    /// \param limit        分页限制
    /// \param order        排序
    /// \return
    auto getRows(const char *table_name, const char *selectCol = "*",
                 const std::map<std::string, std::string> &wheres = {}, const std::pair<int, int> &limit = { 0, 0 },
                 const XORDER &order = { "", LXD_ADESC }) -> XROWS;

    auto getRows(const char *table_name, const char *selectCol = "*",
                 const std::pair<std::string, std::string> &where = { "", "" }, const std::pair<int, int> & = { 0, 0 },
                 const XORDER                              &order = { "", LXD_ADESC }) -> XROWS;

    auto getRows(const char *table_name, const std::vector<std::string> &selectCols,
                 const std::pair<std::string, std::string> &where, const std::pair<int, int> &limit,
                 const XORDER &order) -> XROWS;

    /// \brief
    /// \param table_name
    /// \param selectCols
    /// \param wheres
    /// \param limit
    /// \param order
    /// \return
    auto getRows(const char *table_name, const std::vector<std::string> &selectCols,
                 const std::map<std::string, std::string> &wheres = { "", "" },
                 const std::pair<int, int> &limit = { 0, 0 }, const XORDER &order = { "", LXD_ADESC }) -> XROWS;


    /// \brief 统计数据
    /// \param table_name
    /// \param where
    /// \return
    auto getCount(const char *table_name, const std::pair<std::string, std::string> &where = { "", "" }) -> int;

    auto getRemoveSql(const char *table_name, const std::map<std::string, std::string> &wheres,
                      lX_CONDICTION lc = LX_C_EQUAL) -> std::string;

    /// \brief 删除数据
    /// \param table_name
    /// \param wheres 删除条件
    /// \return
    auto remove(const char *table_name, const std::map<std::string, std::string> &wheres, lX_CONDICTION lc = LX_C_EQUAL)
            -> bool;

    /// \brief 获取上一次插入的ID号
    /// \return
    auto getInSqlInId() -> int;

private:
    class PImpl;
    std::unique_ptr<PImpl> impl_;
};

#endif // LXMYSQL_H
