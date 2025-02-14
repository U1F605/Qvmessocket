#pragma once

#include "CoreObjectModels.hpp"
#include "QvConfigIdentifier.hpp"
#include "QvSafeType.hpp"

namespace Qvmessocket::base::objects::complex
{
    enum ComplexTagNodeMode
    {
        NODE_INBOUND,
        NODE_OUTBOUND,
        NODE_RULE
    };

    enum MetaOutboundObjectType
    {
        METAOUTBOUND_ORIGINAL,
        METAOUTBOUND_EXTERNAL,
        METAOUTBOUND_BALANCER,
        METAOUTBOUND_CHAIN
    };

    constexpr auto META_OUTBOUND_KEY_NAME = "QVMESSOCKET_OUTBOUND_METADATA";
    constexpr auto QVMESSOCKET_CHAINED_OUTBOUND_PORT_ALLOCATION = 15500;

    typedef BalancerObject ComplexBalancerObject;

    struct OutboundObjectMeta
    {
        MetaOutboundObjectType metaType;
        QString displayName;
        //
        ConnectionId connectionId;
        QList<QString> outboundTags;
        QString strategyType;
        int chainPortAllocation = QVMESSOCKET_CHAINED_OUTBOUND_PORT_ALLOCATION;
        //
        safetype::OUTBOUND realOutbound;
        QString getDisplayName() const
        {
            if (metaType == METAOUTBOUND_ORIGINAL)
                return realOutbound["tag"].toString();
            else
                return displayName;
        }
        static OutboundObjectMeta loadFromOutbound(const safetype::OUTBOUND &out)
        {
            OutboundObjectMeta meta;
            meta.loadJson(out[META_OUTBOUND_KEY_NAME].toObject());
            meta.realOutbound = out;
            return meta;
        }
        OutboundObjectMeta() : metaType(METAOUTBOUND_ORIGINAL){};
        JSONSTRUCT_REGISTER(OutboundObjectMeta, F(metaType, displayName, connectionId, outboundTags, chainPortAllocation, strategyType))
    };

    inline OutboundObjectMeta make_chained_outbound(const QList<QString> &chain, const QString &tag)
    {
        OutboundObjectMeta meta;
        meta.metaType = METAOUTBOUND_CHAIN;
        meta.outboundTags = chain;
        meta.displayName = tag;
        return meta;
    }

    inline OutboundObjectMeta make_balancer_outbound(const QList<QString> &outbounds, const QString &type, const QString &tag)
    {
        OutboundObjectMeta meta;
        meta.metaType = METAOUTBOUND_BALANCER;
        meta.outboundTags = outbounds;
        meta.strategyType = type;
        meta.displayName = tag;
        return meta;
    }

    inline OutboundObjectMeta make_external_outbound(const ConnectionId &id, const QString &tag)
    {
        OutboundObjectMeta meta;
        meta.metaType = METAOUTBOUND_EXTERNAL;
        meta.connectionId = id;
        meta.displayName = tag;
        return meta;
    }

    inline OutboundObjectMeta make_normal_outbound(const safetype::OUTBOUND &outbound)
    {
        OutboundObjectMeta meta;
        meta.metaType = METAOUTBOUND_ORIGINAL;
        meta.realOutbound = outbound;
        meta.displayName = outbound["tag"].toString();
        return meta;
    }
}

using namespace Qvmessocket::base::objects::complex;
