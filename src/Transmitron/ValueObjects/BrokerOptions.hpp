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

  explicit BrokerOptions(
    bool autoReconnec,
    std::string clientI,
    std::string hostnam,
    std::string passwor,
    std::string usernam,
    unsigned keepAliveInterva,
    unsigned maxInFligh,
    unsigned por,
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
  ) {
    auto it = data.find(key);
    if (it == std::end(data)) { return std::nullopt; }

    if (std::is_same<unsigned, T>::value)
    {
      if (it->type() != nlohmann::json::value_t::number_unsigned) { return std::nullopt; }
      return it->get<unsigned>();
    }
    else if (std::is_same<std::string, T>::value)
    {
      if (it->type() != nlohmann::json::value_t::string) { return std::nullopt; }
      return it->get<std::string>();
    }
    else if (std::is_same<bool, T>::value)
    {
      if (it->type() != nlohmann::json::value_t::boolean) { return std::nullopt; }
      return it->get<bool>();
    }

    return std::nullopt;
  }
};

}

#endif // TRANSMITRON_VALUEOBJECTS_BROKEROPTIONS_HPP
