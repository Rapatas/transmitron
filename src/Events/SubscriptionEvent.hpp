#ifndef TRANSMITRON_EVENTS_SUBSCRIPTION_HPP
#define TRANSMITRON_EVENTS_SUBSCRIPTION_HPP

#include <wx/event.h>
#include "Models/SubscriptionData.hpp"

class SubscriptionEvent;
wxDECLARE_EVENT(EVT_SUB_SUBSCRIBED, SubscriptionEvent);
wxDECLARE_EVENT(EVT_SUB_UNSUBSCRIBED, SubscriptionEvent);

class SubscriptionEvent : public wxCommandEvent
{
public:
  SubscriptionEvent(wxEventType commandType, int id = 0) :
    wxCommandEvent(commandType, id)
  {}

  SubscriptionEvent(const SubscriptionEvent& event) :
    wxCommandEvent(event)
  {
    this->setSubscription(event.getSubscription());
    this->setMessage(event.getMessage());
  }

  wxEvent* Clone() const
  {
    return new SubscriptionEvent(*this);
  }

  SubscriptionData *getSubscription() const
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

  void setSubscription(SubscriptionData *subscriptionData)
  {
    mSubscriptionData = subscriptionData;
  }

private:

  SubscriptionData *mSubscriptionData;
  mqtt::const_message_ptr mMsg;
};

typedef void (wxEvtHandler::*SubscriptionEventFunction)(SubscriptionEvent &);
#define \
  SubscriptionEventHandler(func) \
  wxEVENT_HANDLER_CAST(SubscriptionEventFunction, func)

#endif // TRANSMITRON_EVENTS_SUBSCRIPTION_HPP
