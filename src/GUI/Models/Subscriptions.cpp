#include "Subscriptions.hpp"

#include <algorithm>
#include <cstddef>
#include <cstring>
#include <fstream>
#include <iterator>
#include <memory>

#include <wx/dcmemory.h>

#include "Common/Filesystem.hpp"
#include "Common/Log.hpp"
#include "GUI/Events/Subscription.hpp"
#include "GUI/Resources/qos/qos-0.hpp"
#include "GUI/Resources/qos/qos-1.hpp"
#include "GUI/Resources/qos/qos-2.hpp"
#include "GUI/Types/Subscription.hpp"
#include "MQTT/Subscription.hpp"

using namespace Rapatas::Transmitron;
using namespace GUI::Models;
using namespace Common;

Subscriptions::Subscriptions(std::shared_ptr<MQTT::Client> client) :
  mClient(std::move(client)) //
{
  mLogger = Log::create("Models::Subscriptions");
}

Subscriptions::Subscriptions() {
  mLogger = Log::create("Models::Subscriptions");
}

size_t Subscriptions::attachObserver(Observer *observer) {
  static size_t id = 0;
  return mObservers.insert(std::make_pair(id++, observer)).first->first;
}

bool Subscriptions::detachObserver(size_t id) {
  auto it = mObservers.find(id);
  if (it == std::end(mObservers)) { return false; }

  mObservers.erase(it);
  return true;
}

bool Subscriptions::load(const std::string &recording) {
  if (recording.empty()) {
    mLogger->error("No file provided");
    return false;
  }

  const bool exists = fs::exists(recording);
  if (!exists) {
    mLogger->warn("File does not exist: {}", recording);
    return false;
  }

  std::ifstream input(recording);
  if (!input.is_open()) {
    const auto ec = std::error_code(errno, std::system_category());
    mLogger->warn("Could not open '{}': {}", recording, ec.message());
    return false;
  }

  std::stringstream buffer;
  buffer << input.rdbuf();
  if (!nlohmann::json::accept(buffer.str())) {
    mLogger->warn("Could not parse '{}'", recording);
    return false;
  }
  const auto data = nlohmann::json::parse(buffer.str());

  const auto subscriptionsIt = data.find("subscriptions");
  if (subscriptionsIt == std::end(data)) {
    mLogger->warn("Could not find key 'subscriptions' in '{}'", recording);
    return false;
  }
  if (!subscriptionsIt->is_array()) {
    mLogger->warn("Key 'subscriptions' is not an array");
    return false;
  }

  for (const auto &sub : *subscriptionsIt) {
    MQTT::Subscription::Id_t id{};
    std::string filter;
    MQTT::QoS qos = MQTT::QoS::AtLeastOnce;

    const auto idIt = sub.find("id");
    if (false // NOLINT
        || idIt == std::end(sub) || !idIt->is_number_unsigned()) {
      mLogger->warn("Subscription is missing id");
      return false;
    }
    id = *idIt;

    const auto filterIt = sub.find("filter");
    if (true // NOLINT
        && filterIt != std::end(sub) && filterIt->is_string()) {
      filter = *filterIt;
    }

    const auto qosIt = sub.find("qos");
    if (true // NOLINT
        && qosIt != std::end(sub) && qosIt->is_number_unsigned()) {
      qos = *qosIt;
    }

    auto subscription = std::make_unique<Types::Subscription>(id, filter, qos);
    mSubscriptions.emplace(id, std::move(subscription));
    mRemap.push_back(id);
  }

  mLogger->info("Loaded {} subscriptions", mSubscriptions.size());

  return true;
}

nlohmann::json Subscriptions::toJson() const {
  nlohmann::json result;

  for (const auto &[id, subscription] : mSubscriptions) {
    result.push_back({
      {"id", id},
      {"filter", subscription->getFilter()},
      {"qos", subscription->getQos()},
    });
  }

  return result;
}

std::string Subscriptions::getFilter(wxDataViewItem item) const {
  const auto &sub = mSubscriptions.at(mRemap.at(GetRow(item)));
  return sub->getFilter();
}

std::string Subscriptions::getFilter(MQTT::Subscription::Id_t subscriptionId
) const {
  const auto &sub = mSubscriptions.at(subscriptionId);
  return sub->getFilter();
}

bool Subscriptions::getMuted(wxDataViewItem item) const {
  const auto &sub = mSubscriptions.at(mRemap.at(GetRow(item)));
  return sub->getMuted();
}

bool Subscriptions::getMuted(MQTT::Subscription::Id_t subscriptionId) const {
  const auto &sub = mSubscriptions.at(subscriptionId);
  return sub->getMuted();
}

wxColor Subscriptions::getColor(MQTT::Subscription::Id_t subscriptionId) const {
  const auto &sub = mSubscriptions.at(subscriptionId);
  return sub->getColor();
}

void Subscriptions::setColor(wxDataViewItem item, const wxColor &color) {
  auto &sub = mSubscriptions.at(mRemap.at(GetRow(item)));
  sub->setColor(color);
  for (const auto &[id, observer] : mObservers) {
    observer->onColorSet(sub->getId(), color);
  }
  ValueChanged(item, static_cast<unsigned>(Column::Icon));
}

void Subscriptions::unmute(wxDataViewItem item) {
  auto &sub = mSubscriptions.at(mRemap.at(GetRow(item)));
  sub->setMuted(false);
  for (const auto &[id, observer] : mObservers) {
    observer->onUnmuted(sub->getId());
  }
  ItemChanged(item);
}

void Subscriptions::clear(wxDataViewItem item) {
  auto &sub = mSubscriptions.at(mRemap.at(GetRow(item)));
  for (const auto &[id, observer] : mObservers) {
    observer->onCleared(sub->getId());
  }
  ItemChanged(item);
}

