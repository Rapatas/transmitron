#include <wx/dcmemory.h>
#include <wx/log.h>
#include "History.hpp"
#include "Transmitron/Resources/pin/pinned-18x18.hpp"
#include "Transmitron/Resources/pin/not-pinned-18x18.hpp"
#include "Transmitron/Resources/qos/qos-0.hpp"
#include "Transmitron/Resources/qos/qos-1.hpp"
#include "Transmitron/Resources/qos/qos-2.hpp"

#define wxLOG_COMPONENT "models/history"

using namespace Transmitron::Models;
using namespace Transmitron;

History::History() {}
History::~History() {}

size_t History::attachObserver(Observer *observer)
{
  size_t id = 0;
  do {
    id = rand();
  } while (mObservers.find(id) != std::end(mObservers));

  return mObservers.insert(std::make_pair(id, observer)).first->first;
}

void History::insert(
  Types::SubscriptionData *sub,
  mqtt::const_message_ptr msg
) {
  Message m{sub, msg};
  mMessages.push_back(m);

  if (!sub->getMuted())
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

void History::remove(Types::SubscriptionData *sub)
{
  for (auto it = std::begin(mMessages); it != std::end(mMessages); )
  {
    if (it->sub == sub)
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
  size_t before = mRemap.size();
  mRemap.clear();
  mRemap.reserve(mMessages.size());

  for (size_t i = 0; i < mMessages.size(); ++i)
  {
    if (!mMessages[i].sub->getMuted())
    {
      mRemap.push_back(i);
    }
  }

  mRemap.shrink_to_fit();

  for (size_t i = 0; i < mRemap.size(); ++i)
  {
    RowChanged(i);
  }

  if (before < mRemap.size())
  {
    size_t diff = mRemap.size() - before;

    for (size_t i = 0; i < diff; ++i)
    {
      RowAppended();
    }
  }
  else if (before > mRemap.size())
  {
    wxArrayInt rows;
    size_t diff = before - mRemap.size();

    for (size_t i = 0; i < diff; ++i)
    {
      rows.Add(mRemap.size() + i);
    }

    RowsDeleted(rows);
  }
}

void History::refresh(Types::SubscriptionData *sub)
{
  for (size_t i = 0; i < mRemap.size(); ++i)
  {
    if (mMessages[mRemap[i]].sub == sub)
    {
      RowChanged(i);
    }
  }
}

std::string History::getPayload(const wxDataViewItem &item) const
{
  return mMessages.at(mRemap.at(GetRow(item))).msg->get_payload();
}

std::string History::getTopic(const wxDataViewItem &item) const
{
  return mMessages.at(mRemap.at(GetRow(item))).msg->get_topic();
}

MQTT::QoS History::getQos(const wxDataViewItem &item) const
{
  return (MQTT::QoS)mMessages.at(mRemap.at(GetRow(item))).msg->get_qos();
}

bool History::getRetained(const wxDataViewItem &item) const
{
  return mMessages.at(mRemap.at(GetRow(item))).msg->is_retained();
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

      wxBitmap b(10, 20);
      wxMemoryDC mem;
      mem.SelectObject(b);
      mem.SetBackground(wxBrush(m.sub->getColor()));
      mem.Clear();
      mem.SelectObject(wxNullBitmap);

      variant << b;
    } break;
    case Column::Topic: {
      wxDataViewIconText result;
      result.SetText(m.msg->get_topic());
      if (m.msg->is_retained())
      {
        wxIcon icon;
        icon.CopyFromBitmap(*bin2c_pinned_18x18_png);
        result.SetIcon(icon);
      }
      variant << result;
    } break;
    case Column::Qos: {
      wxBitmap *result;
      switch ((MQTT::QoS)m.msg->get_qos()) {
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
