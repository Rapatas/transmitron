#include <chrono>
#include <iterator>
#include <thread>

#include <fmt/core.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include "Subscription.hpp"
#include "Client.hpp"
#include "Common/Log.hpp"

using namespace Rapatas::Transmitron::MQTT;

constexpr size_t CancelCheckIntervalMs = 50;
constexpr size_t ReconnectAfterMs = 2500;

// Public {

// Management {

Client::Client()
{
  mLogger = Common::Log::create("MQTT::Client");
}

size_t Client::attachObserver(Observer *observer)
{
  size_t id = 0;
  do {
    id = (size_t)std::abs(rand());
  } while (mObservers.find(id) != std::end(mObservers));

  return mObservers.insert(std::make_pair(id, observer)).first->first;
}

void Client::detachObserver(size_t id)
{
  mObservers.erase(id);
}

// Management }

// Actions {

void Client::connect()
{
  mCanceled = false;
  mShouldReconnect = false;
  mRetries = 0;
  const std::string address = fmt::format(
    "{}:{}",
    mBrokerOptions.getHostname(),
    mBrokerOptions.getPort()
  );
  mLogger->info("Connecting to {}", address);
  try
  {
    mClient = std::make_shared<mqtt::async_client>(
      address,
      mBrokerOptions.getClientId()
    );
    mClient->set_callback(*this);
    mClient->connect(mConnectOptions, nullptr, *this);
  }
  catch (const mqtt::exception& event)
  {
    mLogger->error("Connection failed: {}", event.what());
    exit(1);
  }
}

void Client::disconnect()
{
  if (mClient == nullptr) { return; }
  if (!mClient->is_connected()) { return; }

  mCanceled = false;
  mLogger->info("Disconnecting from {}", mBrokerOptions.getHostname());
  try
  {
    mClient->disconnect(
      mBrokerOptions.getDisconnectTimeout(),
      nullptr,
      *this
    );
    cleanSubscriptions();
  }
  catch (const mqtt::exception& exc)
  {
    mLogger->error("Disconnection failed: {}", exc.what());
    exit(1);
  }
}

void Client::cancel()
{
  mCanceled = true;
}

std::shared_ptr<Subscription> Client::subscribe(const std::string &topic)
{
  const auto it = std::find_if(
    std::begin(mSubscriptions),
    std::end(mSubscriptions),
    [topic](const auto &sub)
    {
      return sub.second->getFilter() == topic;
    }
  );

  if (it != std::end(mSubscriptions))
  {
    mLogger->info("Already subscribed!");
    return it->second;
  }

  mLogger->info("Creating subscription");
  ++mSubscriptionIds;
  auto sub = std::make_shared<Subscription>(
    mSubscriptionIds,
    topic,
    QoS::ExactlyOnce,
    shared_from_this()
  );
  sub->setState(Subscription::State::ToSubscribe);
  mSubscriptions.insert({mSubscriptionIds, sub});

  mLogger->info("Checking if connected");
  if (connected())
  {
    doSubscribe(mSubscriptionIds);
  }

  return sub;
}

void Client::unsubscribe(size_t id)
{
  const auto it = mSubscriptions.find(id);
  if (it == std::end(mSubscriptions)) { return; }
  mLogger->info("Unsubscribing from {}", it->second->getFilter());
  if (connected())
  {
    it->second->setState(Subscription::State::PendingUnsubscription);
    mClient->unsubscribe(it->second->getFilter(), nullptr, *this);
  }
  else
  {
    it->second->setState(Subscription::State::Unsubscribed);
    it->second->onUnsubscribed();
    mSubscriptions.erase(it);
  }
}

void Client::publish(
  const Message &message
) {
  if (!connected())
  {
    mLogger->warn("Could not publish: not connected");
    return;
  }

  mClient->publish(
    message.topic,
    message.payload.data(),
    message.payload.size(),
    (int)message.qos,
    message.retained,
    nullptr,
    *this
  );
}

// Actions }

// Setters {

void Client::setBrokerOptions(BrokerOptions brokerOptions)
{
  if (connected()) { return; }
  mBrokerOptions = std::move(brokerOptions);

  // Connect.
  mConnectOptions.set_clean_session(true);
  mConnectOptions.set_keep_alive_interval(mBrokerOptions.getKeepAliveInterval());
  mConnectOptions.set_connect_timeout(mBrokerOptions.getConnectTimeout());
  mConnectOptions.set_user_name(mBrokerOptions.getUsername());
  mConnectOptions.set_password(mBrokerOptions.getPassword());
}

// Setters }

// Getters {

const BrokerOptions &Client::brokerOptions() const
{
  return mBrokerOptions;
}

bool Client::connected() const
{
  return mClient && mClient->is_connected();
}

// Getters }

// Public }

// Private {

// mqtt::iaction_listener interface {

