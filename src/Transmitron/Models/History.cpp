#include <wx/dcmemory.h>
#include "History.hpp"
#include "Transmitron/Resources/pin/pinned-18x18.hpp"
#include "Transmitron/Resources/pin/not-pinned-18x18.hpp"
#include "Transmitron/Resources/qos/qos-0.hpp"
#include "Transmitron/Resources/qos/qos-1.hpp"
#include "Transmitron/Resources/qos/qos-2.hpp"

using namespace Transmitron::Models;
using namespace Transmitron;

History::History(const wxObjectDataPtr<Subscriptions> &subscriptions) :
  mSubscriptions(subscriptions)
{
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

void History::onMessage(const MQTT::Message &message)
{
  mMessages.push_back(message);
  const bool isMuted = mSubscriptions->getMuted(message.subscriptionId);
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
      || mMessages[i].topic.find(mFilter) != std::string::npos;

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
  return mMessages.at(mRemap.at(GetRow(item))).payload;
}

std::string History::getTopic(const wxDataViewItem &item) const
{
  return mMessages.at(mRemap.at(GetRow(item))).topic;
}

MQTT::QoS History::getQos(const wxDataViewItem &item) const
{
  return (MQTT::QoS)mMessages.at(mRemap.at(GetRow(item))).qos;
}

bool History::getRetained(const wxDataViewItem &item) const
{
  return mMessages.at(mRemap.at(GetRow(item))).retained;
}

const MQTT::Message &History::getMessage(const wxDataViewItem &item) const
{
  return mMessages.at(mRemap.at(GetRow(item)));
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

  const auto &m = mMessages.at(mRemap.at(row));

  constexpr size_t MessageIconWidth = 10;
  constexpr size_t MessageIconHeight = 20;

  switch ((Column)col) {
    case Column::Icon: {

      auto color = mSubscriptions->getColor(m.subscriptionId);

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
      const auto wxs = wxString::FromUTF8(m.topic.data(), m.topic.length());
      result.SetText(wxs);
      if (m.retained)
      {
        wxIcon icon;
        icon.CopyFromBitmap(*bin2cPinned18x18());
        result.SetIcon(icon);
      }
      variant << result;
    } break;
    case Column::Qos: {
      const wxBitmap *result = nullptr;
      switch ((MQTT::QoS)m.qos) {
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
