#include "XRouteServer.h"

#include "XConfigClient.h"
#include "XRouteHandle.h"
#include "XSSL_CTX.h"
#include "XTools.h"

XRouteServer::XRouteServer() = default;

XRouteServer::~XRouteServer() = default;

XServiceHandle* XRouteServer::createHandle()
{
    auto router = new XRouteHandle();
    bool is_ssl = XConfigClient::get()->GetBool("is_ssl");
    std::cout << "is_ssl = " << is_ssl << std::endl;
    if (!is_ssl)
    {
        return router;
    }

    /// 已经设置过， 暂时不考虑修改
    if (getSSLContent())
    {
        return router;
    }
    auto               ctx      = new XSSL_CTX();
    const std::string& crt_path = XConfigClient::get()->GetString("crt_path");
    const std::string& key_path = XConfigClient::get()->GetString("key_path");
    const std::string& ca_path  = XConfigClient::get()->GetString("ca_path");

    std::cout << "crt_path = " << crt_path << std::endl;
    std::cout << "key_path = " << key_path << std::endl;
    std::cout << "ca_path = " << ca_path << std::endl;

    ctx->initServer(crt_path.c_str(), key_path.c_str(), ca_path.c_str());

    this->setSSLContent(ctx);

    return router;
}
