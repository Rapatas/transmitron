#pragma once

#include <wx/event.h>

#include "MQTT/Message.hpp"
#include "MQTT/Subscription.hpp"

namespace Rapatas::Transmitron::GUI::Events {

class Subscription;
wxDECLARE_EVENT(SUBSCRIPTION_SUBSCRIBED, Subscription);
wxDECLARE_EVENT(SUBSCRIPTION_UNSUBSCRIBED, Subscription);
wxDECLARE_EVENT(SUBSCRIPTION_RECEIVED, Subscription);

// NOLINTNEXTLINE
class Subscription : public wxCommandEvent
{
public:

  explicit Subscription(wxEventType commandType, int id = 0) :
    wxCommandEvent(commandType, id) //
  {}

  Subscription(const Subscription &event) :
    wxCommandEvent(event) {
    this->setId(event.getSubscriptionId());
    this->setMessage(event.getMessage());
  }

  [[nodiscard]] wxEvent *Clone() const override {
    return new Subscription(*this);
  }

  [[nodiscard]] MQTT::Subscription::Id getSubscriptionId() const {
    return mId;
  }

  [[nodiscard]] MQTT::Message getMessage() const { return mMsg; }

  void setMessage(MQTT::Message msg) { mMsg = std::move(msg); }

  void setId(MQTT::Subscription::Id id) { mId = id; }

private:

  MQTT::Message mMsg;
  MQTT::Subscription::Id mId = 0;
};

} // namespace Rapatas::Transmitron::GUI::Events
