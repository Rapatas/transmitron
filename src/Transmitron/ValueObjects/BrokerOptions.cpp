#include "BrokerOptions.hpp"
#include <fmt/format.h>

using namespace Transmitron::ValueObjects;

const BrokerOptions BrokerOptions::defaults = BrokerOptions {
  false,
  {},
  "127.0.0.1",
  {},
  {},
  60,
  10,
  1883,
  30
};

BrokerOptions::BrokerOptions()
{
  *this = defaults;
}

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
  mClientId(clientId),
  mHostname(hostname),
  mPassword(password),
  mUsername(username),
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

template<>
std::optional<unsigned> BrokerOptions::extract<unsigned>(
  const nlohmann::json &data,
  const std::string &key
) {
  auto it = data.find(key);
  if (
    it == std::end(data)
    || it->type() != nlohmann::json::value_t::number_unsigned
  ) {
    return std::nullopt;
  }
  return it->get<unsigned>();
}

template<>
std::optional<std::string> BrokerOptions::extract<std::string>(
  const nlohmann::json &data,
  const std::string &key
) {
  auto it = data.find(key);
  if (
    it == std::end(data)
    || it->type() != nlohmann::json::value_t::string
  ) {
    return std::nullopt;
  }
  return it->get<std::string>();
}

template<>
std::optional<bool> BrokerOptions::extract<bool>(
  const nlohmann::json &data,
  const std::string &key
) {
  auto it = data.find(key);
  if (
    it == std::end(data)
    || it->type() != nlohmann::json::value_t::boolean
  ) {
    return std::nullopt;
  }
  return it->get<bool>();
}

BrokerOptions BrokerOptions::fromJson(const nlohmann::json &data)
{
  bool autoReconnect = extract<bool>(data, "autoReconnect")
    .value_or(defaults.getAutoReconnect());

  std::string clientId = extract<std::string>(data, "clientId")
    .value_or(defaults.getClientId());

  std::string hostname = extract<std::string>(data, "hostname")
    .value_or(defaults.getHostname());

  std::string password = extract<std::string>(data, "password")
    .value_or(defaults.getPassword());

  std::string username = extract<std::string>(data, "username")
    .value_or(defaults.getUsername());

  unsigned keepAliveInterval = extract<unsigned>(data, "keepAliveInterval")
    .value_or(defaults.getKeepAliveInterval());

  unsigned maxInFlight = extract<unsigned>(data, "maxInFlight")
    .value_or(defaults.getMaxInFlight());

  unsigned port = extract<unsigned>(data, "port")
    .value_or(defaults.getPort());

  unsigned timeout = extract<unsigned>(data, "timeout")
    .value_or(defaults.getTimeout());

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


