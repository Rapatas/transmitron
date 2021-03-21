#include <algorithm>
#include <iterator>
#include <wx/dcmemory.h>
#include <wx/log.h>
#include "Subscriptions.hpp"
#include "Transmitron/Resources/qos/qos-0.hpp"
#include "Transmitron/Resources/qos/qos-1.hpp"
#include "Transmitron/Resources/qos/qos-2.hpp"

#define wxLOG_COMPONENT "models/subscriptions"

using namespace Transmitron::Models;

Subscriptions::Subscriptions(std::shared_ptr<MQTT::Client> client) :
  mClient(client)
{}

size_t Subscriptions::attachObserver(Observer *observer)
{
  size_t id = 0;
  do {
    id = (size_t)std::abs(rand());
  } while (mObservers.find(id) != std::end(mObservers));

  return mObservers.insert(std::make_pair(id, observer)).first->first;
}

bool Subscriptions::detachObserver(size_t id)
{
  auto it = mObservers.find(id);
  if (it == std::end(mObservers))
  {
    return false;
  }

  mObservers.erase(it);
  return true;
}

std::string Subscriptions::getFilter(wxDataViewItem item) const
{
  auto &sub = mSubscriptions.at(mRemap.at(GetRow(item)));
  return sub->getFilter();
}

std::string Subscriptions::getFilter(MQTT::Subscription::Id_t subscriptionId) const
{
  auto &sub = mSubscriptions.at(subscriptionId);
  return sub->getFilter();
}

bool Subscriptions::getMuted(wxDataViewItem item) const
{
  auto &sub = mSubscriptions.at(mRemap.at(GetRow(item)));
  return sub->getMuted();
}

bool Subscriptions::getMuted(MQTT::Subscription::Id_t subscriptionId) const
{
  auto &sub = mSubscriptions.at(subscriptionId);
  return sub->getMuted();
}

wxColor Subscriptions::getColor(MQTT::Subscription::Id_t subscriptionId) const
{
  auto &sub = mSubscriptions.at(subscriptionId);
  return sub->getColor();
}

void Subscriptions::setColor(wxDataViewItem item, const wxColor &color)
{
  auto &sub = mSubscriptions.at(mRemap.at(GetRow(item)));
  sub->setColor(color);
  for (const auto &o : mObservers)
  {
    o.second->onColorSet(sub->getId(), color);
  }
  ValueChanged(item, (unsigned)Column::Icon);
}

void Subscriptions::unmute(wxDataViewItem item)
{
  auto &sub = mSubscriptions.at(mRemap.at(GetRow(item)));
  sub->setMuted(false);
  for (const auto &o : mObservers)
  {
    o.second->onUnmuted(sub->getId());
  }
  ItemChanged(item);
}

void Subscriptions::clear(wxDataViewItem item)
{
  auto &sub = mSubscriptions.at(mRemap.at(GetRow(item)));
  for (const auto &o : mObservers)
  {
    o.second->onCleared(sub->getId());
  }
  ItemChanged(item);
}

void Subscriptions::mute(wxDataViewItem item)
{
  auto &sub = mSubscriptions.at(mRemap.at(GetRow(item)));
  sub->setMuted(true);
  for (const auto &o : mObservers)
  {
    o.second->onMuted(sub->getId());
  }
  ItemChanged(item);
}

void Subscriptions::solo(wxDataViewItem item)
{
  for (auto &subscription : mSubscriptions)
  {
    subscription.second->setMuted(true);
  }

  wxDataViewItemArray array;
  for (size_t i = 0; i != mRemap.size(); ++i)
  {
    array.push_back(GetItem((unsigned)i));
  }
  ItemsChanged(array);

  auto &sub = mSubscriptions.at(mRemap.at(GetRow(item)));
  sub->setMuted(false);
  for (const auto &o : mObservers)
  {
    o.second->onSolo(sub->getId());
  }
  ItemChanged(item);
}

