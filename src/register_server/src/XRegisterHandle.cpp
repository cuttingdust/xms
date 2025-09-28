#include "XRegisterHandle.h"

#include <XTools.h>

#include <print>


static xmsg::XServiceMap *service_map = nullptr; ///< 注册服务列表的缓存
static std::mutex         service_map_mutex;     ///< 多线程访问的锁

auto XRegisterHandle::registerReq(xmsg::XMsgHead *head, XMsg *msg) -> void
{
    LOGDEBUG("服务端接收到用户的注册请求");

    /// 回应的消息
    xmsg::XMessageRes res;

    ///解析请求
    xmsg::XRegisterReq req;
    if (!req.ParseFromArray(msg->data, msg->size))
    {
        LOGDEBUG("XRegisterReq ParseFromArray failed!");
        res.set_return_(xmsg::XMessageRes::XR_ERROR);
        res.set_msg("XRegisterReq ParseFromArray failed!");
        sendMsg(xmsg::MT_REGISTER_RES, &res);
        return;
    }


    /// 接收到用户的服务名称、服务IP、服务端口
    std::string service_name = req.name();
    if (service_name.empty())
    {
        std::string error = "service_name is empty!";
        LOGDEBUG(error.c_str());
        res.set_return_(xmsg::XMessageRes::XR_ERROR);
        res.set_msg(error);
        sendMsg(xmsg::MT_REGISTER_RES, &res);
        return;
    }
    std::string service_ip = req.ip();
    if (service_ip.empty())
    {
        LOGDEBUG("service_ip is empty : client ip:" + std::string(this->getClientIP()));
        service_ip = this->getClientIP();
    }

    int service_port = req.port();
    if (service_port <= 0 || service_port > 65535)
    {
        std::stringstream ss;
        //string error = "service_port is error!";
        ss << "service_port is error!" << service_port;
        LOGDEBUG(ss.str().c_str());
        res.set_return_(xmsg::XMessageRes::XR_ERROR);
        res.set_msg(ss.str());
        sendMsg(xmsg::MT_REGISTER_RES, &res);
        return;
    }

    /// 接收用户注册信息正常
    std::stringstream ss;
    ss << "接收到用户注册信息:" << service_name << "|" << service_ip << ":" << service_port;
    LOGINFO(ss.str().c_str());


    /// 存储用户注册信息，如果已经注册需要更新
    {
        XMutex mutex(&service_map_mutex);
        if (!service_map)
            service_map = new xmsg::XServiceMap();
        auto smap = service_map->mutable_servicemap();

        /// 是否由同类型已经注册
        /// 集群微服务
        auto service_list = smap->find(service_name);
        if (service_list == smap->end())
        {
            /// 没有注册过
            (*smap)[service_name] = xmsg::XServiceMap::XServiceList();
            service_list          = smap->find(service_name);
        }
        auto services = service_list->second.mutable_services();
        /// 查找是否用同ip和端口的
        for (const auto &service : (*services))
        {
            if (service.ip() == service_ip && service.port() == service_port)
            {
                std::stringstream ss;
                ss << service_name << "|" << service_ip << ":" << service_port << "微服务已经注册过";
                LOGDEBUG(ss.str().c_str());
                res.set_return_(xmsg::XMessageRes::XR_ERROR);
                res.set_msg(ss.str());
                sendMsg(xmsg::MT_REGISTER_RES, &res);
                return;
            }
        }
        /// 添加新的微服务
        auto ser = service_list->second.add_services();
        ser->set_ip(service_ip);
        ser->set_port(service_port);
        ser->set_name(service_name);
        std::stringstream ss;
        ss << service_name << "|" << service_ip << ":" << service_port << "新的微服务注册成功！";
        LOGDEBUG(ss.str().c_str());
    }

    res.set_return_(xmsg::XMessageRes::XR_OK);
    res.set_msg("OK");
    sendMsg(xmsg::MT_REGISTER_RES, &res);
}

auto XRegisterHandle::getServiceReq(xmsg::XMsgHead *head, XMsg *msg) -> void
{
    /// 暂时只发送全部
    LOGDEBUG("XRegisterHandle::getServiceReq");
    xmsg::XGetServiceReq req;

    /// 错误处理
    xmsg::XServiceMap res;
    res.mutable_res()->set_return_(xmsg::XMessageRes_XReturn::XMessageRes_XReturn_XR_ERROR);
    if (!req.ParseFromArray(msg->data, msg->size))
    {
        std::stringstream ss;
        ss << "req.ParseFromArray failed!";
        LOGDEBUG(ss.str().c_str());
        res.mutable_res()->set_msg(ss.str().c_str());
        sendMsg(xmsg::MT_GET_SERVICE_RES, &res);
        return;
    }


    std::string       service_name = req.name();
    std::stringstream ss;
    ss << "GetServiceReq : service name " << service_name;
    LOGDEBUG(ss.str().c_str());
    xmsg::XServiceMap *send_map = &res;

    ///发送全部微服务数据
    service_map_mutex.lock();
    if (!service_map)
    {
        service_map = new xmsg::XServiceMap();
    }

    ///返回全部
    if (req.type() == xmsg::XServiceType::XT_ALL)
    {
        send_map = service_map;
    }
    else ///返回单种
    {
        auto smap = service_map->mutable_servicemap();
        if (smap && smap->find(service_name) != smap->end())
        {
            (*send_map->mutable_servicemap())[service_name] = (*smap)[service_name];
        }
    }
    service_map_mutex.unlock();


    /// 返回单种还是全部
    service_map->set_type(req.type());
    service_map->mutable_res()->set_return_(xmsg::XMessageRes_XReturn::XMessageRes_XReturn_XR_OK);
    sendMsg(xmsg::MT_GET_SERVICE_RES, service_map);
}

auto XRegisterHandle::heartRes(xmsg::XMsgHead *head, XMsg *msg) -> void
{
    std::print("{}", __func__);
}

auto XRegisterHandle::regMsgCallback() -> void
{
    regCB(xmsg::MT_HEART_REQ, static_cast<MsgCBFunc>(&XRegisterHandle::heartRes));
    regCB(xmsg::MT_REGISTER_REQ, static_cast<MsgCBFunc>(&XRegisterHandle::registerReq));
    regCB(xmsg::MT_GET_SERVICE_REQ, static_cast<MsgCBFunc>(&XRegisterHandle::getServiceReq));
}
