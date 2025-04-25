#include "XMLog.h"

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/msvc_sink.h>


#include <filesystem>
#include <comutil.h>

#pragma comment(lib, "comsuppw.lib")

std::string g_logName("mlog");
std::string g_logFilePath = "";
std::string g_currWorkDir = "";

MLog::MLog()
{
    {
        TCHAR modulePath[1024];
        GetModuleFileName(nullptr, modulePath, 1024);
        std::string mdPath2Str = _com_util::ConvertBSTRToString(modulePath);

        g_currWorkDir = std::string(mdPath2Str, 0, mdPath2Str.find_last_of("\\") + 1);
        printf("currWorkDir = %s \n", g_currWorkDir.c_str());

        if (mdPath2Str.empty())
        {
            g_currWorkDir = std::filesystem::path().string();
            g_logFilePath = g_currWorkDir + _ES_("/logs/_log.txt");
        }
        else
        {
            mdPath2Str.resize(mdPath2Str.size() - 4);
            g_logFilePath = mdPath2Str + _ES_("_log.txt");
        }
    }

    auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(g_logFilePath, true);
    file_sink->set_level(spdlog::level::trace);
    file_sink->set_pattern("%^[%H:%M:%S.%e][%t] %v %$");

    auto msvc_sink = std::make_shared<spdlog::sinks::msvc_sink_mt>(false);
    msvc_sink->set_level(spdlog::level::trace);
    msvc_sink->set_pattern("%v %$");

    spdlog::sinks_init_list sinks = {};
    if (std::filesystem::exists(g_currWorkDir + "/Dbg.Co"))
    {
        spdlog::sinks_init_list sinks = { file_sink, msvc_sink };
    }
    logger_ = std::make_shared<spdlog::logger>(g_logName, sinks);
    logger_->set_level(spdlog::level::trace);
    logger_->flush_on(spdlog::level::info);
    spdlog::register_logger(logger_);
    spdlog::flush_every(std::chrono::seconds(3));
}

MLog::~MLog()
{
    if (std::filesystem::exists(g_currWorkDir + "/Dbg.Co"))
    {
        FreeConsole();
    }
    spdlog::drop_all();
}

MLog* MLog::Instance()
{
    static MLog mlog;
    return &mlog;
}

std::shared_ptr<spdlog::logger> MLog::GetLogger()
{
    return logger_;
}

void MLog::ResetLogger(spdlog_target st, const std::string& logPath)
{
    spdlog::drop(g_logName);
    st_                       = st;
    std::string actualLogPath = (logPath.empty()) ? g_logFilePath : logPath;
    if (std::filesystem::exists(g_currWorkDir + "/Dbg.Co"))
    {
        AllocConsole();
    }
    else
    {
        return;
    }

    if (spdlog_target::FILE == st)
    {
        logger_ = spdlog::basic_logger_mt(g_logName, actualLogPath, true);
        logger_->flush_on(spdlog::level::warn);
        spdlog::register_logger(logger_);
        spdlog::flush_every(std::chrono::seconds(3));
    }
    else if (spdlog_target::CONSOLE == st)
    {
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        console_sink->set_level(spdlog::level::trace);
        console_sink->set_pattern("%^[%H:%M:%S.%e][%t] %v %$");

        auto msvc_sink = std::make_shared<spdlog::sinks::msvc_sink_mt>(false);
        msvc_sink->set_level(spdlog::level::trace);
        msvc_sink->set_pattern("%v %$");

        spdlog::sinks_init_list sinks = { console_sink, msvc_sink };

        logger_ = std::make_shared<spdlog::logger>(g_logName, sinks);
        logger_->set_level(spdlog::level::trace);
        //logger_->flush_on(spdlog::level::info);
        spdlog::register_logger(logger_);
        spdlog::set_default_logger(logger_);
    }
    else if (spdlog_target::CONSOLE_FILE == st)
    {
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        console_sink->set_level(spdlog::level::trace);
        console_sink->set_pattern("%^[%H:%M:%S.%e][%t] %v %$");

        auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(actualLogPath, true);
        file_sink->set_level(spdlog::level::trace);
        file_sink->set_pattern("%^[%H:%M:%S.%e][%t] %v %$");

        spdlog::sinks_init_list sinks = { console_sink, file_sink };

        logger_ = std::make_shared<spdlog::logger>(g_logName, sinks);
        spdlog::set_default_logger(logger_);
        logger_->set_level(spdlog::level::trace);
        logger_->flush_on(spdlog::level::info);
        spdlog::register_logger(logger_);
        spdlog::flush_every(std::chrono::seconds(3));
    }
    else if (spdlog_target::CONSOLE_FILE_MSVC == st)
    {
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        console_sink->set_level(spdlog::level::trace);
        console_sink->set_pattern("%^[%H:%M:%S.%e][%t] %v %$");

        auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(actualLogPath, true);
        file_sink->set_level(spdlog::level::trace);
        file_sink->set_pattern("%^[%H:%M:%S.%e][%t] %v %$");

        auto msvc_sink = std::make_shared<spdlog::sinks::msvc_sink_mt>(false);
        msvc_sink->set_level(spdlog::level::trace);
        msvc_sink->set_pattern("%v %$");

        spdlog::sinks_init_list sinks = { console_sink, file_sink, msvc_sink };

        logger_ = std::make_shared<spdlog::logger>(g_logName, sinks);
        logger_->set_level(spdlog::level::trace);
        logger_->flush_on(spdlog::level::info);
        spdlog::register_logger(logger_);
        spdlog::set_default_logger(logger_);
        spdlog::flush_every(std::chrono::seconds(3));
    }
    else if (spdlog_target::MSVC == st)
    {
        auto msvc_sink = std::make_shared<spdlog::sinks::msvc_sink_mt>(false);
        msvc_sink->set_level(spdlog::level::trace);
        msvc_sink->set_pattern("%v %$");
        logger_ = std::make_shared<spdlog::logger>(g_logName, msvc_sink);

        logger_->set_level(spdlog::level::trace);
    }
    else if (spdlog_target::MSVC_FILE == st)
    {
        auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(actualLogPath, true);
        file_sink->set_level(spdlog::level::trace);
        file_sink->set_pattern("%^[%H:%M:%S.%e][%t] %v %$");

        auto msvc_sink = std::make_shared<spdlog::sinks::msvc_sink_mt>(false);
        msvc_sink->set_level(spdlog::level::trace);
        msvc_sink->set_pattern("%v %$");

        spdlog::sinks_init_list sinks = { file_sink, msvc_sink };

        logger_ = std::make_shared<spdlog::logger>(g_logName, sinks);
        logger_->set_level(spdlog::level::trace);
        //logger_->flush_on(spdlog::level::info);
        spdlog::register_logger(logger_);
        spdlog::flush_every(std::chrono::seconds(3));
    }
}

void MLog::FreeLogger()
{
    if (std::filesystem::exists(g_currWorkDir + "/Dbg.Co"))
    {
        FreeConsole();
    }
    spdlog::drop_all();
}
