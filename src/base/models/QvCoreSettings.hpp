#pragma once
#include "QJsonStruct.hpp"
#include "base/models/CoreObjectModels.hpp"

namespace Qvmessocket::base::config
{
    struct QvConfig_Route
    {
        struct QvRouteConfig_Impl
        {
            QList<QString> direct;
            QList<QString> block;
            QList<QString> proxy;
            QvRouteConfig_Impl(){};
            QvRouteConfig_Impl(const QList<QString> &_direct, const QList<QString> &_block, const QList<QString> &_proxy)
                : direct(_direct), block(_block), proxy(_proxy){};
            JSONSTRUCT_COMPARE(QvRouteConfig_Impl, direct, block, proxy)
            JSONSTRUCT_REGISTER(QvRouteConfig_Impl, F(proxy, block, direct))
        };

        QString domainStrategy;
        QString domainMatcher = "mph";
        QvRouteConfig_Impl domains;
        QvRouteConfig_Impl ips;
        QvConfig_Route(){};
        QvConfig_Route(const QvRouteConfig_Impl &_domains, const QvRouteConfig_Impl &_ips, const QString &ds)
            : domainStrategy(ds), domains(_domains), ips(_ips){};
        JSONSTRUCT_COMPARE(QvConfig_Route, domainStrategy, domainMatcher, domains, ips)
        JSONSTRUCT_REGISTER(QvConfig_Route, A(domainStrategy, domainMatcher, domains, ips))
    };

    struct QvConfig_Outbounds
    {
        int mark = 255;
        JSONSTRUCT_COMPARE(QvConfig_Outbounds, mark)
        JSONSTRUCT_REGISTER(QvConfig_Outbounds, F(mark))
    };

    struct QvConfig_ForwardProxy
    {
        bool enableForwardProxy = false;
        QString type = "http";
        QString serverAddress = "";
        int port = 0;
        bool useAuth = false;
        QString username = "";
        QString password = "";
        JSONSTRUCT_COMPARE(QvConfig_ForwardProxy, enableForwardProxy, type, serverAddress, port, useAuth, username, password)
        JSONSTRUCT_REGISTER(QvConfig_ForwardProxy, F(enableForwardProxy, type, serverAddress, port, useAuth, username, password))
    };

    struct QvConfig_Connection
    {
        bool enableProxy = true;
        bool bypassCN = true;
        bool bypassBT = false;
        bool bypassLAN = true;
        bool FreedomDNS = false;
        bool dnsIntercept = false;
        JSONSTRUCT_COMPARE(QvConfig_Connection, enableProxy, //
                           bypassCN, bypassBT, bypassLAN,    //
                           FreedomDNS, dnsIntercept)
        JSONSTRUCT_REGISTER(QvConfig_Connection, F(bypassCN, bypassBT, bypassLAN, enableProxy, FreedomDNS, dnsIntercept))
    };

    struct QvConfig_SystemProxy
    {
        bool setSystemProxy = true;
        JSONSTRUCT_COMPARE(QvConfig_SystemProxy, setSystemProxy)
        JSONSTRUCT_REGISTER(QvConfig_SystemProxy, F(setSystemProxy))
    };

    struct QvConfig_ProtocolInboundBase
    {
        int port = 0;
        bool useAuth = false;
        bool sniffing = false;
        QList<QString> destOverride = { "http", "tls" };
        objects::AccountObject account;
        bool metadataOnly = true;
        QvConfig_ProtocolInboundBase(){};
        JSONSTRUCT_REGISTER(QvConfig_ProtocolInboundBase, F(port, useAuth, sniffing, destOverride, account, metadataOnly))
    };

    struct QvConfig_SocksInbound : QvConfig_ProtocolInboundBase
    {
        bool enableUDP = true;
        QString localIP = "127.0.0.1";
        QvConfig_SocksInbound() : QvConfig_ProtocolInboundBase()
        {
            port = 1089;
        }
        JSONSTRUCT_COMPARE(QvConfig_SocksInbound, enableUDP, localIP, port, useAuth, sniffing, destOverride, metadataOnly)
        JSONSTRUCT_REGISTER(QvConfig_SocksInbound, B(QvConfig_ProtocolInboundBase), F(enableUDP, localIP))
    };

    struct QvConfig_HttpInbound : QvConfig_ProtocolInboundBase
    {
        QvConfig_HttpInbound() : QvConfig_ProtocolInboundBase()
        {
            port = 8889;
        }
        JSONSTRUCT_COMPARE(QvConfig_HttpInbound, port, useAuth, sniffing, destOverride, metadataOnly)
        JSONSTRUCT_REGISTER(QvConfig_HttpInbound, B(QvConfig_ProtocolInboundBase))
    };

    struct QvConfig_TProxy : QvConfig_ProtocolInboundBase
    {
        QString tProxyIP = "127.0.0.1";
        QString tProxyV6IP = "::1";
        bool hasTCP = true;
        bool hasUDP = true;
        QString mode = "tproxy";
        QvConfig_TProxy() : QvConfig_ProtocolInboundBase()
        {
            port = 12345;
            sniffing = true;
        }
        JSONSTRUCT_COMPARE(QvConfig_TProxy, tProxyIP, tProxyV6IP, hasTCP, hasUDP, mode, port, useAuth, sniffing, destOverride, metadataOnly)
        JSONSTRUCT_REGISTER(QvConfig_TProxy, B(QvConfig_ProtocolInboundBase), F(tProxyIP, tProxyV6IP, hasTCP, hasUDP, mode))
    };

    struct QvConfig_Inbounds
    {
        QString listenip = "127.0.0.1";
        bool useSocks = true;
        bool useHTTP = true;
        bool useTPROXY = false;
        //
        QvConfig_TProxy tProxySettings;
        QvConfig_HttpInbound httpSettings;
        QvConfig_SocksInbound socksSettings;
        QvConfig_SystemProxy systemProxySettings;
        JSONSTRUCT_COMPARE(QvConfig_Inbounds, listenip, useSocks, useHTTP, useTPROXY, tProxySettings, httpSettings, socksSettings,
                           systemProxySettings);
        JSONSTRUCT_REGISTER(QvConfig_Inbounds,                         //
                            A(socksSettings),                          //
                            F(listenip, useSocks, useHTTP, useTPROXY), //
                            F(tProxySettings, httpSettings, systemProxySettings));
    };
}
