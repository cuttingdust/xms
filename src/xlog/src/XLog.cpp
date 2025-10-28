#include "XLog.h"

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/msvc_sink.h>
#include <spdlog/details/null_mutex.h>
#include <spdlog/details/synchronous_factory.h>
#include <spdlog/sinks/base_sink.h>
#include <spdlog/spdlog.h>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <span>

namespace
{
    constexpr std::string_view DEFAULT_LOGGER_NAME = "mlog";

    /// 全局变量
    std::string g_logName{ std::string(DEFAULT_LOGGER_NAME) };
    std::string g_logFilePath;
    std::string g_currWorkDir;

    constexpr size_t           DEFAULT_PATH_LENGTH = 1024;
    constexpr std::string_view DEBUG_FILE_NAME     = "DBG.CO";

} // namespace

//////////////////////////////////////////////////////////////////

namespace fs = std::filesystem;

namespace spdlog
{
    namespace sinks
    {
        template <typename Mutex>
            requires std::same_as<Mutex, std::mutex> || std::same_as<Mutex, spdlog::details::null_mutex>
        class unicode_file_sink final : public base_sink<Mutex>
        {
        public:
            explicit unicode_file_sink(const fs::path &filename, bool truncate = false) : file_path_(filename)
            {
                open_file(truncate);
            }

            ~unicode_file_sink()
            {
                close_file();
            }

            [[nodiscard]] const fs::path &filename() const noexcept
            {
                return file_path_;
            }

            void truncate()
            {
                std::lock_guard<Mutex> lock(base_sink<Mutex>::mutex_);
                close_file();
                open_file(true);
            }

        protected:
            void sink_it_(const spdlog::details::log_msg &msg) override
            {
                spdlog::memory_buf_t formatted;
                base_sink<Mutex>::formatter_->format(msg, formatted);

                std::span<const char> data{ formatted.data(), formatted.size() };

                DWORD bytes_written = 0;
                if (!WriteFile(file_handle_, data.data(), static_cast<DWORD>(data.size()), &bytes_written, nullptr))
                {
                    throw spdlog_ex(std::format("Failed to write to file: {}", file_path_.string()));
                }
            }

            void flush_() override
            {
                if (file_handle_ != INVALID_HANDLE_VALUE)
                {
                    FlushFileBuffers(file_handle_);
                }
            }

        private:
            fs::path file_path_;
            HANDLE   file_handle_{ INVALID_HANDLE_VALUE };

            void open_file(bool truncate)
            {
                close_file();

                // Create parent directory if needed
                auto parent_path = file_path_.parent_path();
                if (!parent_path.empty())
                {
                    std::error_code ec;
                    fs::create_directories(parent_path, ec);
                    if (ec)
                    {
                        throw spdlog_ex(std::format("Failed to create directories for: {}", file_path_.string()));
                    }
                }

                // Open file with proper Unicode support
                DWORD access_mode          = GENERIC_WRITE;
                DWORD share_mode           = FILE_SHARE_READ;
                DWORD creation_disposition = truncate ? CREATE_ALWAYS : OPEN_ALWAYS;

                file_handle_ = CreateFileW(file_path_.c_str(), access_mode, share_mode, nullptr, creation_disposition,
                                           FILE_ATTRIBUTE_NORMAL, nullptr);

                if (file_handle_ == INVALID_HANDLE_VALUE)
                {
                    throw spdlog_ex(std::format("Failed to open file: {}", file_path_.string()));
                }

                if (!truncate)
                {
                    SetFilePointer(file_handle_, 0, nullptr, FILE_END);
                }
            }

            void close_file() noexcept
            {
                if (file_handle_ != INVALID_HANDLE_VALUE)
                {
                    CloseHandle(file_handle_);
                    file_handle_ = INVALID_HANDLE_VALUE;
                }
            }
        };

        using unicode_file_sink_mt = unicode_file_sink<std::mutex>;

    } // namespace sinks

