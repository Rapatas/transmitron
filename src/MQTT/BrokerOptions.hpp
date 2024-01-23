#pragma once

#include <random>
#include <chrono>
#include <string>
#include <optional>
#include <nlohmann/json.hpp>

namespace Rapatas::Transmitron::MQTT
{

class BrokerOptions
{
public:

  static constexpr bool DefaultAutoReconnect = false;
  static constexpr const std::string_view DefaultHostname = "127.0.0.1";
  static constexpr const std::string_view DefaultPassword {};
  static constexpr const std::string_view DefaultUsername {};
  static constexpr std::chrono::seconds DefaultTimeout { 5 };
  static constexpr std::chrono::seconds DefaultKeepAliveInterval { 60 };
  static constexpr size_t DefaultMaxReconnectRetries = 10;
  static constexpr size_t DefaultMaxInFlight = 10;
  static constexpr size_t DefaultPort = 1883;

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
  [[nodiscard]] nlohmann::json toJson() const;

  [[nodiscard]] bool getAutoReconnect() const;
  [[nodiscard]] std::chrono::seconds getConnectTimeout() const;
  [[nodiscard]] std::chrono::seconds getDisconnectTimeout() const;
  [[nodiscard]] std::chrono::seconds getKeepAliveInterval() const;
  [[nodiscard]] std::string getClientId() const;
  [[nodiscard]] std::string getHostname() const;
  [[nodiscard]] std::string getPassword() const;
  [[nodiscard]] std::string getUsername() const;
  [[nodiscard]] size_t getMaxInFlight() const;
  [[nodiscard]] size_t getMaxReconnectRetries() const;
  [[nodiscard]] size_t getPort() const;

  void setHostname(std::string hostname);
  void setPort(size_t port);

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

} // namespace Rapatas::Transmitron::MQTT
