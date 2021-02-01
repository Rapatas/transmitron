#ifndef MQTT_MESSAGE_HPP
#define MQTT_MESSAGE_HPP

#include <string>
#include "QualityOfService.hpp"
namespace MQTT
{

struct Message
{
  std::string topic;
  std::string payload;
  MQTT::QoS qos;
  bool retained;
};

}

#endif // MQTT_MESSAGE_HPP