    //
    // Factory functions
    //
    template <typename Factory = spdlog::synchronous_factory>
    [[nodiscard]] inline std::shared_ptr<logger> unicode_logger_mt(std::string_view logger_name,
                                                                   const fs::path &filename, bool truncate = false)
    {
        return Factory::template create<sinks::unicode_file_sink_mt>(logger_name, filename, truncate);
    }

} // namespace spdlog


//////////////////////////////////////////////////////////////////


[[nodiscard]]
auto GetOrCreateUnicodeFileSink(const fs::path &logPath)
{
    /// 使用static缓存已创建的sink
    static std::unordered_map<std::string, std::weak_ptr<spdlog::sinks::unicode_file_sink_mt>> existingSinks;
    std::string path_str = logPath.string();

    /// Clean up expired sinks
    for (auto it = existingSinks.begin(); it != existingSinks.end();)
    {
        if (it->second.expired())
        {
            it = existingSinks.erase(it);
        }
        else
        {
            ++it;
        }
    }

    ///  尝试获取现有sink
    if (auto it = existingSinks.find(path_str); it != existingSinks.end())
    {
        if (auto existing = it->second.lock())
        {
            existing->set_pattern("%^[%H:%M:%S.%e][%t] %v %$");
            existing->set_level(spdlog::level::trace);
            return existing;
        }
    }

    auto unicode_file_sink = std::make_shared<spdlog::sinks::unicode_file_sink_mt>(logPath, true);
    unicode_file_sink->set_pattern("%^[%H:%M:%S.%e][%t] %v %$");
    unicode_file_sink->set_level(spdlog::level::trace);

    existingSinks[path_str] = unicode_file_sink;

    return unicode_file_sink;
}

//////////////////////////////////////////////////////////////////


#ifdef _WIN32
#include <windows.h>
std::wstring getExecutablePath()
{
    std::vector<wchar_t> buffer(DEFAULT_PATH_LENGTH);
    DWORD                length = GetModuleFileName(nullptr, buffer.data(), buffer.size());
    if (length > 0 && length < buffer.size())
    {
        return std::wstring(buffer.data(), length);
    }
    return {};
}

#elif __linux__
#include <unistd.h>
#include <limits.h>

std::string getExecutablePath()
{
    char    buffer[DEFAULT_PATH_LENGTH];
    ssize_t length = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
    if (length != -1)
    {
        buffer[length] = '\0';
        return std::string(buffer);
    }
    return {};
}

#elif __APPLE__
#include <mach-o/dyld.h>

std::string getExecutablePath()
{
    char     buffer[1024];
    uint32_t size = sizeof(buffer);
    if (_NSGetExecutablePath(buffer, &size) == 0)
    {
        return std::string(buffer);
    }
    return {};
}

#else
#error "Unsupported platform"
#endif


class XLog::PImpl
{
public:
    PImpl(XLog *owenr);
    ~PImpl();

public:
    XLog    *owenr_  = nullptr;
    SmartLog logger_ = nullptr;

#ifdef _WIN32
    LogTarget currentTarget_ = LogTarget::CONSOLE_FILE_MSVC;
#else
    LogTarget currentTarget_ = LogTarget::CONSOLE_FILE;
#endif
};

