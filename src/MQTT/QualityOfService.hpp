#pragma once

#include <cstdint>

namespace Rapatas::Transmitron::MQTT {

enum class QoS : uint8_t {
  AtLeastOnce = 0,
  AtMostOnce = 1,
  ExactlyOnce = 2
};

} // namespace Rapatas::Transmitron::MQTT
