#pragma once

#include <map>
#include <memory>

#include <mqtt/async_client.h>
#include <mqtt/disconnect_options.h>
#include <spdlog/spdlog.h>

#include "BrokerOptions.hpp"
#include "Message.hpp"

namespace Rapatas::Transmitron::MQTT {

class Subscription;

class Client :
  public virtual mqtt::iaction_listener,
  public virtual mqtt::callback,
  public std::enable_shared_from_this<Client>
{
public:

  using SubscriptionId = size_t;

  struct Observer {
    Observer() = default;
    Observer(const Observer &other) = default;
    Observer(Observer &&other) = default;
    Observer &operator=(const Observer &other) = default;
    Observer &operator=(Observer &&other) = default;
    virtual ~Observer() = default;

    virtual void onConnected() = 0;
    virtual void onDisconnected() = 0;
    virtual void onConnectionFailure() = 0;
    virtual void onConnectionLost() = 0;
  };

  explicit Client();
  size_t attachObserver(Observer *observer);
  void detachObserver(size_t id);

  // Actions.
  void connect();
  void disconnect();
  void cancel();
  void publish(const Message &message);
  std::shared_ptr<Subscription> subscribe(const std::string &topic);
  void unsubscribe(size_t id);

  // Setters.
  void setBrokerOptions(BrokerOptions brokerOptions);

  // Getters.
  const BrokerOptions &brokerOptions() const;
  bool connected() const;

private:

  std::shared_ptr<spdlog::logger> mLogger;
  BrokerOptions mBrokerOptions;
  SubscriptionId mSubscriptionIds = 0;
  bool mShouldReconnect = false;
  bool mCanceled = false;
  mqtt::connect_options mConnectOptions;
  size_t mRetries = 0;
  std::map<SubscriptionId, std::shared_ptr<Subscription>> mSubscriptions;
  std::map<size_t, MQTT::Client::Observer *> mObservers;
  std::shared_ptr<mqtt::async_client> mClient;

  // mqtt::iaction_listener interface.
  void on_success(const mqtt::token &tok) override;
  void on_failure(const mqtt::token &tok) override;

  // mqtt::callback interface.
  void connected(const std::string &cause) override;
  void connection_lost(const std::string &cause) override;
  void message_arrived(mqtt::const_message_ptr msg) override;
  void delivery_complete(mqtt::delivery_token_ptr token) override;

  void onSuccessConnect(const mqtt::token &tok);
  void onSuccessDisconnect(const mqtt::token &tok);
  void onSuccessPublish(const mqtt::token &tok);
  void onSuccessSubscribe(const mqtt::token &tok);
  void onSuccessUnsubscribe(const mqtt::token &tok);
  void onFailureConnect(const mqtt::token &tok);
  void onFailureDisconnect(const mqtt::token &tok);
  void onFailurePublish(const mqtt::token &tok);
  void onFailureSubscribe(const mqtt::token &tok);
  void onFailureUnsubscribe(const mqtt::token &tok);

  void reconnect();
  void doSubscribe(size_t id);
  void cleanSubscriptions();

  static const std::map<int, std::string> &codeDescriptions();
  static bool match(const std::string &filter, const std::string &topic);
  static std::string codeToStr(int code);
};

} // namespace Rapatas::Transmitron::MQTT
