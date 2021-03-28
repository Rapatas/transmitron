#include "Client.hpp"
#include <chrono>
#include <fmt/core.h>
#include <iterator>
#include <wx/log.h>
#include <thread>
#include "Subscription.hpp"

#define wxLOG_COMPONENT "mqtt/client" // NOLINT

using namespace MQTT;

constexpr size_t CancelCheckIntervalMs = 50;
constexpr size_t ReconnectAfterMs = 2500;

// Public {

// Management {

size_t Client::attachObserver(Observer *o)
{
  size_t id = 0;
  do {
    id = (size_t)std::abs(rand());
  } while (mObservers.find(id) != std::end(mObservers));

  return mObservers.insert(std::make_pair(id, o)).first->first;
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
  wxLogMessage("Connecting to %s", address);
  try
  {
    mClient = std::make_shared<mqtt::async_client>(
      address,
      mBrokerOptions.getClientId()
    );
    mClient->set_callback(*this);
    mClient->connect(mConnectOptions, nullptr, *this);
  }
  catch (const mqtt::exception& e)
  {
    wxLogError("Connection failed: %s", e.what());
    exit(1);
  }
}

void Client::disconnect()
{
  mCanceled = false;
  wxLogMessage("Disconnecting from %s", mBrokerOptions.getHostname());
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
    wxLogError("Disconnection failed: %s", exc.what());
    exit(1);
  }
}

void Client::cancel()
{
  mCanceled = true;
}

std::shared_ptr<Subscription> Client::subscribe(const std::string &topic)
{
  auto it = std::find_if(
    std::begin(mSubscriptions),
    std::end(mSubscriptions),
    [topic](const auto &sub)
    {
      return sub.second->getFilter() == topic;
    }
  );

  if (it != std::end(mSubscriptions))
  {
    wxLogMessage("Already subscribed!");
    return it->second;
  }

  wxLogMessage("Creating subscription");
  ++mSubscriptionIds;
  auto sub = std::make_shared<Subscription>(
    mSubscriptionIds,
    topic,
    QoS::ExactlyOnce,
    shared_from_this()
  );
  sub->setState(Subscription::State::Unsubscribed);
  mSubscriptions.insert({mSubscriptionIds, sub});

  wxLogMessage("Checking if connected");
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
  it->second->setState(Subscription::State::PendingUnsubscription);
  wxLogMessage("Unsubscribing from %s", it->second->getFilter());
  mClient->unsubscribe(it->second->getFilter(), nullptr, *this);
}