void Subscriptions::subscribe(const std::string &topic, MQTT::QoS /* qos */)
{
  auto it = std::find_if(
    std::begin(mSubscriptions),
    std::end(mSubscriptions),
    [&topic](auto &sub)
    {
      return sub.second->getFilter() == topic;
    }
  );

  if (it != std::end(mSubscriptions))
  {
    wxLogMessage("Already subscribed!");
    return;
  }

  auto mqttSubscription = mClient->subscribe(topic);
  auto sub = std::make_unique<Types::Subscription>(mqttSubscription);
  const auto id = sub->getId();
  sub->Bind(Events::SUBSCRIBED, &Subscriptions::onSubscribed, this);
  sub->Bind(Events::UNSUBSCRIBED, &Subscriptions::onUnsubscribed, this);
  sub->Bind(Events::RECEIVED, &Subscriptions::onMessage, this);
  mSubscriptions.insert({id, std::move(sub)});
  mRemap.push_back(id);
  RowAppended();
}

void Subscriptions::unsubscribe(wxDataViewItem item)
{
  auto &sub = mSubscriptions.at(mRemap.at(GetRow(item)));
  sub->unsubscribe();
}

unsigned Subscriptions::GetColumnCount() const
{
  return (unsigned)Column::Max;
}

unsigned Subscriptions::GetCount() const
{
  return (unsigned)mSubscriptions.size();
}

wxString Subscriptions::GetColumnType(unsigned int col) const
{
  switch ((Column)col)
  {
    case Column::Icon:  { return "wxColor";                                  } break;
    case Column::Qos:   { return wxDataViewBitmapRenderer::GetDefaultType(); } break;
    case Column::Topic: { return wxDataViewTextRenderer::GetDefaultType();   } break;
    default: { return "string"; }
  }
}

void Subscriptions::GetValueByRow(
  wxVariant &variant,
  unsigned int row,
  unsigned int col
) const {
  auto &sub = mSubscriptions.at(mRemap.at(row));

  switch ((Column)col) {
    case Column::Icon: {

      wxBitmap b(10, 20);
      wxMemoryDC mem;
      mem.SelectObject(b);
      mem.SetBackground(wxBrush(sub->getColor()));
      mem.Clear();
      mem.SelectObject(wxNullBitmap);

      variant << b;
    } break;
    case Column::Topic: {
      variant = sub->getFilter();
    } break;
    case Column::Qos: {
      wxBitmap *result;
      switch (sub->getQos()) {
        case MQTT::QoS::AtLeastOnce: { result = bin2c_qos_0_png; } break;
        case MQTT::QoS::AtMostOnce:  { result = bin2c_qos_1_png; } break;
        case MQTT::QoS::ExactlyOnce: { result = bin2c_qos_2_png; } break;
      }
      variant << *result;
    } break;
    default: {}
  }
}

bool Subscriptions::GetAttrByRow(
  unsigned int /* row */,
  unsigned int /* col */,
  wxDataViewItemAttr &/* attr */
) const {
  return false;
}

bool Subscriptions::SetValueByRow(
  const wxVariant &/* variant */,
  unsigned int /* row */,
  unsigned int /* col */
) {
  return false;
}

void Subscriptions::onSubscribed(Events::Subscription &/* e */) {}

void Subscriptions::onUnsubscribed(Events::Subscription &e)
{
  const auto id = e.getId();
  auto it = std::find_if(
    std::begin(mRemap),
    std::end(mRemap),
    [id](auto subId)
    {
      return subId == id;
    }
  );
  if (it == std::end(mRemap))
  {
    wxLogError("Could not find subscription");
    return;
  }
  const auto index = (size_t)std::distance(std::begin(mRemap), it);
  const auto item = GetItem((unsigned)index);
  for (const auto &o : mObservers)
  {
    o.second->onUnsubscribed(id);
  }
  mSubscriptions.erase(id);
  mRemap.erase(it);
  ItemDeleted(wxDataViewItem(0), item);
}

void Subscriptions::onMessage(Events::Subscription &e)
{
  for (const auto &o : mObservers)
  {
    o.second->onMessage(e.getId(), e.getMessage());
  }
}
