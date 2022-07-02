#include <filesystem>
#include <fstream>
#include <wx/dcmemory.h>
#include "Common/Helpers.hpp"
#include "History.hpp"
#include "Common/Log.hpp"
#include "MQTT/Message.hpp"
#include "Transmitron/Resources/pin/pinned-18x18.hpp"
#include "Transmitron/Resources/pin/not-pinned-18x18.hpp"
#include "Transmitron/Resources/qos/qos-0.hpp"
#include "Transmitron/Resources/qos/qos-1.hpp"
#include "Transmitron/Resources/qos/qos-2.hpp"

namespace fs = std::filesystem;
using namespace Transmitron::Models;
using namespace Transmitron;

History::History(const wxObjectDataPtr<Subscriptions> &subscriptions) :
  mSubscriptions(subscriptions)
{
  mLogger = Common::Log::create("Models::History");
  mSubscriptions->attachObserver(this);
}

size_t History::attachObserver(Observer *observer)
{
  size_t id = 0;
  do {
    id = (size_t)std::abs(rand());
  } while (mObservers.find(id) != std::end(mObservers));

  return mObservers.insert(std::make_pair(id, observer)).first->first;
}

bool History::detachObserver(size_t id)
{
  auto it = mObservers.find(id);
  if (it == std::end(mObservers))
  {
    return false;
  }

  mObservers.erase(it);
  return true;
}

void History::clear()
{
  mMessages.clear();
  remap();
}

bool History::load(const std::string &recording)
{
  if (recording.empty())
  {
    mLogger->error("No file provided");
    return false;
  }

  const bool exists = fs::exists(recording);
  if (!exists)
  {
    mLogger->warn("File does not exist: {}", recording);
    return false;
  }

  std::ifstream input(recording);
  if (!input.is_open())
  {
    mLogger->warn("Could not open '{}': {}", recording, std::strerror(errno));
    return false;
  }

  std::stringstream buffer;
  buffer << input.rdbuf();
  if (!nlohmann::json::accept(buffer.str()))
  {
    mLogger->warn("Could not parse '{}'", recording);
    return false;
  }
  const auto data = nlohmann::json::parse(buffer.str());

  const auto messagesIt = data.find("messages");
  if (messagesIt == std::end(data))
  {
    mLogger->warn("Could not find key 'messages' in '{}'", recording);
    return false;
  }
  if (!messagesIt->is_array())
  {
    mLogger->warn("Key 'messages' is not an array");
    return false;
  }

  for (const auto &msg : *messagesIt)
  {
    Node node;

    const auto subscriptionIt = msg.find("subscription");
    if (false // NOLINT
      || subscriptionIt == std::end(msg)
      || !subscriptionIt->is_number_unsigned()
    ) {
      mLogger->warn("Message is missing subscription id");
      return false;
    }
    node.subscriptionId = *subscriptionIt;

    const auto topicIt = msg.find("topic");
    if (true // NOLINT
      && topicIt != std::end(msg)
      && topicIt->is_string()
    ) {
      node.message.topic = *topicIt;
    }

    const auto qosIt = msg.find("qos");
    if (true // NOLINT
      && qosIt != std::end(msg)
      && qosIt->is_number_unsigned()
    ) {
      node.message.qos = *qosIt;
    }

    const auto payloadIt = msg.find("payload");
    if (true // NOLINT
      && payloadIt != std::end(msg)
      && payloadIt->is_string()
    ) {
      node.message.payload = *payloadIt;
    }

    const auto timestampIt = msg.find("timestamp");
    if (true // NOLINT
      && timestampIt != std::end(msg)
      && timestampIt->is_string()
    ) {
      node.message.timestamp = Common::Helpers::stringToTime(*timestampIt);
    }

    const auto retainedIt = msg.find("retained");
    if (true // NOLINT
      && retainedIt != std::end(msg)
      && retainedIt->is_string()
    ) {
      node.message.retained = *retainedIt;
    }

    mMessages.push_back(node);
    mRemap.push_back(mMessages.size() - 1);
    RowAppended();
  }

  mLogger->info("Loaded {} messages", mMessages.size());

  return true;
}

nlohmann::json History::toJson() const
{
  nlohmann::json result;

  for (const auto &m : mMessages)
  {
    const auto timestamp = Common::Helpers::timeToString(m.message.timestamp);
    result.push_back({
      {"subscription", m.subscriptionId},
      {"topic", m.message.topic},
      {"qos", m.message.qos},
      {"payload", m.message.payload},
      {"retained", m.message.retained},
      {"timestamp", timestamp},
    });
  }

  return result;
}

void History::onMessage(
  MQTT::Subscription::Id_t subscriptionId,
  const MQTT::Message &message
) {
  Node node {
    message,
    subscriptionId
  };
  mMessages.push_back(std::move(node));
  const bool isMuted = mSubscriptions->getMuted(subscriptionId);
  const bool isFiltered = mFilter.empty()
    || message.topic.find(mFilter) != std::string::npos;

  if (!isMuted && isFiltered)
  {
    mRemap.push_back(mMessages.size() - 1);
    RowAppended();

    const auto item = GetItem((unsigned)mRemap.size() - 1);
    for (const auto &o : mObservers)
    {
      o.second->onMessage(item);
    }
  }
}

void History::onMuted(MQTT::Subscription::Id_t /* subscriptionId */)
{
  remap();
}

void History::onUnmuted(MQTT::Subscription::Id_t /* subscriptionId */)
{
  remap();
}

void History::onSolo(MQTT::Subscription::Id_t /* subscriptionId */)
{
  remap();
}