XLog::PImpl::PImpl(XLog *owenr) : owenr_(owenr)
{
    ///  设置控制台代码页为UTF-8以正确显示Unicode字符

    setlocale(LC_ALL, "zh_CN.UTF-8");
#ifdef _WIN32
    ::SetConsoleOutputCP(CP_UTF8);
#endif

    fs::path modulePath;

    auto wModulePathStr = getExecutablePath();
    if (!wModulePathStr.empty())
    {
        modulePath = fs::path(wModulePathStr);

        /// 将当前工作目录设置为包含可执行文件的目录
        g_currWorkDir = modulePath.parent_path().string();

        /// 创建日志文件路径，将.exe替换为_log.txt
        if (modulePath.extension() == ".exe")
        {
            fs::path logPath = modulePath.stem(); /// 只获取文件名部分（不含扩展名）
            logPath += "_log.txt";                /// 添加自定义后缀
            g_logFilePath = (modulePath.parent_path() / logPath).string();
        }
    }

    /// 如果模块路径为空，则回退到当前目录
    if (g_currWorkDir.empty())
    {
        g_currWorkDir = fs::current_path().string();
        g_logFilePath = (fs::path(g_currWorkDir) / "logs" / "_log.txt").string();
    }

    /// 确保工作目录路径以斜杠结尾
    if (!g_currWorkDir.empty() && g_currWorkDir.back() != '\\' && g_currWorkDir.back() != '/')
    {
        g_currWorkDir += fs::path::preferred_separator;
    }

    /// Output diagnostic information
    std::cout << std::format("currWorkDir = {}\n", g_currWorkDir);
    std::cout << std::format("logFilePath = {}\n", g_logFilePath);

    /// 检查DBG.CO是否存在
    fs::path dbgFilePath     = fs::path(g_currWorkDir) / std::string(DEBUG_FILE_NAME);
    bool     debugFileExists = fs::exists(dbgFilePath);

    auto file_sink = GetOrCreateUnicodeFileSink(g_logFilePath);
    file_sink->set_level(spdlog::level::trace);
    file_sink->set_pattern("%^[%H:%M:%S.%e][%t] %v %$");

    auto msvc_sink = std::make_shared<spdlog::sinks::msvc_sink_mt>(false);
    msvc_sink->set_level(spdlog::level::trace);
    msvc_sink->set_pattern("%v %$");

    auto sinks =
            debugFileExists ? spdlog::sinks_init_list{ file_sink, msvc_sink } : spdlog::sinks_init_list{ file_sink };

    logger_ = std::make_shared<spdlog::logger>(g_logName, sinks);
    logger_->set_level(spdlog::level::trace);
    logger_->flush_on(spdlog::level::info);
    spdlog::register_logger(logger_);
    spdlog::flush_every(std::chrono::seconds(3));
}

XLog::PImpl::~PImpl()
{
    owenr_->FreeLogger();
}

auto XLog::Instance() -> XLog *
{
    static XLog log;
    return &log;
}

XLog::~XLog() = default;

