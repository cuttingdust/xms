#ifndef XSINGLETON_H
#define XSINGLETON_H

#include <type_traits>

template <typename T>
class XSingleton
{
protected:
    XSingleton()                   = default;
    virtual ~XSingleton() noexcept = default;

public:
    static T *getInstance() noexcept(std::is_nothrow_constructible<T>::value)
    {
        static T instance;
        return &instance;
    }

    XSingleton(const XSingleton &)            = delete;
    XSingleton &operator=(const XSingleton &) = delete;

    XSingleton(XSingleton &&)            = delete;
    XSingleton &operator=(XSingleton &&) = delete;
};

#endif // XSINGLETON_H