void Client::on_failure(const mqtt::token& tok)
{
  switch (tok.get_type()) {
    case mqtt::delivery_token::Type::CONNECT: {
      onFailureConnect(tok);
    } break;
    case mqtt::delivery_token::Type::DISCONNECT: {
      onFailureDisconnect(tok);
    } break;
    case mqtt::delivery_token::Type::PUBLISH: {
      onFailurePublish(tok);
    } break;
    case mqtt::delivery_token::Type::SUBSCRIBE: {
      onFailureSubscribe(tok);
    } break;
    case mqtt::delivery_token::Type::UNSUBSCRIBE: {
      onFailureUnsubscribe(tok);
    } break;
  }
}

void Client::on_success(const mqtt::token& tok)
{
  switch (tok.get_type()) {
    case mqtt::delivery_token::Type::CONNECT: {
      onSuccessConnect(tok);
    } break;
    case mqtt::delivery_token::Type::DISCONNECT: {
      onSuccessDisconnect(tok);
    } break;
    case mqtt::delivery_token::Type::PUBLISH: {
      onSuccessPublish(tok);
    } break;
    case mqtt::delivery_token::Type::SUBSCRIBE: {
      onSuccessSubscribe(tok);
    } break;
    case mqtt::delivery_token::Type::UNSUBSCRIBE: {
      onSuccessUnsubscribe(tok);
    } break;
  }
}

void Client::onSuccessConnect(const mqtt::token& /* tok */)
{
  mRetries = 0;
}

void Client::onSuccessDisconnect(const mqtt::token& /* tok */)
{
  for (const auto &[id, observer] : mObservers)
  {
    observer->onDisconnected();
  }
}

void Client::onSuccessPublish(const mqtt::token& /* tok */) {}

void Client::onSuccessSubscribe(const mqtt::token& tok)
{
  const auto it = std::find_if(
    std::begin(mSubscriptions),
    std::end(mSubscriptions),
    [&tok](const auto &sub)
    {
      return sub.second->getFilter() == tok.get_topics()->operator[](0);
    }
  );

  if (it == std::end(mSubscriptions))
  {
    mLogger->error("Unknown subscription ACK!");
    exit(1);
  }

  it->second->setState(Subscription::State::Subscribed);
  it->second->onSubscribed();

  mLogger->info("Subscribed to topics:");
  const auto size = tok.get_topics()->size();
  for (size_t i = 0; i != size; ++i)
  {
    mLogger->info("  - {}", tok.get_topics()->c_arr()[i]);
  }
}

void Client::onSuccessUnsubscribe(const mqtt::token& tok)
{
  const auto it = std::find_if(
    std::begin(mSubscriptions),
    std::end(mSubscriptions),
    [&tok](const auto &sub)
    {
      return sub.second->getFilter() == tok.get_topics()->operator[](0);
    }
  );

  if (it == std::end(mSubscriptions))
  {
    mLogger->error("Unknown unsubscription ACK!");
    exit(1);
  }

  mLogger->info(
    "Client::onSuccessUnsubscribe: {}",
    it->second->getFilter()
  );

  it->second->setState(Subscription::State::Unsubscribed);
  it->second->onUnsubscribed();
  mSubscriptions.erase(it);
}

void Client::onFailureConnect(const mqtt::token& tok)
{
  const auto code = tok.get_return_code();
  const auto codeIt = mReturnCodes.find(code);
  if (codeIt != std::end(mReturnCodes))
  {
    mLogger->warn("Connection attempt failed: {}", codeIt->second);
  }
  else
  {
    mLogger->warn("Connection attempt failed: {}", code);
  }
  reconnect();
}

void Client::onFailureDisconnect(const mqtt::token& tok)
{
  const auto code = tok.get_return_code();
  const auto codeIt = mReturnCodes.find(code);
  if (codeIt != std::end(mReturnCodes))
  {
    mLogger->warn("Disconnection attempt failed: {}", codeIt->second);
  }
  else
  {
    mLogger->warn("Disconnection attempt failed: {}", code);
  }
}

void Client::onFailurePublish(const mqtt::token& tok)
{
  const auto code = tok.get_return_code();
  const auto codeIt = mReturnCodes.find(code);
  if (codeIt != std::end(mReturnCodes))
  {
    mLogger->warn("Publishing attempt failed: {}", codeIt->second);
  }
  else
  {
    mLogger->warn("Publishing attempt failed: {}", code);
  }
}

void Client::onFailureSubscribe(const mqtt::token& tok)
{
  const auto size = tok.get_topics()->size();

  if (size == 0)
  {
    mLogger->warn("Subscription attempt failed: Response empty");
    return;
  }

  const auto topic = (*tok.get_topics())[size - 1];

  for (
    auto subIt = std::begin(mSubscriptions);
    subIt != std::end(mSubscriptions);
    ++subIt
  ) {
    if (Client::match(subIt->second->getFilter(), topic))
    {
      subIt->second->onUnsubscribed();
      mSubscriptions.erase(subIt);
      break;
    }
  }

  const auto code = tok.get_return_code();
  const auto codeIt = mReturnCodes.find(code);

  if (codeIt != std::end(mReturnCodes))
  {
    mLogger->warn("Subscription attempt failed: {}", codeIt->second);
  }
  else
  {
    mLogger->warn("Subscription attempt failed: {}", code);
  }
}

