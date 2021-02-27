#include <algorithm>
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

Subscriptions::~Subscriptions()
{
  for (const auto &s : mSubscriptions)
  {
    delete s;
  }
}

size_t Subscriptions::attachObserver(Observer *observer)
{
  size_t id = 0;
  do {
    id = rand();
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

std::string Subscriptions::getFilter(const wxDataViewItem &item) const
{
  auto sub = mSubscriptions.at(GetRow(item));
  return sub->getFilter();
}

bool Subscriptions::getMuted(const wxDataViewItem &item) const
{
  auto sub = mSubscriptions.at(GetRow(item));
  return sub->getMuted();
}

wxColor Subscriptions::getColor(const wxDataViewItem &item) const
{
  auto sub = mSubscriptions.at(GetRow(item));
  return sub->getColor();
}

void Subscriptions::setColor(const wxDataViewItem &item, const wxColor &color)
{
  auto sub = mSubscriptions.at(GetRow(item));
  sub->setColor(color);
  for (const auto &o : mObservers)
  {
    o.second->onColorSet(item, color);
  }
  ValueChanged(item, (unsigned)Column::Icon);
}

void Subscriptions::unmute(const wxDataViewItem &item)
{
  auto sub = mSubscriptions.at(GetRow(item));
  sub->setMuted(false);
  for (const auto &o : mObservers)
  {
    o.second->onUnmuted(item);
  }
  ItemChanged(item);
}

void Subscriptions::mute(const wxDataViewItem &item)
{
  auto sub = mSubscriptions.at(GetRow(item));
  sub->setMuted(true);
  for (const auto &o : mObservers)
  {
    o.second->onMuted(item);
  }
  ItemChanged(item);
}

void Subscriptions::solo(const wxDataViewItem &item)
{
  for (size_t i = 0; i < mSubscriptions.size(); ++i)
  {
    mSubscriptions[i]->setMuted(true);
    ItemChanged(GetItem(i));
  }

  auto sub = mSubscriptions.at(GetRow(item));
  sub->setMuted(false);
  for (const auto &o : mObservers)
  {
    o.second->onSolo(item);
  }
  ItemChanged(item);
}

void Subscriptions::subscribe(const std::string &topic, MQTT::QoS qos)
{
  auto it = std::find_if(
    std::begin(mSubscriptions),
    std::end(mSubscriptions),
    [&topic](Types::SubscriptionData *sub)
    {
      return sub->getFilter() == topic;
    }
  );

  if (it != std::end(mSubscriptions))
  {
    wxLogMessage("Already subscribed!");
    return;
  }

  auto sub = mClient->subscribe(topic);
  auto sd = new Types::SubscriptionData(sub);
  mSubscriptions.push_back(sd);
  sd->Bind(Events::SUBSCRIBED, &Subscriptions::onSubscribed, this);
  sd->Bind(Events::UNSUBSCRIBED, &Subscriptions::onUnsubscribed, this);
  sd->Bind(Events::RECEIVED, &Subscriptions::onMessage, this);

  auto item = GetItem(mSubscriptions.size() - 1);
  ItemAdded(wxDataViewItem(0), item);
}

void Subscriptions::unsubscribe(const wxDataViewItem &item)
{
  auto sub = mSubscriptions.at(GetRow(item));
  sub->unsubscribe();
}

unsigned Subscriptions::GetColumnCount() const
{
  return (unsigned)Column::Max;
}

unsigned Subscriptions::GetCount() const
{
  return mSubscriptions.size();
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
  auto s = mSubscriptions.at(row);

  switch ((Column)col) {
    case Column::Icon: {

      wxBitmap b(10, 20);
      wxMemoryDC mem;
      mem.SelectObject(b);
      mem.SetBackground(wxBrush(s->getColor()));
      mem.Clear();
      mem.SelectObject(wxNullBitmap);

      variant << b;
    } break;
    case Column::Topic: {
      variant = s->getFilter();
    } break;
    case Column::Qos: {
      wxBitmap *result;
      switch (s->getQos()) {
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
  unsigned int row,
  unsigned int col,
  wxDataViewItemAttr &attr
) const {
  return false;
}

bool Subscriptions::SetValueByRow(
  const wxVariant &variant,
  unsigned int row,
  unsigned int col
) {
  return false;
}

void Subscriptions::onSubscribed(Events::Subscription &e)
{

}

void Subscriptions::onUnsubscribed(Events::Subscription &e)
{
  auto it = std::find(
    std::begin(mSubscriptions),
    std::end(mSubscriptions),
    e.getSubscription()
  );
  if (it == std::end(mSubscriptions))
  {
    wxLogError("Could not find subscription");
    return;
  }

  auto index = it - std::begin(mSubscriptions);
  auto item = GetItem(index);
  for (const auto &o : mObservers)
  {
    o.second->onUnsubscribed(item);
  }
  ItemDeleted(wxDataViewItem(0), item);
  mSubscriptions.erase(it);
}

void Subscriptions::onMessage(Events::Subscription &e)
{
  auto it = std::find(
    std::begin(mSubscriptions),
    std::end(mSubscriptions),
    e.getSubscription()
  );
  if (it == std::end(mSubscriptions))
  {
    wxLogError("Could not find subscription");
    return;
  }

  auto index = it - std::begin(mSubscriptions);
  auto item = GetItem(index);
  for (const auto &o : mObservers)
  {
    o.second->onMessage(item, e.getMessage());
  }
}