auto XLog::ResetLogger(LogTarget target, std::string_view logPath) -> void
{
    spdlog::drop(g_logName);
    impl_->currentTarget_ = target;

    /// 使用提供的日志路径或默认路径（如果为空）
    fs::path actualLogPath = logPath.empty() ? fs::path(g_logFilePath) : fs::path(std::string(logPath));

    /// 检查是否通过DBG.CO文件启用了调试模式
    fs::path dbgFilePath     = fs::path(g_currWorkDir) / std::string(DEBUG_FILE_NAME);
    bool     debugFileExists = fs::exists(dbgFilePath);

    /// Setup console if debug file exists
    if (debugFileExists)
    {
        setlocale(LC_ALL, "zh_CN.UTF-8");
#ifdef _WIN32
        ::SetConsoleOutputCP(CP_UTF8);
        ::AllocConsole();
#endif
    }
    else
    {
        return; /// Early return if no debug file
    }

    switch (target)
    {
        case LogTarget::FILE:
            {
                /// 如果需要，创建父目录
                if (!actualLogPath.parent_path().empty())
                {
                    std::error_code ec;
                    fs::create_directories(actualLogPath.parent_path(), ec);
                }

                impl_->logger_ = spdlog::basic_logger_mt(g_logName, actualLogPath.string(), true);
                impl_->logger_->flush_on(spdlog::level::warn);
                spdlog::register_logger(impl_->logger_);
                spdlog::flush_every(std::chrono::seconds(3));
                break;
            }

        case LogTarget::CONSOLE:
            {
                auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
                console_sink->set_level(spdlog::level::trace);
                console_sink->set_pattern("%^[%H:%M:%S.%e][%t] %v %$");

                auto msvc_sink = std::make_shared<spdlog::sinks::msvc_sink_mt>(false);
                msvc_sink->set_level(spdlog::level::trace);
                msvc_sink->set_pattern("%v %$");

                impl_->logger_ =
                        std::make_shared<spdlog::logger>(g_logName, spdlog::sinks_init_list{ console_sink, msvc_sink });
                impl_->logger_->set_level(spdlog::level::trace);

                spdlog::register_logger(impl_->logger_);
                spdlog::set_default_logger(impl_->logger_);
                break;
            }

        case LogTarget::CONSOLE_FILE:
            {
                auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
                console_sink->set_level(spdlog::level::trace);
                console_sink->set_pattern("%^[%H:%M:%S.%e][%t] %v %$");

                auto file_sink = GetOrCreateUnicodeFileSink(actualLogPath);
                file_sink->set_level(spdlog::level::trace);
                file_sink->set_pattern("%^[%H:%M:%S.%e][%t] %v %$");

                impl_->logger_ =
                        std::make_shared<spdlog::logger>(g_logName, spdlog::sinks_init_list{ console_sink, file_sink });
                spdlog::set_default_logger(impl_->logger_);
                impl_->logger_->set_level(spdlog::level::trace);
                impl_->logger_->flush_on(spdlog::level::info);

                spdlog::register_logger(impl_->logger_);
                spdlog::flush_every(std::chrono::seconds(3));
                break;
            }

        case LogTarget::CONSOLE_FILE_MSVC:
            {
                auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
                console_sink->set_level(spdlog::level::trace);
                console_sink->set_pattern("%^[%H:%M:%S.%e][%t] %v %$");

                auto file_sink = GetOrCreateUnicodeFileSink(actualLogPath);

                file_sink->set_level(spdlog::level::trace);
                file_sink->set_pattern("%^[%H:%M:%S.%e][%t] %v %$");

                auto msvc_sink = std::make_shared<spdlog::sinks::msvc_sink_mt>(false);
                msvc_sink->set_level(spdlog::level::trace);
                msvc_sink->set_pattern("%v %$");

                impl_->logger_ = std::make_shared<spdlog::logger>(
                        g_logName, spdlog::sinks_init_list{ console_sink, file_sink, msvc_sink });

                impl_->logger_->set_level(spdlog::level::trace);
                impl_->logger_->flush_on(spdlog::level::info);

                spdlog::register_logger(impl_->logger_);
                spdlog::set_default_logger(impl_->logger_);
                spdlog::flush_every(std::chrono::seconds(3));
                break;
            }

        case LogTarget::MSVC:
            {
                auto msvc_sink = std::make_shared<spdlog::sinks::msvc_sink_mt>(false);
                msvc_sink->set_level(spdlog::level::trace);
                msvc_sink->set_pattern("%v %$");

                impl_->logger_ = std::make_shared<spdlog::logger>(g_logName, msvc_sink);
                impl_->logger_->set_level(spdlog::level::trace);
                break;
            }

        case LogTarget::MSVC_FILE:
            {
                auto file_sink = GetOrCreateUnicodeFileSink(actualLogPath);
                file_sink->set_level(spdlog::level::trace);
                file_sink->set_pattern("%^[%H:%M:%S.%e][%t] %v %$");

                auto msvc_sink = std::make_shared<spdlog::sinks::msvc_sink_mt>(false);
                msvc_sink->set_level(spdlog::level::trace);
                msvc_sink->set_pattern("%v %$");

                impl_->logger_ =
                        std::make_shared<spdlog::logger>(g_logName, spdlog::sinks_init_list{ file_sink, msvc_sink });

                impl_->logger_->set_level(spdlog::level::trace);
                spdlog::register_logger(impl_->logger_);
                spdlog::flush_every(std::chrono::seconds(3));
                break;
            }
    }
}

auto XLog::FreeLogger() -> void
{
    fs::path dbgFilePath = fs::path(g_currWorkDir) / std::string(DEBUG_FILE_NAME);
    if (fs::exists(dbgFilePath))
    {
#ifdef _WIN32
        ::FreeConsole();
#endif
    }
    spdlog::drop_all();
}

auto XLog::GetLogger() const noexcept -> SmartLog
{
    return impl_->logger_;
}

XLog::XLog()
{
    impl_ = std::make_unique<XLog::PImpl>(this);
}