void Client::onFailureUnsubscribe(const mqtt::token& tok)
{
  const auto code = tok.get_return_code();
  const auto codeIt = mReturnCodes.find(code);
  if (codeIt != std::end(mReturnCodes))
  {
    mLogger->warn("Unsubscription attempt failed: {}", codeIt->second);
  }
  else
  {
    mLogger->warn("Unsubscription attempt failed: {}", code);
  }
}

// mqtt::iaction_listener interface }

// mqtt::callback interface {

void Client::connected(const std::string& /* cause */)
{
  mLogger->info("Connected!");
  mRetries = 0;
  mShouldReconnect = true;
  for (const auto &[id, observer] : mObservers)
  {
    observer->onConnected();
  }

  mLogger->info("Subscribing to topics:");

  for (const auto &sub : mSubscriptions)
  {
    doSubscribe(sub.first);
  }
}

void Client::connection_lost(const std::string& cause)
{
  mLogger->info("Connection lost: {}", cause);
  for (const auto &[id, observer] : mObservers)
  {
    observer->onConnectionLost();
  }
  cleanSubscriptions();
  reconnect();
}

void Client::message_arrived(mqtt::const_message_ptr msg)
{
  mLogger->info("Message received: {}", msg->get_topic());
  for (const auto &sub : mSubscriptions)
  {
    if (Client::match(sub.second->getFilter(), msg->get_topic()))
    {
      sub.second->onMessage(msg);
    }
  }
}

void Client::delivery_complete(mqtt::delivery_token_ptr token)
{
  mLogger->info("Delivery completed for {}", token->get_message()->get_topic());
}

// mqtt::callback interface }

void Client::reconnect()
{
  if (
    !mShouldReconnect
    || !mBrokerOptions.getAutoReconnect()
    || ++mRetries > mBrokerOptions.getMaxReconnectRetries()
  ) {
    for (const auto &[id, observer] : mObservers)
    {
      observer->onConnectionFailure();
    }
    return;
  }

  // Stop waiting quickly on cancel.
  using namespace std::chrono;
  const auto start = system_clock::now();
  while (
    !mCanceled
    && system_clock::now() - start < milliseconds(ReconnectAfterMs)
  ) {
    std::this_thread::sleep_for(milliseconds(CancelCheckIntervalMs));
  }

  if (mCanceled)
  {
    for (const auto &[id, observer] : mObservers)
    {
      observer->onDisconnected();
    }
    return;
  }

  mLogger->info(
    "Reconnecting attempt {}/{} in {}ms...",
    mRetries,
    mBrokerOptions.getMaxReconnectRetries(),
    ReconnectAfterMs
  );
  mShouldReconnect = true;
  try
  {
    mClient->connect(mConnectOptions, nullptr, *this);
  }
  catch (const mqtt::exception& exc)
  {
    mLogger->error("Error: {}", exc.what());
    exit(1);
  }
}

void Client::doSubscribe(size_t id)
{
  const auto it = mSubscriptions.find(id);
  if (it == std::end(mSubscriptions)) { return; }
  if (it->second->getState() != Subscription::State::ToSubscribe)
  {
    return;
  }

  mLogger->info(
    "Actually subscribing: {} ({})",
    it->second->getFilter(),
    (unsigned)it->second->getQos()
  );
  mClient->subscribe(
    it->second->getFilter(),
    (int)it->second->getQos(),
    nullptr,
    *this
  );
  it->second->setState(Subscription::State::PendingSubscription);
}

void Client::cleanSubscriptions()
{
  for (const auto &sub : mSubscriptions)
  {
    sub.second->setState(Subscription::State::ToSubscribe);
  }
}

// Static {

bool Client::match(const std::string &filter, const std::string &topic)
{
  const auto split = [](const std::string& str, char delim)
    -> std::vector<std::string> {
      std::vector<std::string> strings;
      size_t start = 0;
      size_t end = 0;
      while ((start = str.find_first_not_of(delim, end)) != std::string::npos) {
        end = str.find(delim, start);
        strings.push_back(str.substr(start, end - start));
      }
      return strings;
    };

  if (filter == topic)
  {
    return true;
  }

  const auto plusPos = filter.find('+');

  if (filter.back() == '#' && plusPos == std::string::npos)
  {
    auto root = filter.substr(0, filter.size() - 1);
    if (root.back() == '/')
    {
      root.pop_back();
    }
    return topic.rfind(root, 0) == 0;
  }

  if (plusPos == std::string::npos)
  {
    return false;
  }

  const auto filterLevels = split(filter, '/');
  const auto topicLevels  = split(topic,  '/');

  if (
    filterLevels.size() != topicLevels.size()
    && filter.back() != '#'
  ) {
    return false;
  }

  for (size_t i = 0; i != filterLevels.size(); ++i)
  {
    const bool levelMatches = topicLevels.size() >= filterLevels.size()
      && topicLevels.at(i) == filterLevels.at(i);

    const auto &level = filterLevels.at(i);
    if (level == "#")
    {
      return true;
    }

    if (level != "+" && !levelMatches)
    {
      return false;
    }
  }

  return true;
}

// Static }

// Private }
