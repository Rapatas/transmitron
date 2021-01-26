#include "Connection.hpp"

Connection::Connection() {}
Connection::~Connection() {}

Connection::Connection(const nlohmann::json &data, const std::string &name)
{
  fromJson(data);
  mName = name;
}

bool        Connection::getAutoReconnect()     const { return mAutoReconnect     ; }
unsigned    Connection::getPort()              const { return mPort              ; }
unsigned    Connection::getKeepAliveInterval() const { return mKeepAliveInterval ; }
unsigned    Connection::getMaxInFlight()       const { return mMaxInFlight       ; }
unsigned    Connection::getTimeout()           const { return mTimeout           ; }
std::string Connection::getName()              const { return mName              ; }
std::string Connection::getHostname()          const { return mHostname          ; }
std::string Connection::getClientId()          const { return mClientId          ; }
std::string Connection::getUsername()          const { return mUsername          ; }
std::string Connection::getPassword()          const { return mPassword          ; }

void Connection::setAutoReconnect(bool autoReconnect)             { mAutoReconnect     = autoReconnect;     }
void Connection::setPort(unsigned port)                           { mPort              = port;              }
void Connection::setKeepAliveInterval(unsigned keepAliveInterval) { mKeepAliveInterval = keepAliveInterval; }
void Connection::setMaxInFlight(unsigned maxInFlight)             { mMaxInFlight       = maxInFlight;       }
void Connection::setTimeout(unsigned timeout)                     { mTimeout           = timeout;           }
void Connection::setName(std::string name)                        { mName              = name;              }
void Connection::setHostname(std::string hostname)                { mHostname          = hostname;          }
void Connection::setClientId(std::string clientId)                { mClientId          = clientId;          }
void Connection::setUsername(std::string username)                { mUsername          = username;          }
void Connection::setPassword(std::string password)                { mPassword          = password;          }

nlohmann::json Connection::toJson() const
{
  return {
    {"autoReconnect",     mAutoReconnect     },
    {"port",              mPort              },
    {"keepAliveInterval", mKeepAliveInterval },
    {"maxInFlight",       mMaxInFlight       },
    {"timeout",           mTimeout           },
    {"hostname",          mHostname          },
    {"clientId",          mClientId          },
    {"username",          mUsername          },
    {"password",          mPassword          },
  };
}

template<>
bool Connection::tryToSet<unsigned>(
  const nlohmann::json &data,
  const std::string &key,
  unsigned& value
) {
  auto it = data.find(key);
  if (it == std::end(data)) { return false; }
  if (it->type() != nlohmann::json::value_t::number_unsigned) { return false; }
  value = it->get<int>();
  return true;
}

template<>
bool Connection::tryToSet<std::string>(
  const nlohmann::json &data,
  const std::string &key,
  std::string& value
) {
  auto it = data.find(key);
  if (it == std::end(data)) { return false; }
  if (it->type() != nlohmann::json::value_t::string) { return false; }
  value = it->get<std::string>();
  return true;
}

template<>
bool Connection::tryToSet<bool>(
  const nlohmann::json &data,
  const std::string &key,
  bool& value
) {
  auto it = data.find(key);
  if (it == std::end(data)) { return false; }
  if (it->type() != nlohmann::json::value_t::boolean) { return false; }
  value = it->get<bool>();
  return true;
}

void Connection::fromJson(const nlohmann::json &data)
{
  tryToSet(data, "autoReconnect",     mAutoReconnect     );
  tryToSet(data, "port",              mPort              );
  tryToSet(data, "keepAliveInterval", mKeepAliveInterval );
  tryToSet(data, "maxInFlight",       mMaxInFlight       );
  tryToSet(data, "timeout",           mTimeout           );
  tryToSet(data, "hostname",          mHostname          );
  tryToSet(data, "clientId",          mClientId          );
  tryToSet(data, "username",          mUsername          );
  tryToSet(data, "password",          mPassword          );
}

