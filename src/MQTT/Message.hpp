#pragma once

#include <chrono>
#include <string>

#include <nlohmann/json.hpp>

#include "QualityOfService.hpp"

namespace Rapatas::Transmitron::MQTT {

struct Message {
  std::string topic;
  std::string payload;
  MQTT::QoS qos = MQTT::QoS::AtLeastOnce;
  bool retained = false;
  std::chrono::system_clock::time_point timestamp;

  static Message fromJson(const nlohmann::json &data);
  [[nodiscard]] nlohmann::json toJson() const;
};

} // namespace Rapatas::Transmitron::MQTT
