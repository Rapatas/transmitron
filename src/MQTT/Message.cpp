#include "Message.hpp"
#include "Helpers/Extract.hpp"

using namespace MQTT;

Message Message::fromJson(const nlohmann::json &data)
{
  Message result;

  auto iqos = Helpers::extract<unsigned>(data, "qos").value_or(0);
  if (iqos > 2)
  {
    iqos = 0;
  }

  result.topic = Helpers::extract<std::string>(data, "topic").value_or("");
  result.payload = Helpers::extract<std::string>(data, "payload").value_or("");
  result.qos = static_cast<QoS>(iqos);
  result.retained = Helpers::extract<bool>(data, "retained").value_or(false);

  return result;
}

nlohmann::json Message::toJson(const Message &message)
{
  return {
    {"topic", message.topic},
    {"payload", message.payload},
    {"qos", message.qos},
    {"retained", message.retained},
  };
}
