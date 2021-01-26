#include <algorithm>
#include <wx/dcmemory.h>
#include <wx/log.h>
#include "Subscriptions.hpp"
#include "images/qos/qos-0.hpp"
#include "images/qos/qos-1.hpp"
#include "images/qos/qos-2.hpp"

#define wxLOG_COMPONENT "models/subscriptions"

Subscriptions::Subscriptions(
  std::shared_ptr<MQTT::Client> client,
  wxObjectDataPtr<History> history
) :
  mClient(client),
  mHistory(history)
{}

Subscriptions::~Subscriptions()
{
  for (const auto &s : mSubscriptions)
  {
    delete s;
  }
}

std::string Subscriptions::getFilter(
  const wxDataViewItem &item
) const {
  auto sub = static_cast<SubscriptionData*>(item.GetID());
  return sub->getFilter();
}

void Subscriptions::setColor(const wxDataViewItem &item, const wxColor &color)
{
  auto sub = static_cast<SubscriptionData*>(item.GetID());
  sub->setColor(color);
  mHistory->refresh(sub);
  ValueChanged(item, (unsigned)Column::Icon);
}

void Subscriptions::unmute(const wxDataViewItem &item)
{
  auto sub = static_cast<SubscriptionData*>(item.GetID());
  sub->setMuted(false);
  mHistory->remap();
  ItemChanged(item);
}

void Subscriptions::mute(const wxDataViewItem &item)
{
  auto sub = static_cast<SubscriptionData*>(item.GetID());
  sub->setMuted(true);
  mHistory->remap();
  ItemChanged(item);
}

void Subscriptions::solo(const wxDataViewItem &item)
{
  for (const auto &sub : mSubscriptions)
  {
    sub->setMuted(true);
    ItemChanged(wxDataViewItem(static_cast<void*>(sub)));
  }

  auto sub = static_cast<SubscriptionData*>(item.GetID());
  sub->setMuted(false);
  mHistory->remap();
  ItemChanged(item);
}

void Subscriptions::subscribe(const std::string &topic, MQTT::QoS qos)
{
  auto it = std::find_if(
    std::begin(mSubscriptions),
    std::end(mSubscriptions),
    [&topic](SubscriptionData *sub)
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
  auto sd = new SubscriptionData(sub);
  mSubscriptions.push_back(sd);
  sd->Bind(EVT_SUB_SUBSCRIBED, &Subscriptions::onSubscribed, this);
  sd->Bind(EVT_SUB_UNSUBSCRIBED, &Subscriptions::onUnsubscribed, this);
  sd->attachObserver(this);

  wxDataViewItem child(static_cast<void*>(sd));
  ItemAdded(wxDataViewItem(0), child);
}

void Subscriptions::unsubscribe(const wxDataViewItem &item)
{
  auto sub = static_cast<SubscriptionData*>(item.GetID());
  sub->unsubscribe();
}

unsigned Subscriptions::GetColumnCount() const
{
  return (unsigned)Column::Max;
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

void Subscriptions::GetValue(
  wxVariant &variant,
  const wxDataViewItem &item,
  unsigned int col
) const {
  auto s = static_cast<SubscriptionData*>(item.GetID());

  switch ((Column)col) {
    case Column::Icon: {

      wxBitmap b(20, 20);
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

bool Subscriptions::SetValue(
  const wxVariant &variant,
  const wxDataViewItem &item,
  unsigned int col
) {
  return false;
}

bool Subscriptions::IsEnabled(
  const wxDataViewItem &item,
  unsigned int col
) const {
  return true;
}

wxDataViewItem Subscriptions::GetParent(
  const wxDataViewItem &item
) const {
  return wxDataViewItem(nullptr);
}

bool Subscriptions::IsContainer(
  const wxDataViewItem &item
) const {
  return false;
}

unsigned int Subscriptions::GetChildren(
  const wxDataViewItem &parent,
  wxDataViewItemArray &array
) const {
  for (const auto &s : mSubscriptions)
  {
    array.Add(wxDataViewItem(static_cast<void*>(s)));
  }
  return mSubscriptions.size();
}

void Subscriptions::onSubscribed(SubscriptionEvent &e)
{

}

void Subscriptions::onUnsubscribed(SubscriptionEvent &e)
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

  mHistory->remove(e.getSubscription());
  wxDataViewItem item(static_cast<void*>(e.getSubscription()));
  ItemDeleted(wxDataViewItem(0), item);
  mSubscriptions.erase(it);
}

void Subscriptions::onMessage(
  SubscriptionData *subscriptionData,
  mqtt::const_message_ptr msg
) {
  mHistory->insert(subscriptionData, msg);
}

