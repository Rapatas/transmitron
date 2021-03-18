#include <wx/dcmemory.h>
#include <wx/log.h>
#include "History.hpp"
#include "Transmitron/Resources/pin/pinned-18x18.hpp"
#include "Transmitron/Resources/pin/not-pinned-18x18.hpp"
#include "Transmitron/Resources/qos/qos-0.hpp"
#include "Transmitron/Resources/qos/qos-1.hpp"
#include "Transmitron/Resources/qos/qos-2.hpp"

#define wxLOG_COMPONENT "Models/History"

using namespace Transmitron::Models;
using namespace Transmitron;

History::History(wxObjectDataPtr<Subscriptions> subscriptions) :
  mSubscriptions(std::move(subscriptions))
{
  mSubscriptions->attachObserver(this);
}

size_t History::attachObserver(Observer *observer)
{
  size_t id = 0;
  do {
    id = rand();
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

void History::onMessage(
  MQTT::Subscription::Id_t subscriptionId,
  mqtt::const_message_ptr message
) {
  Message m{subscriptionId, message};
  mMessages.push_back(m);
  bool muted = mSubscriptions->getMuted(subscriptionId);

  if (!mSubscriptions->getMuted(subscriptionId))
  {
    mRemap.push_back(mMessages.size() - 1);
    RowAppended();

    auto item = GetItem(mRemap.size() - 1);
    for (const auto &o : mObservers)
    {
      o.second->onMessage(item);
    }
  }
}

void History::onMuted(MQTT::Subscription::Id_t subscriptionId)
{
  remap();
}

void History::onUnmuted(MQTT::Subscription::Id_t subscriptionId)
{
  remap();
}

void History::onSolo(MQTT::Subscription::Id_t subscriptionId)
{
  remap();
}

void History::onColorSet(MQTT::Subscription::Id_t subscriptionId, wxColor color)
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
    if (!mSubscriptions->getMuted(mMessages[i].subscriptionId))
    {
      mRemap.push_back(i);
    }
  }

  mRemap.shrink_to_fit();
  const size_t after = mRemap.size();

  const auto common = std::min(before, after);
  for (size_t i = 0; i < common; ++i)
  {
    RowChanged(i);
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
      rows.Add(after + i);
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
      RowChanged(i);
    }
  }
}

std::string History::getPayload(const wxDataViewItem &item) const
{
  return mMessages.at(mRemap.at(GetRow(item))).message->get_payload();
}

std::string History::getTopic(const wxDataViewItem &item) const
{
  return mMessages.at(mRemap.at(GetRow(item))).message->get_topic();
}

MQTT::QoS History::getQos(const wxDataViewItem &item) const
{
  return (MQTT::QoS)mMessages.at(mRemap.at(GetRow(item))).message->get_qos();
}

bool History::getRetained(const wxDataViewItem &item) const
{
  return mMessages.at(mRemap.at(GetRow(item))).message->is_retained();
}

std::shared_ptr<MQTT::Message> History::getMessage(const wxDataViewItem &item) const
{
  auto message = mMessages.at(mRemap.at(GetRow(item))).message;
  MQTT::Message m {
    message->get_topic(),
    message->get_payload(),
    static_cast<MQTT::QoS>(message->get_qos()),
    message->is_retained()
  };
  return std::make_shared<MQTT::Message>(std::move(m));
}

unsigned History::GetColumnCount() const
{
  return (unsigned)Column::Max;
}

unsigned History::GetCount() const
{
  return mRemap.size();
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

  switch ((Column)col) {
    case Column::Icon: {

      auto color = mSubscriptions->getColor(m.subscriptionId);

      wxBitmap b(10, 20);
      wxMemoryDC mem;
      mem.SelectObject(b);
      mem.SetBackground(wxBrush(color));
      mem.Clear();
      mem.SelectObject(wxNullBitmap);

      variant << b;
    } break;
    case Column::Topic: {
      wxDataViewIconText result;
      result.SetText(m.message->get_topic());
      if (m.message->is_retained())
      {
        wxIcon icon;
        icon.CopyFromBitmap(*bin2c_pinned_18x18_png);
        result.SetIcon(icon);
      }
      variant << result;
    } break;
    case Column::Qos: {
      wxBitmap *result;
      switch ((MQTT::QoS)m.message->get_qos()) {
        case MQTT::QoS::AtLeastOnce: { result = bin2c_qos_0_png; } break;
        case MQTT::QoS::AtMostOnce:  { result = bin2c_qos_1_png; } break;
        case MQTT::QoS::ExactlyOnce: { result = bin2c_qos_2_png; } break;
      }
      variant << *result;
    } break;
    default: {}
  }
}

bool History::GetAttrByRow(
  unsigned int row,
  unsigned int col,
  wxDataViewItemAttr &attr
) const {
  return false;
}

bool History::SetValueByRow(
  const wxVariant &variant,
  unsigned int row,
  unsigned int col
) {
  return false;
}
