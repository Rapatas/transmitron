#ifndef TRANSMITRON_TYPES_SUBSCRIPTION_HPP
#define TRANSMITRON_TYPES_SUBSCRIPTION_HPP

#include <map>
#include <wx/colour.h>
#include <wx/event.h>
#include "MQTT/Subscription.hpp"

namespace Transmitron::Types
{

class Subscription :
  public wxEvtHandler,
  public MQTT::Subscription::Observer
{
public:

  Subscription(const std::shared_ptr<MQTT::Subscription> &sub);

  // MQTT::Subscription::Observer interface.
  void onSubscribed() override;
  void onUnsubscribed() override;
  void onMessage(const MQTT::Message &message) override;

  void setMuted(bool muted);
  void setColor(const wxColor &color);
  void unsubscribe();

  std::string getFilter() const;
  wxColor getColor() const;
  MQTT::QoS getQos() const;
  bool getMuted() const;
  size_t getId() const;

private:

  std::shared_ptr<MQTT::Subscription> mSub;
  wxColor mColor;
  bool mMuted;

  static wxColor colorFromString(const std::string &data);
};

}

#endif // TRANSMITRON_TYPES_SUBSCRIPTION_HPP
