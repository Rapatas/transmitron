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

  SubscriptionData(std::shared_ptr<MQTT::Subscription> sub);

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

  static wxColor colorFromString(const std::string &data);
};

}

#endif // TRANSMITRON_TYPES_SUBSCRIPTIONDATA_HPP
