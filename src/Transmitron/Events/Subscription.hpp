#ifndef TRANSMITRON_EVENTS_SUBSCRIPTION_HPP
#define TRANSMITRON_EVENTS_SUBSCRIPTION_HPP

#include <wx/event.h>
#include "MQTT/Subscription.hpp"

namespace Transmitron::Events
{

class Subscription;
wxDECLARE_EVENT(SUBSCRIBED, Subscription);
wxDECLARE_EVENT(UNSUBSCRIBED, Subscription);
wxDECLARE_EVENT(RECEIVED, Subscription);

class Subscription : public wxCommandEvent
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

  wxEvent* Clone() const
  {
    return new Subscription(*this);
  }

  MQTT::Subscription::Id_t getId() const
  {
    return mId;
  }

  mqtt::const_message_ptr getMessage() const
  {
    return mMsg;
  }

  void setMessage(mqtt::const_message_ptr msg)
  {
    mMsg = std::move(msg);
  }

  void setId(MQTT::Subscription::Id_t id)
  {
    mId = id;
  }

private:

  MQTT::Subscription::Id_t mId;
  mqtt::const_message_ptr mMsg;
};

typedef void (wxEvtHandler::*SubscriptionFunction)(Subscription &);
#define \
  SubscriptionHandler(func) \
  wxEVENT_HANDLER_CAST(SubscriptionFunction, func)

}

#endif // TRANSMITRON_EVENTS_SUBSCRIPTION_HPP
