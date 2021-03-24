#ifndef MQTT_MESSAGE_HPP
#define MQTT_MESSAGE_HPP

#include <string>
#include <nlohmann/json.hpp>
#include "QualityOfService.hpp"

namespace MQTT
{

struct Message
{
  std::string topic;
  std::string payload;
  MQTT::QoS qos = MQTT::QoS::AtLeastOnce;
  bool retained = false;

  static Message fromJson(const nlohmann::json &data);
  static nlohmann::json toJson(const Message &message);
};

}

#endif // MQTT_MESSAGE_HPP
