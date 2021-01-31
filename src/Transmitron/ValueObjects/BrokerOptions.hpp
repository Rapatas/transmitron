#ifndef TRANSMITRON_VALUEOBJECTS_BROKEROPTIONS_HPP
#define TRANSMITRON_VALUEOBJECTS_BROKEROPTIONS_HPP

#include <string>
#include <optional>
#include <nlohmann/json.hpp>

namespace Transmitron::ValueObjects
{

class BrokerOptions
{
public:

  explicit BrokerOptions();
  explicit BrokerOptions(
    bool autoReconnect,
    std::string clientId,
    std::string hostname,
    std::string password,
    std::string username,
    unsigned keepAliveInterval,
    unsigned maxInFlight,
    unsigned port,
    unsigned timeout
  );
  virtual ~BrokerOptions() = default;

  static BrokerOptions fromJson(const nlohmann::json &data);
  nlohmann::json toJson() const;

  bool getAutoReconnect() const;
  std::string getClientId() const;
  std::string getHostname() const;
  std::string getPassword() const;
  std::string getUsername() const;
  unsigned getKeepAliveInterval() const;
  unsigned getMaxInFlight() const;
  unsigned getPort() const;
  unsigned getTimeout() const;

private:

  static const BrokerOptions defaults;

  bool mAutoReconnect;
  std::string mClientId;
  std::string mHostname;
  std::string mPassword;
  std::string mUsername;
  unsigned mKeepAliveInterval;
  unsigned mMaxInFlight;
  unsigned mPort;
  unsigned mTimeout;

  template<typename T>
  static std::optional<T> extract(
    const nlohmann::json &data,
    const std::string &key
  );
};

}

#endif // TRANSMITRON_VALUEOBJECTS_BROKEROPTIONS_HPP
