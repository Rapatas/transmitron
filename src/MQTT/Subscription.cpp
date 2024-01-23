#include <chrono>
#include <cstdlib>

#include "Subscription.hpp"
#include "Message.hpp"
#include "Common/Log.hpp"

using namespace Rapatas::Transmitron;
using namespace MQTT;

Subscription::Subscription(
  Id_t id,
  const std::string &filter,
  QoS qos,
  std::shared_ptr<Client> client
) :
  mId(id),
  mFilter(filter),
  mQos(qos),
  mState(State::Unsubscribed),
  mClient(std::move(client)),
  mLogger(Common::Log::create("MQTT::Subscription"))
{}

size_t Subscription::attachObserver(Observer *observer)
{
  static size_t id = 0;

  if (mState == State::Subscribed)
  {
    onSubscribed();
  }

  return mObservers.insert(std::make_pair(id++, observer)).first->first;
}

void Subscription::unsubscribe()
{
  switch (mState)
  {
    case State::ToSubscribe:
    case State::PendingSubscription:
    case State::Subscribed:
    {
      mClient->unsubscribe(mId);
    } break;
    default: {}
  }
}

void Subscription::onSubscribed()
{
  for (const auto &[id, observer] : mObservers)
  {
    observer->onSubscribed();
  }
}

void Subscription::onUnsubscribed()
{
  for (const auto &[id, observer] : mObservers)
  {
    observer->onUnsubscribed();
  }
}

void Subscription::onMessage(
  const mqtt::const_message_ptr &msg
) {
  for (const auto &[id, observer] : mObservers)
  {
    const std::string payload {
      std::begin(msg->get_payload()),
      std::end(msg->get_payload())
    };

    QoS qos = QoS::AtLeastOnce;
    switch (msg->get_qos())
    {
      case 0: qos = MQTT::QoS::AtLeastOnce; break;
      case 1: qos = MQTT::QoS::AtMostOnce;  break;
      case 2: qos = MQTT::QoS::ExactlyOnce; break;
    }

    const auto timestamp = std::chrono::system_clock::now();

    const Message message {
      msg->get_topic(),
      payload,
      qos,
      msg->is_retained(),
      timestamp,
    };

    observer->onMessage(message);
  }
}

std::string Subscription::getFilter() const
{
  return mFilter;
}

QoS Subscription::getQos() const
{
  return mQos;
}

Subscription::State Subscription::getState() const
{
  return mState;
}

Subscription::Id_t Subscription::getId() const
{
  return mId;
}

void Subscription::setState(State newState)
{
  mState = newState;
}
