#ifndef MQTT_MESSAGE_HPP
#define MQTT_MESSAGE_HPP

#include <chrono>
#include <string>
#include <nlohmann/json.hpp>
#include "QualityOfService.hpp"
#include "Subscription.hpp"

namespace MQTT
{

struct Message
{
  std::string topic;
  std::string payload;
  MQTT::QoS qos = MQTT::QoS::AtLeastOnce;
  bool retained = false;
  Subscription::Id_t subscriptionId = 0;
  std::chrono::system_clock::time_point timestamp;

  static Message fromJson(const nlohmann::json &data);
  static nlohmann::json toJson(const Message &message);
};

}

#endif // MQTT_MESSAGE_HPP
