#ifndef TRANSMITRON_TYPES_SUBSCRIPTIONDATA_HPP
#define TRANSMITRON_TYPES_SUBSCRIPTIONDATA_HPP

#include <map>
#include <wx/colour.h>
#include <wx/event.h>
#include "MQTT/Subscription.hpp"

namespace Transmitron::Types
{

class SubscriptionData :
  public wxEvtHandler,
  public MQTT::Subscription::Observer
{
public:

  struct Observer
  {
    virtual void onMessage(
      SubscriptionData *subscriptionData,
      mqtt::const_message_ptr msg
    ) = 0;
  };

  SubscriptionData(std::shared_ptr<MQTT::Subscription> sub);

  size_t attachObserver(Observer *o);

  // MQTT::Subscription::Observer interface.
  void onSubscribed() override;
  void onUnsubscribed() override;
  void onMessage(mqtt::const_message_ptr msg) override;

  void setMuted(bool muted);
  void setColor(const wxColor &color);
  void unsubscribe();

  std::string getFilter() const;
  wxColor getColor() const;
  MQTT::QoS getQos() const;
  bool getMuted() const;

private:

  std::shared_ptr<MQTT::Subscription> mSub;
  wxColor mColor;
  bool mMuted;
  std::map<size_t, Observer*> mObservers;

  static wxColor colorFromString(const std::string &data);
};

}

#endif // TRANSMITRON_TYPES_SUBSCRIPTIONDATA_HPP
