#pragma once

#include <map>
#include <string>

#include <mqtt/async_client.h>
#include <spdlog/spdlog.h>

#include "Client.hpp"

namespace Rapatas::Transmitron::MQTT {

struct Message;

class Subscription : public std::enable_shared_from_this<Subscription>
{
public:

  using Id = size_t;

  enum class State {
    ToSubscribe,
    PendingSubscription,
    Subscribed,
    PendingUnsubscription,
    Unsubscribed
  };

  struct Observer {
    Observer() = default;
    Observer(const Observer &other) = default;
    Observer(Observer &&other) = default;
    Observer &operator=(const Observer &other) = default;
    Observer &operator=(Observer &&other) = default;
    virtual ~Observer() = default;
    virtual void onSubscribed() = 0;
    virtual void onUnsubscribed() = 0;
    virtual void onMessage(const Message &message) = 0;
  };

  explicit Subscription(
    Id id,
    std::string filter,
    QoS qos,
    std::shared_ptr<Client> client
  );

  size_t attachObserver(Observer *observer);

  void unsubscribe();

  void onMessage(const mqtt::const_message_ptr &msg);
  void onUnsubscribed();
  void onSubscribed();

  std::string getFilter() const;
  State getState() const;
  QoS getQos() const;
  Id getId() const;

private:

  Id mId;
  std::string mFilter;
  QoS mQos;
  State mState = State::Unsubscribed;
  std::shared_ptr<Client> mClient;
  std::map<size_t, MQTT::Subscription::Observer *> mObservers;
  std::shared_ptr<spdlog::logger> mLogger;

  friend class Client; // Can set the state.
  void setState(State newState);
};

} // namespace Rapatas::Transmitron::MQTT
