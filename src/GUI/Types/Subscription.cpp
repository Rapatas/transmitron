#include "Subscription.hpp"

#include "Common/Helpers.hpp"
#include "GUI/Events/Subscription.hpp"

using namespace Rapatas::Transmitron;
using namespace GUI::Types;
using namespace GUI;

Subscription::Subscription(const std::shared_ptr<MQTT::Subscription> &sub) :
  mSub(sub),
  mMuted(false),
  mId(mSub->getId()),
  mFilter(mSub->getFilter()),
  mQos(mSub->getQos()),
  mColor(colorFromString(mFilter)) //
{
  sub->attachObserver(this);
}

Subscription::Subscription(
  MQTT::Subscription::Id id,
  std::string filter,
  MQTT::QoS qos
) :
  mSub(nullptr),
  mMuted(false),
  mId(id),
  mFilter(std::move(filter)),
  mQos(qos),
  mColor(colorFromString(mFilter)) //
{}

void Subscription::onSubscribed() {
  auto *event = new Events::Subscription(Events::SUBSCRIPTION_SUBSCRIBED);
  event->setId(mId);
  wxQueueEvent(this, event);
}

void Subscription::onUnsubscribed() {
  auto *event = new Events::Subscription(Events::SUBSCRIPTION_UNSUBSCRIBED);
  event->setId(mId);
  wxQueueEvent(this, event);
}

void Subscription::onMessage(const MQTT::Message &message) {
  auto *event = new Events::Subscription(Events::SUBSCRIPTION_RECEIVED);
  event->setMessage(message);
  event->setId(mId);
  wxQueueEvent(this, event);
}

size_t Subscription::getId() const { return mId; }

void Subscription::setMuted(bool muted) { mMuted = muted; }

void Subscription::setColor(const wxColor &color) { mColor = color; }

void Subscription::unsubscribe() {
  if (mSub == nullptr) { return; }
  mSub->unsubscribe();
}

std::string Subscription::getFilter() const { return mFilter; }

wxColor Subscription::getColor() const { return mColor; }

MQTT::QoS Subscription::getQos() const { return mQos; }

bool Subscription::getMuted() const { return mMuted; }

wxColor Subscription::colorFromString(const std::string &data) {
  const size_t hash = std::hash<std::string>{}(data);
  return Common::Helpers::colorFromNumber(hash);
}
