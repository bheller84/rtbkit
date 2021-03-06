/* http_adserver_connector.h                                       -*- C++ -*-
   Wolfgang Sourdeau, April 2013
   Copyright (c) 2013 Datacratic.  All rights reserved.
*/


#pragma once

#include <string>
#include <vector>

#include "soa/service/http_endpoint.h"
#include "soa/service/json_endpoint.h"

#include "adserver_connector.h"


namespace RTBKIT {

/****************************************************************************/
/* HTTPADSERVERCONNECTIONHANDLER                                            */
/****************************************************************************/

class HttpAdServerHttpEndpoint;

typedef std::function<void (const HttpHeader & header,
                            const Json::Value & json,
                            const std::string & jsonStr)>
    HttpAdServerRequestCb;

struct HttpAdServerConnectionHandler
    : public Datacratic::JsonConnectionHandler {
    HttpAdServerConnectionHandler(HttpAdServerHttpEndpoint & endpoint,
                                  const HttpAdServerRequestCb & requestCb);

    virtual void handleJson(const HttpHeader & header,
                            const Json::Value & json,
                            const std::string & jsonStr);

private:
    HttpAdServerHttpEndpoint & endpoint_;
    const HttpAdServerRequestCb & requestCb_;
};


/****************************************************************************/
/* HTTPADSERVERHTTPENDPOINT                                                 */
/****************************************************************************/

struct HttpAdServerHttpEndpoint : public Datacratic::HttpEndpoint {
    HttpAdServerHttpEndpoint(int port,
                             const HttpAdServerRequestCb & requestCb);
    HttpAdServerHttpEndpoint(HttpAdServerHttpEndpoint && otherEndpoint);

    ~HttpAdServerHttpEndpoint();

    HttpAdServerHttpEndpoint & operator =
        (const HttpAdServerHttpEndpoint& other);

    int getPort() const;

    /* carbon logging */
    typedef std::function<void (const char * eventName,
                                EventType,
                                float)> OnEvent;
    OnEvent onEvent;

    void doEvent(const char * eventName, EventType type = ET_COUNT,
                 float value = 1.0, const char * units = "")
      const
    {
        if (onEvent) {
            std::string prefixedName(name() + "." + eventName);
            onEvent(prefixedName.c_str(), type, value);
        }
    }

    virtual std::shared_ptr<ConnectionHandler> makeNewHandler();

private:
    int port_;
    HttpAdServerRequestCb requestCb_;
};
        
/****************************************************************************/
/* HTTPADSERVERCONNECTOR                                                    */
/****************************************************************************/

struct HttpAdServerConnector : public AdServerConnector {
    HttpAdServerConnector(const std::string & serviceName,
                          const std::shared_ptr<Datacratic::ServiceProxies>
                          & proxy);
    ~HttpAdServerConnector() {
        shutdown();
    }

    void registerEndpoint(int port, const HttpAdServerRequestCb & requestCb);

    void init(const std::shared_ptr<ConfigurationService> & config);
    void shutdown();

    void bindTcp();

    void start();

private:
    std::vector<HttpAdServerHttpEndpoint> endpoints_;
};

} //namespace RTBKIT
