/**
 * @file   XConfigClient.h
 * @brief  
 *
 * @details   
 *
 * @author 31667
 * @date   2024-12-23
 */

#ifndef XCONFIGCLIENT_H
#define XCONFIGCLIENT_H

#include "XPlatfrom_Global.h"
#include <XServiceClient.h>

typedef void (*ConfigResCBFunc)(bool is_ok, const char *msg);

class XPLATFROM_EXPORT XConfigClient : public XServiceClient
{
public:
    static XConfigClient *get()
    {
        static XConfigClient instance;
        return &instance;
    }

private:
    XConfigClient();
    ~XConfigClient() override;

public:
    /// \brief ��������
    /// \param conf
    void sendConfig(xmsg::XConfig *conf);

    /// \brief ���յ��������õ���Ϣ
    /// \param head
    /// \param msg
    void sendConfigRes(xmsg::XMsgHead *head, XMsg *msg);

    /// \brief ��ȡ�������� IP���ΪNULL ��ȡ�����������ĵĵ�ַ
    /// \param ip
    /// \param port
    void loadConfig(const char *ip, int port);

    void loadConfigRes(xmsg::XMsgHead *head, XMsg *msg);

    /// \brief
    /// \param ip
    /// \param port
    /// \param out_conf
    /// \return
    bool getConfig(const char *ip, int port, xmsg::XConfig *out_conf);

    /// \brief ����proto�ļ� �̲߳���ȫ
    /// \param file_name  �ļ�·��
    /// \param class_name  ���õ�����
    /// \param out_proto_code ��ȡ���Ĵ��룬�����ռ�Ͱ汾
    /// \return
    google::protobuf::Message *loadProto(const std::string &file_name, const std::string &class_name,
                                         std::string &out_proto_code);

    static void regMsgCallback();

    void wait();

    /// \brief �����������ģ���ʼ��ʱ����ȡ����
    /// \param server_ip
    /// \param server_port
    /// \param local_ip
    /// \param local_port
    /// \param conf_message
    /// \param timeout_sec
    /// \return
    bool startGetConf(const char *server_ip, int server_port, const char *local_ip, int local_port,
                      google::protobuf::Message *conf_message, int timeout_sec = 10);

    /// \brief ��ȡ���صı��ز���
    /// \return
    std::string getString(const char *key);
    int         getInt(const char *key);
    bool        getBool(const char *key);

    void timerCB() override;

    /// \brief ���õ�ǰ���õĶ���
    /// \param message
    void setCurServiceMessage(google::protobuf::Message *message);

    void loadAllConfigRes(xmsg::XMsgHead *head, XMsg *msg);

    /// \brief ��ȡȫ�������б�
    /// 1 �Ͽ������Զ�����
    /// 2 �ȴ��������
    /// \param page
    /// \param page_count
    /// \param timeout_sec
    /// \return
    xmsg::XConfigList getAllConfig(int page, int page_count, int timeout_sec);

    /// \brief ����ɾ����������
    /// \param ip
    /// \param port
    void deleteConfig(const char *ip, int port);

    void deleteConfigRes(xmsg::XMsgHead *head, XMsg *msg);

public:
    ConfigResCBFunc sendConfigResCB = nullptr;

private:
    class PImpl;
    std::unique_ptr<PImpl> impl_;
};


#endif // XCONFIGCLIENT_H
