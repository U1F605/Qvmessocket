#pragma once

#include "base/QvBase.hpp"

namespace Qvmessocket::core::handler
{
    class RouteHandler : public QObject
    {
        Q_OBJECT
      public:
        explicit RouteHandler(QObject *parent = nullptr);
        ~RouteHandler();
        void SaveRoutes() const;
        //
        std::pair<bool, QvConfig_Route> GetAdvancedRoutingSettings(const GroupRoutingId &id) const
        {
            return { configs[id].overrideRoute, configs[id].routeConfig };
        }
        //
        bool SetAdvancedRouteSettings(const GroupRoutingId &id, bool overrideGlobal, const QvConfig_Route &dns);
        //
        OUTBOUNDS ExpandExternalConnection(const OUTBOUNDS &outbounds) const;
        //
        // Final Config Generation
        CONFIGROOT GenerateFinalConfig(const ConnectionGroupPair &pair, bool hasAPI = true) const;
        CONFIGROOT GenerateFinalConfig(CONFIGROOT root, const GroupRoutingId &routingId, bool hasAPI = true) const;
        //
        bool ExpandChainedOutbounds(CONFIGROOT &) const;

      private:
        QHash<GroupRoutingId, GroupRoutingConfig> configs;
    };
    inline ::Qvmessocket::core::handler::RouteHandler *RouteManager = nullptr;
}