void History::onColorSet(MQTT::Subscription::Id_t subscriptionId, wxColor /* color */)
{
  refresh(subscriptionId);
}

void History::onUnsubscribed(MQTT::Subscription::Id_t subscriptionId)
{
  for (auto it = std::begin(mMessages); it != std::end(mMessages); )
  {
    if (it->subscriptionId == subscriptionId)
    {
      it = mMessages.erase(it);
    }
    else
    {
      ++it;
    }
  }

  remap();
}

void History::onCleared(MQTT::Subscription::Id_t subscriptionId)
{
  for (auto it = std::begin(mMessages); it != std::end(mMessages); )
  {
    if (it->subscriptionId == subscriptionId)
    {
      it = mMessages.erase(it);
    }
    else
    {
      ++it;
    }
  }

  remap();
}

void History::remap()
{
  const size_t before = mRemap.size();
  mRemap.clear();
  mRemap.reserve(mMessages.size());

  for (size_t i = 0; i < mMessages.size(); ++i)
  {
    const bool isMuted = mSubscriptions->getMuted(mMessages[i].subscriptionId);
    const bool isFiltered = mFilter.empty()
      || mMessages[i].message.topic.find(mFilter) != std::string::npos;

    if (!isMuted && isFiltered)
    {
      mRemap.push_back(i);
    }
  }

  mRemap.shrink_to_fit();
  const size_t after = mRemap.size();

  const auto common = std::min(before, after);
  for (size_t i = 0; i != common; ++i)
  {
    RowChanged((unsigned)i);
  }

  if (after > before)
  {
    size_t diff = after - common;
    for (size_t i = 0; i < diff; ++i)
    {
      RowAppended();
    }
  }
  else if (after < before)
  {
    wxArrayInt rows;
    size_t diff = before - common;
    for (size_t i = 0; i < diff; ++i)
    {
      rows.Add((int)(after + i));
    }
    RowsDeleted(rows);
  }
}

void History::refresh(MQTT::Subscription::Id_t subscriptionId)
{
  for (size_t i = 0; i < mRemap.size(); ++i)
  {
    if (mMessages[mRemap[i]].subscriptionId == subscriptionId)
    {
      RowChanged((unsigned)i);
    }
  }
}

std::string History::getPayload(const wxDataViewItem &item) const
{
  return mMessages.at(mRemap.at(GetRow(item))).message.payload;
}

std::string History::getTopic(const wxDataViewItem &item) const
{
  return mMessages.at(mRemap.at(GetRow(item))).message.topic;
}

MQTT::QoS History::getQos(const wxDataViewItem &item) const
{
  return (MQTT::QoS)mMessages.at(mRemap.at(GetRow(item))).message.qos;
}

bool History::getRetained(const wxDataViewItem &item) const
{
  return mMessages.at(mRemap.at(GetRow(item))).message.retained;
}

const MQTT::Message &History::getMessage(const wxDataViewItem &item) const
{
  return mMessages.at(mRemap.at(GetRow(item))).message;
}

void History::setFilter(const std::string &filter)
{
  mFilter = filter;
  remap();
}

std::string History::getFilter() const
{
  return mFilter;
}

unsigned History::GetColumnCount() const
{
  return (unsigned)Column::Max;
}

unsigned History::GetCount() const
{
  return (unsigned)mRemap.size();
}

wxString History::GetColumnType(unsigned int col) const
{
  switch ((Column)col)
  {
    case Column::Icon:
    {
      return "wxColour";
    } break;

    case Column::Qos:
    {
      return wxDataViewBitmapRenderer::GetDefaultType();
    } break;

    case Column::Topic:
    {
      return wxDataViewIconTextRenderer::GetDefaultType();
    } break;

    default:
    {
      return "string";
    }
  }
}

void History::GetValueByRow(
  wxVariant &variant,
  unsigned int row,
  unsigned int col
) const {

  const auto &node = mMessages.at(mRemap.at(row));

  constexpr size_t MessageIconWidth = 10;
  constexpr size_t MessageIconHeight = 20;

  switch ((Column)col) {
    case Column::Icon: {

      auto color = mSubscriptions->getColor(node.subscriptionId);

      wxBitmap b(MessageIconWidth, MessageIconHeight);
      wxMemoryDC mem;
      mem.SelectObject(b);
      mem.SetBackground(wxBrush(color));
      mem.Clear();
      mem.SelectObject(wxNullBitmap);

      variant << b;
    } break;
    case Column::Topic: {
      wxDataViewIconText result;
      const auto &topic = node.message.topic;
      const auto wxs = wxString::FromUTF8(topic.data(), topic.length());
      result.SetText(wxs);
      if (node.message.retained)
      {
        wxIcon icon;
        icon.CopyFromBitmap(*bin2cPinned18x18());
        result.SetIcon(icon);
      }
      variant << result;
    } break;
    case Column::Qos: {
      const wxBitmap *result = nullptr;
      switch ((MQTT::QoS)node.message.qos) {
        case MQTT::QoS::AtLeastOnce: { result = bin2cQos0(); } break;
        case MQTT::QoS::AtMostOnce:  { result = bin2cQos1(); } break;
        case MQTT::QoS::ExactlyOnce: { result = bin2cQos2(); } break;
      }
      variant << *result;
    } break;
    default: {}
  }
}

bool History::GetAttrByRow(
  unsigned int /* row */,
  unsigned int /* col */,
  wxDataViewItemAttr &/* attr */
) const {
  return false;
}

bool History::SetValueByRow(
  const wxVariant &/* variant */,
  unsigned int /* row */,
  unsigned int /* col */
) {
  return false;
}
