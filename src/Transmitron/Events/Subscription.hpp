#ifndef TRANSMITRON_EVENTS_SUBSCRIPTION_HPP
#define TRANSMITRON_EVENTS_SUBSCRIPTION_HPP

#include <wx/event.h>
#include "MQTT/Subscription.hpp"
#include "MQTT/Message.hpp"

namespace Transmitron::Events
{

class Subscription;
wxDECLARE_EVENT(SUBSCRIBED, Subscription);
wxDECLARE_EVENT(UNSUBSCRIBED, Subscription);
wxDECLARE_EVENT(RECEIVED, Subscription);

// NOLINTNEXTLINE
class Subscription :
  public wxCommandEvent
{
public:

  Subscription(wxEventType commandType, int id = 0) :
    wxCommandEvent(commandType, id)
  {}

  Subscription(const Subscription& event) :
    wxCommandEvent(event)
  {
    this->setId(event.getId());
    this->setMessage(event.getMessage());
  }

  wxEvent* Clone() const override
  {
    return new Subscription(*this);
  }

  MQTT::Subscription::Id_t getId() const
  {
    return mId;
  }

  MQTT::Message getMessage() const
  {
    return mMsg;
  }

  void setMessage(MQTT::Message msg)
  {
    mMsg = std::move(msg);
  }

  void setId(MQTT::Subscription::Id_t id)
  {
    mId = id;
  }

private:

  MQTT::Message mMsg;
  MQTT::Subscription::Id_t mId = 0;
};

}

#endif // TRANSMITRON_EVENTS_SUBSCRIPTION_HPP
