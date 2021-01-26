#ifndef MQTT_QOS_HPP
#define MQTT_QOS_HPP

#include <cstdint>

namespace MQTT
{

enum class QoS : uint8_t
{
  AtLeastOnce = 0,
  AtMostOnce = 1,
  ExactlyOnce = 2
};

}

#endif // MQTT_QOS_HPP
