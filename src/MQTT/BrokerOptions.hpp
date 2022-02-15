#ifndef MQTT_BROKEROPTIONS_HPP
#define MQTT_BROKEROPTIONS_HPP

#include <chrono>
#include <string>
#include <optional>
#include <nlohmann/json.hpp>

namespace MQTT
{

class BrokerOptions
{
public:

  explicit BrokerOptions();
  explicit BrokerOptions(
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
  );

  static BrokerOptions fromJson(const nlohmann::json &data);
  nlohmann::json toJson() const;

  bool getAutoReconnect() const;
  std::chrono::seconds getConnectTimeout() const;
  std::chrono::seconds getDisconnectTimeout() const;
  std::chrono::seconds getKeepAliveInterval() const;
  std::string getClientId() const;
  std::string getHostname() const;
  std::string getPassword() const;
  std::string getUsername() const;
  size_t getMaxInFlight() const;
  size_t getMaxReconnectRetries() const;
  size_t getPort() const;

private:

  bool mAutoReconnect;
  size_t mMaxInFlight;
  size_t mMaxReconnectRetries;
  size_t mPort;
  std::chrono::seconds mConnectTimeout;
  std::chrono::seconds mDisconnectTimeout;
  std::chrono::seconds mKeepAliveInterval;
  std::string mClientId;
  std::string mHostname;
  std::string mPassword;
  std::string mUsername;

};

}

#endif // MQTT_BROKEROPTIONS_HPP
