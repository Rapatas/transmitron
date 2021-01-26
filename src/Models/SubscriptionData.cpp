#include "SubscriptionData.hpp"
#include "Events/SubscriptionEvent.hpp"

wxDEFINE_EVENT(EVT_SUB_SUBSCRIBED, SubscriptionEvent);
wxDEFINE_EVENT(EVT_SUB_UNSUBSCRIBED, SubscriptionEvent);

SubscriptionData::SubscriptionData(std::shared_ptr<MQTT::Subscription> sub) :
  mSub(sub),
  mMuted(false)
{
  mColor = colorFromString(sub->getFilter());
  sub->attachObserver(this);
}

size_t SubscriptionData::attachObserver(Observer *o)
{
  size_t id = 0;
  do {
    id = rand();
  } while (mObservers.find(id) != std::end(mObservers));

  return mObservers.insert(std::make_pair(id, o)).first->first;
}

void SubscriptionData::onSubscribed()
{
  auto e = new SubscriptionEvent(EVT_SUB_SUBSCRIBED);
  e->setSubscription(this);
  wxQueueEvent(this, e);
}

void SubscriptionData::onUnsubscribed()
{
  auto e = new SubscriptionEvent(EVT_SUB_UNSUBSCRIBED);
  e->setSubscription(this);
  wxQueueEvent(this, e);
}

void SubscriptionData::onMessage(mqtt::const_message_ptr msg)
{
  for (const auto &o : mObservers)
  {
    o.second->onMessage(this, msg);
  }
}

void SubscriptionData::setMuted(bool muted)
{
  mMuted = muted;
}

void SubscriptionData::setColor(const wxColor &color)
{
  mColor = color;
}

void SubscriptionData::unsubscribe()
{
  mSub->unsubscribe();
}

std::string SubscriptionData::getFilter() const
{
  return mSub->getFilter();
}

wxColor SubscriptionData::getColor() const
{
  return mColor;
}

MQTT::QoS SubscriptionData::getQos() const
{
  return mSub->getQos();
}

bool SubscriptionData::getMuted() const
{
  return mMuted;
}

wxColor SubscriptionData::colorFromString(const std::string &data)
{
  size_t x = std::hash<std::string>{}(data);
  uint8_t r = ((x >> 0)  & 0xFF);
  uint8_t g = ((x >> 8)  & 0xFF);
  uint8_t b = ((x >> 16) & 0xFF);
  r = (r - 0) / (float)(255 - 0) * (255 - 100) + 100;
  g = (g - 0) / (float)(255 - 0) * (255 - 100) + 100;
  b = (b - 0) / (float)(255 - 0) * (255 - 100) + 100;
  return wxColor(r, g, b);
}
