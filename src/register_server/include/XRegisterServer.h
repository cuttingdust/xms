/**
 * @file   XRegisterServer.h
 * @brief  注册中心服务端
 *
 * @details   
 *
 * @author 31667
 * @date   2024-12-17
 */

#ifndef XREGISTERSERVER_H
#define XREGISTERSERVER_H

#include <XService.h>

class XRegisterServer : public XService
{
public:
    /// 根据参数 初始化服务，需要先调用
    auto main(int argc, char *argv[]) -> void;

    /// 等待线程退出
    auto wait() -> void;

    auto createHandle() -> XServiceHandle * override;
};


#endif // XREGISTERSERVER_H
