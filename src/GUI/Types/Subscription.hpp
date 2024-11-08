#pragma once

#include <map>

#include <wx/colour.h>
#include <wx/event.h>

#include "MQTT/QualityOfService.hpp"
#include "MQTT/Subscription.hpp"

namespace Rapatas::Transmitron::GUI::Types {

class Subscription :
  public wxEvtHandler, //
  public MQTT::Subscription::Observer
{
public:

  explicit Subscription(const std::shared_ptr<MQTT::Subscription> &sub);
  Subscription(MQTT::Subscription::Id id, std::string filter, MQTT::QoS qos);

  // MQTT::Subscription::Observer interface.
  void onSubscribed() override;
  void onUnsubscribed() override;
  void onMessage(const MQTT::Message &message) override;

  void setMuted(bool muted);
  void setColor(const wxColor &color);
  void unsubscribe();

  [[nodiscard]] std::string getFilter() const;
  [[nodiscard]] wxColor getColor() const;
  [[nodiscard]] MQTT::QoS getQos() const;
  [[nodiscard]] bool getMuted() const;
  [[nodiscard]] size_t getId() const;

private:

  std::shared_ptr<MQTT::Subscription> mSub;
  bool mMuted;
  MQTT::Subscription::Id mId;
  std::string mFilter;
  MQTT::QoS mQos;
  wxColor mColor;

  static wxColor colorFromString(const std::string &data);
};

} // namespace Rapatas::Transmitron::GUI::Types