void Subscriptions::mute(wxDataViewItem item) {
  auto &sub = mSubscriptions.at(mRemap.at(GetRow(item)));
  sub->setMuted(true);
  for (const auto &[id, observer] : mObservers) {
    observer->onMuted(sub->getId());
  }
  ItemChanged(item);
}

void Subscriptions::solo(wxDataViewItem item) {
  for (auto &subscription : mSubscriptions) {
    subscription.second->setMuted(true);
  }

  wxDataViewItemArray array;
  array.reserve(mRemap.size());
  for (uint32_t i = 0; i != mRemap.size(); ++i) { array.push_back(GetItem(i)); }
  ItemsChanged(array);

  auto &sub = mSubscriptions.at(mRemap.at(GetRow(item)));
  sub->setMuted(false);
  for (const auto &[id, observer] : mObservers) {
    observer->onSolo(sub->getId());
  }
  ItemChanged(item);
}

void Subscriptions::subscribe(const std::string &topic, MQTT::QoS /* qos */) {
  if (mClient == nullptr) { return; }

  auto it = std::find_if(
    std::begin(mSubscriptions),
    std::end(mSubscriptions),
    [&topic](auto &sub) { return sub.second->getFilter() == topic; }
  );

  if (it != std::end(mSubscriptions)) {
    mLogger->info("Already subscribed!");
    return;
  }

  auto mqttSubscription = mClient->subscribe(topic);
  auto sub = std::make_unique<Types::Subscription>(mqttSubscription);
  const auto id = sub->getId();
  sub->Bind(
    Events::SUBSCRIPTION_SUBSCRIBED,
    &Subscriptions::onSubscribed,
    this //
  );
  sub->Bind(
    Events::SUBSCRIPTION_UNSUBSCRIBED,
    &Subscriptions::onUnsubscribed,
    this
  );
  sub->Bind(Events::SUBSCRIPTION_RECEIVED, &Subscriptions::onMessage, this);
  mSubscriptions.insert({id, std::move(sub)});
  mRemap.push_back(id);
  mLogger->info("RowAppended");
  RowAppended();
}

void Subscriptions::unsubscribe(wxDataViewItem item) {
  auto &sub = mSubscriptions.at(mRemap.at(GetRow(item)));
  sub->unsubscribe();
}

unsigned Subscriptions::GetColumnCount() const {
  return static_cast<unsigned>(Column::Max);
}

unsigned Subscriptions::GetCount() const {
  return static_cast<unsigned>(mSubscriptions.size());
}

wxString Subscriptions::GetColumnType(unsigned int col) const {
  switch (static_cast<Column>(col)) {
    case Column::Icon: {
      return "wxColor";
    } break;
    case Column::Qos: {
      return wxDataViewBitmapRenderer::GetDefaultType();
    } break;
    case Column::Topic: {
      return wxDataViewTextRenderer::GetDefaultType();
    } break;
    default: {
      return "string";
    }
  }
}

void Subscriptions::GetValueByRow(
  wxVariant &variant,
  unsigned int row,
  unsigned int col
) const {
  const auto &sub = mSubscriptions.at(mRemap.at(row));

  constexpr size_t SubscriptionIconHeight = 10;
  constexpr size_t SubscriptionIconWidth = 20;

  switch (static_cast<Column>(col)) {
    case Column::Icon: {
      wxBitmap bitmap(SubscriptionIconHeight, SubscriptionIconWidth);
      wxMemoryDC mem;
      mem.SelectObject(bitmap);
      mem.SetBackground(wxBrush(sub->getColor()));
      mem.Clear();
      mem.SelectObject(wxNullBitmap);

      variant << bitmap;
    } break;
    case Column::Topic: {
      const auto utf8 = sub->getFilter();
      const auto wxs = wxString::FromUTF8(utf8.data(), utf8.length());
      variant = wxs;
    } break;
    case Column::Qos: {
      const wxBitmap *result = nullptr;
      switch (sub->getQos()) {
        case MQTT::QoS::AtLeastOnce: {
          result = bin2cQos0();
        } break;
        case MQTT::QoS::AtMostOnce: {
          result = bin2cQos1();
        } break;
        case MQTT::QoS::ExactlyOnce: {
          result = bin2cQos2();
        } break;
      }
      variant << *result;
    } break;
    default: {
    }
  }
}

bool Subscriptions::GetAttrByRow(
  unsigned int /* row */,
  unsigned int /* col */,
  wxDataViewItemAttr & /* attr */
) const {
  return false;
}

bool Subscriptions::SetValueByRow(
  const wxVariant & /* variant */,
  unsigned int /* row */,
  unsigned int /* col */
) {
  return false;
}

void Subscriptions::onSubscribed(Events::Subscription & /* event */) {}

void Subscriptions::onUnsubscribed(Events::Subscription &event) {
  const auto id = event.getSubscriptionId();
  const auto it = std::find(std::begin(mRemap), std::end(mRemap), id);
  if (it == std::end(mRemap)) {
    mLogger->error("Could not find subscription");
    return;
  }
  const auto index = static_cast<size_t>(std::distance(std::begin(mRemap), it));
  for (const auto &[id, observer] : mObservers) {
    observer->onUnsubscribed(id);
  }
  mSubscriptions.erase(id);
  mRemap.erase(it);
  RowDeleted(static_cast<unsigned>(index));
}

void Subscriptions::onMessage(Events::Subscription &event) {
  for (const auto &[id, observer] : mObservers) {
    observer->onMessage(event.getSubscriptionId(), event.getMessage());
  }
}
