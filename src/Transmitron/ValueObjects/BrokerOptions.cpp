#include "BrokerOptions.hpp"
#include <fmt/format.h>
#include "Helpers/Extract.hpp"

using namespace Transmitron::ValueObjects;

constexpr bool DefaultAutoReconnect = false;
constexpr const char *DefaultClientId = "";
constexpr const char *DefaultHostname = "127.0.0.1";
constexpr const char *DefaultPassword = "";
constexpr const char *DefaultUsername = "";
constexpr unsigned DefaultKeepAliveIntervalSec = 60;
constexpr unsigned DefaultMaxInFlight = 10;
constexpr unsigned DefaultPort = 1883;
constexpr unsigned DefaultTimeoutSec = 30;

BrokerOptions::BrokerOptions() :
   mAutoReconnect(DefaultAutoReconnect),
   mClientId(DefaultClientId),
   mHostname(DefaultHostname),
   mPassword(DefaultPassword),
   mUsername(DefaultUsername),
   mKeepAliveInterval(DefaultKeepAliveIntervalSec),
   mMaxInFlight(DefaultMaxInFlight),
   mPort(DefaultPort),
   mTimeout(DefaultTimeoutSec)
{}

BrokerOptions::BrokerOptions(
  bool autoReconnect,
  std::string clientId,
  std::string hostname,
  std::string password,
  std::string username,
  unsigned keepAliveInterval,
  unsigned maxInFlight,
  unsigned port,
  unsigned timeout
) :
  mAutoReconnect(autoReconnect),
  mClientId(std::move(clientId)),
  mHostname(std::move(hostname)),
  mPassword(std::move(password)),
  mUsername(std::move(username)),
  mKeepAliveInterval(keepAliveInterval),
  mMaxInFlight(maxInFlight),
  mPort(port),
  mTimeout(timeout)
{
  if (mClientId.empty())
  {
    mClientId = fmt::format("{}-client", rand());
  }
}

BrokerOptions BrokerOptions::fromJson(const nlohmann::json &data)
{
  using namespace Helpers;

  bool autoReconnect = extract<bool>(data, "autoReconnect")
    .value_or(DefaultAutoReconnect);

  std::string clientId = extract<std::string>(data, "clientId")
    .value_or(DefaultClientId);

  std::string hostname = extract<std::string>(data, "hostname")
    .value_or(DefaultHostname);

  std::string password = extract<std::string>(data, "password")
    .value_or(DefaultPassword);

  std::string username = extract<std::string>(data, "username")
    .value_or(DefaultUsername);

  unsigned keepAliveInterval = extract<unsigned>(data, "keepAliveInterval")
    .value_or(DefaultKeepAliveIntervalSec);

  unsigned maxInFlight = extract<unsigned>(data, "maxInFlight")
    .value_or(DefaultMaxInFlight);

  unsigned port = extract<unsigned>(data, "port")
    .value_or(DefaultPort);

  unsigned timeout = extract<unsigned>(data, "timeout")
    .value_or(DefaultTimeoutSec);

  return BrokerOptions {
    autoReconnect,
    clientId,
    hostname,
    password,
    username,
    keepAliveInterval,
    maxInFlight,
    port,
    timeout,
  };
}

nlohmann::json BrokerOptions::toJson() const
{
  return {
    {"autoReconnect",     mAutoReconnect     },
    {"clientId",          mClientId          },
    {"hostname",          mHostname          },
    {"keepAliveInterval", mKeepAliveInterval },
    {"maxInFlight",       mMaxInFlight       },
    {"password",          mPassword          },
    {"port",              mPort              },
    {"timeout",           mTimeout           },
    {"username",          mUsername          },
  };
}

bool BrokerOptions::getAutoReconnect() const
{
  return mAutoReconnect;
}

unsigned BrokerOptions::getPort() const
{
  return mPort;
}

unsigned BrokerOptions::getKeepAliveInterval() const
{
  return mKeepAliveInterval;
}

unsigned BrokerOptions::getMaxInFlight() const
{
  return mMaxInFlight;
}

unsigned BrokerOptions::getTimeout() const
{
  return mTimeout;
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
