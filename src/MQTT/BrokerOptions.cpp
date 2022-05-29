#include "BrokerOptions.hpp"
#include <wx/utils.h>
#include <chrono>
#include <fmt/format.h>
#include <string_view>
#include "Common/Extract.hpp"

using namespace MQTT;

BrokerOptions::BrokerOptions() :
  mAutoReconnect(DefaultAutoReconnect),
  mMaxInFlight(DefaultMaxInFlight),
  mMaxReconnectRetries(DefaultMaxReconnectRetries),
  mPort(DefaultPort),
  mConnectTimeout(DefaultTimeout),
  mDisconnectTimeout(DefaultTimeout),
  mKeepAliveInterval(DefaultKeepAliveInterval),
  mClientId(DefaultClientId),
  mHostname(DefaultHostname),
  mPassword(DefaultPassword),
  mUsername(DefaultUsername)
{}

BrokerOptions::BrokerOptions(
  bool autoReconnect,
  size_t maxInFlight,
  size_t maxReconnectRetries,
  size_t port,
  std::chrono::seconds connectTimeout,
  std::chrono::seconds disconnectTimeout,
  std::chrono::seconds keepAliveInterval,
  std::string clientId,
  std::string hostname,
  std::string password,
  std::string username
) :
  mAutoReconnect(autoReconnect),
  mMaxInFlight(maxInFlight),
  mMaxReconnectRetries(maxReconnectRetries),
  mPort(port),
  mConnectTimeout(connectTimeout),
  mDisconnectTimeout(disconnectTimeout),
  mKeepAliveInterval(keepAliveInterval),
  mClientId(std::move(clientId)),
  mHostname(std::move(hostname)),
  mPassword(std::move(password)),
  mUsername(std::move(username))
{
  if (mClientId.empty())
  {
    mClientId = fmt::format("{}_{}", wxGetHostName(), rand());
  }
}

BrokerOptions BrokerOptions::fromJson(const nlohmann::json &data)
{
  using namespace Common;

  bool autoReconnect = extract<bool>(data, "autoReconnect")
    .value_or(DefaultAutoReconnect);

  std::string clientId = extract<std::string>(data, "clientId")
    .value_or(std::string(DefaultClientId));

  std::string hostname = extract<std::string>(data, "hostname")
    .value_or(std::string(DefaultHostname));

  std::string password = extract<std::string>(data, "password")
    .value_or(std::string(DefaultPassword));

  std::string username = extract<std::string>(data, "username")
    .value_or(std::string(DefaultUsername));

  unsigned keepAliveInterval = extract<unsigned>(data, "keepAliveInterval")
    .value_or(DefaultKeepAliveInterval.count());

  unsigned maxInFlight = extract<unsigned>(data, "maxInFlight")
    .value_or(DefaultMaxInFlight);

  unsigned port = extract<unsigned>(data, "port")
    .value_or(DefaultPort);

  unsigned connectTimeout = extract<unsigned>(data, "connectTimeout")
    .value_or(DefaultTimeout.count());

  unsigned disconnectTimeout = extract<unsigned>(data, "disconnectTimeout")
    .value_or(DefaultTimeout.count());

  unsigned maxReconnectRetries = extract<unsigned>(data, "maxReconnectRetries")
    .value_or(DefaultMaxReconnectRetries);

  return BrokerOptions {
    autoReconnect,
    maxInFlight,
    maxReconnectRetries,
    port,
    std::chrono::seconds(connectTimeout),
    std::chrono::seconds(disconnectTimeout),
    std::chrono::seconds(keepAliveInterval),
    clientId,
    hostname,
    password,
    username,
  };
}

nlohmann::json BrokerOptions::toJson() const
{
  return {
    {"autoReconnect",       mAutoReconnect             },
    {"maxReconnectRetries", mMaxReconnectRetries       },
    {"clientId",            mClientId                  },
    {"connectTimeout",      mConnectTimeout.count()    },
    {"disconnectTimeout",   mDisconnectTimeout.count() },
    {"hostname",            mHostname                  },
    {"keepAliveInterval",   mKeepAliveInterval.count() },
    {"maxInFlight",         mMaxInFlight               },
    {"password",            mPassword                  },
    {"port",                mPort                      },
    {"username",            mUsername                  },
  };
}

bool BrokerOptions::getAutoReconnect() const
{
  return mAutoReconnect;
}

size_t BrokerOptions::getPort() const
{
  return mPort;
}

std::chrono::seconds BrokerOptions::getKeepAliveInterval() const
{
  return mKeepAliveInterval;
}

size_t BrokerOptions::getMaxInFlight() const
{
  return mMaxInFlight;
}

std::chrono::seconds BrokerOptions::getConnectTimeout() const
{
  return mConnectTimeout;
}

std::chrono::seconds BrokerOptions::getDisconnectTimeout() const
{
  return mDisconnectTimeout;
}

std::string BrokerOptions::getHostname() const
{
  return mHostname;
}

std::string BrokerOptions::getClientId() const
{
  return mClientId;
}

std::string BrokerOptions::getUsername() const
{
  return mUsername;
}

std::string BrokerOptions::getPassword() const
{
  return mPassword;
}

size_t BrokerOptions::getMaxReconnectRetries() const
{
  return mMaxReconnectRetries;
}
