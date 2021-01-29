#ifndef TRANSMITRON_EVENTS_SUBSCRIPTION_HPP
#define TRANSMITRON_EVENTS_SUBSCRIPTION_HPP

#include <wx/event.h>
#include "Transmitron/Types/SubscriptionData.hpp"

namespace Transmitron::Events
{

class Subscription;
wxDECLARE_EVENT(SUB_SUBSCRIBED, Subscription);
wxDECLARE_EVENT(SUB_UNSUBSCRIBED, Subscription);

class Subscription : public wxCommandEvent
{
public:
  Subscription(wxEventType commandType, int id = 0) :
    wxCommandEvent(commandType, id)
  {}

  Subscription(const Subscription& event) :
    wxCommandEvent(event)
  {
    this->setSubscription(event.getSubscription());
    this->setMessage(event.getMessage());
  }

  wxEvent* Clone() const
  {
    return new Subscription(*this);
  }

  Transmitron::Types::SubscriptionData *getSubscription() const
  {
    return mSubscriptionData;
  }

  mqtt::const_message_ptr getMessage() const
  {
    return mMsg;
  }

  void setMessage(mqtt::const_message_ptr msg)
  {
    mMsg = msg;
  }

  void setSubscription(Transmitron::Types::SubscriptionData *subscriptionData)
  {
    mSubscriptionData = subscriptionData;
  }

private:

  Transmitron::Types::SubscriptionData *mSubscriptionData;
  mqtt::const_message_ptr mMsg;
};

typedef void (wxEvtHandler::*SubscriptionFunction)(Subscription &);
#define \
  SubscriptionHandler(func) \
  wxEVENT_HANDLER_CAST(SubscriptionFunction, func)

}

#endif // TRANSMITRON_EVENTS_SUBSCRIPTION_HPP