void Client::publish(
  const std::string &topic,
  const std::string &payload,
  QoS qos,
  bool retained
) {
  mClient->publish(
    topic,
    payload.data(),
    payload.size(),
    (int)qos,
    retained,
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
  for (const auto &o : mObservers)
  {
    o.second->onDisconnected();
  }
}

void Client::onSuccessPublish(const mqtt::token& /* tok */) {}

void Client::onSuccessSubscribe(const mqtt::token& tok)
{
  auto it = std::find_if(
    std::begin(mSubscriptions),
    std::end(mSubscriptions),
    [&tok](const auto &sub)
    {
      return sub.second->getFilter() == tok.get_topics()->operator[](0);
    }
  );

  if (it == std::end(mSubscriptions))
  {
    std::cerr << "Unknown subscription ACK!\n";
    exit(1);
  }

  it->second->setState(Subscription::State::Subscribed);
  it->second->onSubscribed();

  wxLogMessage("Subscribed to topics:");
  auto size = tok.get_topics()->size();
  for (size_t i = 0; i != size; ++i)
  {
    wxLogMessage("  - %s", tok.get_topics()->c_arr()[i]);
  }
}

void Client::onSuccessUnsubscribe(const mqtt::token& tok)
{
  auto it = std::find_if(
    std::begin(mSubscriptions),
    std::end(mSubscriptions),
    [&tok](const auto &sub)
    {
      return sub.second->getFilter() == tok.get_topics()->operator[](0);
    }
  );

  if (it == std::end(mSubscriptions))
  {
    std::cerr << "Unknown unsubscription ACK!\n";
    exit(1);
  }

  wxLogMessage(
    "Client::onSuccessUnsubscribe: %s",
    it->second->getFilter()
  );

  it->second->setState(Subscription::State::Unsubscribed);
  it->second->onUnsubscribed();
  mSubscriptions.erase(it);
}

void Client::onFailureConnect(const mqtt::token& tok)
{
  wxLogWarning(
    "Connection attempt failed: %s",
    mReturnCodes.at(tok.get_return_code()).c_str()
  );
  reconnect();
}

void Client::onFailureDisconnect(const mqtt::token& tok)
{
  wxLogWarning(
    "Disconnection attempt failed: %s",
    mReturnCodes.at(tok.get_return_code()).c_str()
  );
}

void Client::onFailurePublish(const mqtt::token& tok)
{
  wxLogWarning(
    "Publishing attempt failed: %s",
    mReturnCodes.at(tok.get_return_code()).c_str()
  );
}

void Client::onFailureSubscribe(const mqtt::token& tok)
{
  wxLogWarning(
    "Subscription attempt failed: %s",
    mReturnCodes.at(tok.get_return_code()).c_str()
  );
}

void Client::onFailureUnsubscribe(const mqtt::token& tok)
{
  wxLogWarning(
    "Unsubscription attempt failed: %s",
    mReturnCodes.at(tok.get_return_code()).c_str()
  );
}

// mqtt::iaction_listener interface }

// mqtt::callback interface {

void Client::connected(const std::string& /* cause */)
{
  wxLogMessage("Connected!");
  mRetries = 0;
  mShouldReconnect = true;
  for (const auto &o : mObservers)
  {
    o.second->onConnected();
  }

  wxLogMessage("Subscribing to topics:");

  for (const auto &sub : mSubscriptions)
  {
    doSubscribe(sub.first);
  }
}

void Client::connection_lost(const std::string& cause)
{
  wxLogMessage("Connection lost: %s", cause);
  for (const auto &o : mObservers)
  {
    o.second->onConnectionLost();
  }
  cleanSubscriptions();
  reconnect();
}

void Client::message_arrived(mqtt::const_message_ptr msg)
{
  wxLogMessage("Message received: %s", msg->get_topic());
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
  wxLogMessage("Delivery completed for %s", token->get_message()->get_topic());
}

// mqtt::callback interface }

void Client::reconnect()
{
  if (
    !mShouldReconnect
    || !mBrokerOptions.getAutoReconnect()
    || ++mRetries >= mBrokerOptions.getMaxReconnectRetries()
  ) {
    for (const auto &o : mObservers)
    {
      o.second->onConnectionFailure();
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
    for (const auto &o : mObservers)
    {
      o.second->onDisconnected();
    }
    return;
  }

  wxLogMessage(
    "Reconnecting attempt %zu/%zu in %zums...",
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
    std::cerr << "Error: " << exc.what() << std::endl;
    exit(1);
  }
}

void Client::doSubscribe(size_t id)
{
  auto it = mSubscriptions.find(id);
  if (it == std::end(mSubscriptions)) { return; }
  if (it->second->getState() == Subscription::State::Unsubscribed)
  {
    wxLogMessage(
      "Actually subscribing: %s (%d)",
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
}

void Client::cleanSubscriptions()
{
  for (const auto &sub : mSubscriptions)
  {
    sub.second->setState(Subscription::State::Unsubscribed);
  }
}

// Static {

bool Client::match(const std::string &filter, const std::string &topic)
{
  auto split = [](const std::string& str, char delim)
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
    if (level == '#')
    {
      return true;
    }

    if (level != '+' && !levelMatches)
    {
      return false;
    }
  }

  return true;
}

// Static }

// Private }
