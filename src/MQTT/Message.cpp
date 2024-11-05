#include "Message.hpp"

#include "Common/Extract.hpp"

using namespace Rapatas::Transmitron;
using namespace MQTT;

Message Message::fromJson(const nlohmann::json &data) {
  Message result;

  auto iqos = Common::extract<unsigned>(data, "qos").value_or(0);
  if (iqos > 2) { iqos = 0; }

  result.topic = Common::extract<std::string>(data, "topic").value_or("");
  result.payload = Common::extract<std::string>(data, "payload").value_or("");
  result.qos = static_cast<QoS>(iqos);
  result.retained = Common::extract<bool>(data, "retained").value_or(false);

  return result;
}

nlohmann::json Message::toJson() const {
  return {
    {"topic", topic},
    {"payload", payload},
    {"qos", qos},
    {"retained", retained},
  };
}
