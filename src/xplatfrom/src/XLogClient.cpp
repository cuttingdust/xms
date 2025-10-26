#include "XLogClient.h"
#include "XTools.h"

#include <fstream>

#define LOG_LIST_MAX 1000

namespace xms
{
    void XLog(xmsg::XLogLevel level, std::string msg, const char *filename, int line)
    {
        xmsg::XAddLogReq req;
        req.set_log_level(level);
        req.set_log_txt(msg);
        req.set_filename(filename);
        req.set_line(line);
        XLogClient::get()->addLog(&req);
    }
} // namespace xms

class XLogClient::PImpl
{
public:
    PImpl(XLogClient *owenr);
    ~PImpl() = default;

public:
    XLogClient                 *owenr_     = nullptr;
    xmsg::XLogLevel             log_level_ = xmsg::XLOG_INFO;
    std::ofstream               log_ofs_;
    std::mutex                  logs_mutex_;
    std::list<xmsg::XAddLogReq> logs_;

    bool is_print_ = true; ///<  是否输出屏幕
};

XLogClient::PImpl::PImpl(XLogClient *owenr) : owenr_(owenr)
{
}

auto XLogClient::get() -> XLogClient *
{
    static XLogClient xLog;
    return &xLog;
}

XLogClient::XLogClient()
{
    impl_ = std::make_unique<XLogClient::PImpl>(this);
}

XLogClient::~XLogClient() = default;

auto XLogClient::addLog(const xmsg::XAddLogReq *req) -> void
{
    if (!req)
    {
        return;
    }

    if (req->log_level() < impl_->log_level_)
    {
        return;
    }
    std::string level_str = "Debug";
    switch (req->log_level())
    {
        case xmsg::XLOG_DEBUG:
            level_str = "DEBUG";
            break;
        case xmsg::XLOG_INFO:
            level_str = "INFO";
            break;
        case xmsg::XLOG_ERROR:
            level_str = "ERROR";
            break;
        case xmsg::XLOG_FATAL:
            level_str = "FATAL";
            break;
        case xmsg::XLogLevel_INT_MIN_SENTINEL_DO_NOT_USE_:
            break;
        case xmsg::XLogLevel_INT_MAX_SENTINEL_DO_NOT_USE_:
            break;
        default:
            break;
    }

    std::string       log_time = XTools::XGetTime(0, "%F %T");
    std::stringstream log_text;
    log_text << "=========================================================\n";
    log_text << log_time << " " << level_str << "|" << req->filename() << ":" << req->line() << "\n";
    log_text << req->log_txt() << "\n";
    std::cout << log_text.str() << std::endl;

    if (impl_->is_print_)
    {
        //std::cout << "-------------------------------------------------------------" << endl;
        //std::cout << req->DebugString();
        std::cout << log_text.str() << std::endl;
    }

    //////////////////////////////////////////////////////////////////


    // if (impl_->log_ofs_)
    // {
    //     impl_->log_ofs_.write(log_text.str().c_str(), log_text.str().size());
    // }

    if (impl_->log_ofs_)
    {
        impl_->log_ofs_ << "==========================================\n";
        impl_->log_ofs_ << req->DebugString();
    }


    xmsg::XAddLogReq tmp = *req;
    if (tmp.log_time() <= 0)
    {
        tmp.set_log_time(time(0));
    }
    tmp.set_service_port(getServerPort());
    tmp.set_service_name(getServiceName());

    impl_->logs_mutex_.lock();
    if (impl_->logs_.size() > LOG_LIST_MAX)
    {
        impl_->logs_.pop_front();
        std::cout << "-";
    }
    impl_->logs_.push_back(tmp);
    impl_->logs_mutex_.unlock();
}

auto XLogClient::setLogLevel(const xmsg::XLogLevel &log_level) -> void
{
    impl_->log_level_ = log_level;
}

auto XLogClient::setOutFile(const std::string &filePath) -> void
{
    impl_->log_ofs_.open(filePath);
}

auto XLogClient::timerCB() -> void
{
    /// 任务过多，要考虑是否会阻塞其他任务事件
    /// 客户端是独立的线程池，不影响其他业务

    for (;;)
    {
        xmsg::XAddLogReq log;
        {
            XMutex mutex(&impl_->logs_mutex_);
            if (impl_->logs_.empty())
            {
                return;
            }

            log = impl_->logs_.front();
            impl_->logs_.pop_front();
        }
        sendMsg(xmsg::MT_ADD_LOG_REQ, &log);
    }
}

auto XLogClient::startLog() -> bool
{
    if (strlen(this->getServerIP()) == 0)
    {
        this->setServerIP("127.0.0.1");
    }

    if (this->getServerPort() <= 0)
    {
        this->setServerPort(XLOG_PORT);
    }

    setAutoConnect(true);
    setTimeMs(100);

    startConnect();
    return true;
}

auto XLogClient::setPrint(bool is_out) -> void
{
    impl_->is_print_ = is_out;
}
