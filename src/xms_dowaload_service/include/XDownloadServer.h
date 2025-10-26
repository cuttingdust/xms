/**
 * @file   XDownloadServer.h
 * @brief  
 *
 * @details   
 *
 * @author 31667
 * @date   2025-10-27
 */

#ifndef XDOWNLOADSERVER_H
#define XDOWNLOADSERVER_H

#include <XService.h>

class XDownloadServer : public XService
{
public:
    XDownloadServer();
    ~XDownloadServer() override;

public:
    auto createHandle() -> XServiceHandle * override;

    auto main(int argc, char *argv[]) -> void;

private:
    class PImpl;
    std::unique_ptr<PImpl> impl_;
};


#endif // XDOWNLOADSERVER_H
