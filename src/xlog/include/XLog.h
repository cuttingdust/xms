/**
 * @file   XLog.h
 * @brief  
 *
 * @details   
 *
 * @author 31667
 * @date   2025-10-27
 */

#ifndef XLOG_H
#define XLOG_H

#include "XLog_Global.h"

#include <spdlog/logger.h>

#include <memory>
#include <string_view>


// =============================================================================
namespace crypt
{
// =============================================================================

//	// compile-time seed
//#define XSTR_SEED ((__TIME__[7] - '0') * 1ull    + (__TIME__[6] - '0') * 10ull  + \
//                   (__TIME__[4] - '0') * 60ull   + (__TIME__[3] - '0') * 600ull + \
//                   (__TIME__[1] - '0') * 3600ull + (__TIME__[0] - '0') * 36000ull)
#define XSTR_SEED 405

    // -----------------------------------------------------------------------------

    // @return a pseudo random number clamped at 0xFFFFFFFF
    constexpr unsigned long long linear_congruent_generator(unsigned rounds)
    {
        return 1013904223ull +
                (1664525ull * ((rounds > 0) ? linear_congruent_generator(rounds - 1) : (XSTR_SEED))) % 0xFFFFFFFF;
    }

    // -----------------------------------------------------------------------------

#define RandomLinear()               linear_congruent_generator(10)
#define XSTR_RANDOM_NUMBER(Min, Max) (Min + (RandomLinear() % (Max - Min + 1)))

    // -----------------------------------------------------------------------------

    constexpr const unsigned long long XORKEY = XSTR_RANDOM_NUMBER(0, 0xFF);

    // -----------------------------------------------------------------------------

    template <typename Char>
    constexpr Char encrypt_character(const Char character, int index)
    {
        return character ^ (static_cast<Char>(XORKEY) + index);
    }

    // -----------------------------------------------------------------------------

    template <unsigned size, typename Char>
    class Xor_string
    {
    public:
        const unsigned _nb_chars = (size - 1);
        Char           _string[size];

        // if every goes alright this constructor should be executed at compile time
        inline constexpr Xor_string(const Char* string) : _string{}
        {
            for (unsigned i = 0u; i < size; ++i)
                _string[i] = encrypt_character<Char>(string[i], i);
        }

        // This is executed at runtime.
        // HACK: although decrypt() is const we modify '_string' in place
        const Char* decrypt() const
        {
            Char* string = const_cast<Char*>(_string);
            for (unsigned t = 0; t < _nb_chars; t++)
            {
                string[t] = string[t] ^ (static_cast<Char>(XORKEY) + t);
            }
            string[_nb_chars] = '\0';
            return string;
        }
    };

} // namespace crypt

#define XorS(name, my_string) constexpr crypt::Xor_string<(sizeof(my_string) / sizeof(char)), char> name(my_string)
// Because of a limitation/bug in msvc 2017 we need to declare crypt::Xor_string() as a constexpr
// otherwise the constructor iso not evaluated at compile time. The lambda function is here to allow this declaration inside the macro
// because there is no such thing as casting to 'constexpr' (and casting to const does not solve this bug).
#define XorString(my_string)                                                                   \
    []                                                                                         \
    {                                                                                          \
        constexpr crypt::Xor_string<(sizeof(my_string) / sizeof(char)), char> expr(my_string); \
        return expr;                                                                           \
    }()                                                                                        \
            .decrypt()

// Crypt normal string char*
#define _ES_(string) XorString(string)

#define XorWS(name, my_string) \
    constexpr crypt::Xor_string<(sizeof(my_string) / sizeof(wchar_t)), wchar_t> name(my_string)
#define XorWideString(my_string)                                                                     \
    []                                                                                               \
    {                                                                                                \
        constexpr crypt::Xor_string<(sizeof(my_string) / sizeof(wchar_t)), wchar_t> expr(my_string); \
        return expr;                                                                                 \
    }()                                                                                              \
            .decrypt()

// crypt  wide characters
#define _EWS_(string) XorWideString(string)


enum class LogTarget
{
    FILE,
    CONSOLE,
    CONSOLE_FILE,
    CONSOLE_FILE_MSVC,
    MSVC,
    MSVC_FILE
};

class XLOG_EXPORT XLog
{
public:
    static auto Instance() -> XLog*;
    virtual ~XLog();
    using SmartLog = std::shared_ptr<spdlog::logger>;

public:
    auto ResetLogger(LogTarget target, std::string_view logPath) -> void;

    auto FreeLogger() -> void;

    [[nodiscard]]
    auto GetLogger() const noexcept -> SmartLog;

private:
    XLog();
    class PImpl;
    std::unique_ptr<PImpl> impl_;
};

#define XTrace(fmt, ...)    XLog::Instance()->GetLogger()->trace(fmt, __VA_ARGS__)
#define XDebug(fmt, ...)    XLog::Instance()->GetLogger()->debug(fmt, __VA_ARGS__)
#define XInfo(fmt, ...)     XLog::Instance()->GetLogger()->info(fmt, __VA_ARGS__)
#define XWarn(fmt, ...)     XLog::Instance()->GetLogger()->warn(fmt, __VA_ARGS__)
#define XError(fmt, ...)    XLog::Instance()->GetLogger()->error(fmt, __VA_ARGS__)
#define XCritical(fmt, ...) XLog::Instance()->GetLogger()->critical(fmt, __VA_ARGS__)


#endif // XLOG_H
