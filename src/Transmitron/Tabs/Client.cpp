#include "Client.hpp"

#include <sstream>
#include <nlohmann/json.hpp>

#include "Transmitron/Resources/plus/plus-18x18.hpp"

#define wxLOG_COMPONENT "Client"

using namespace Transmitron::Tabs;
using namespace Transmitron;

wxDEFINE_EVENT(Events::CONNECTED, Events::Connection);
wxDEFINE_EVENT(Events::DISCONNECTED, Events::Connection);

Client::Client(
  wxWindow* parent,
  std::shared_ptr<Types::Connection> connection
) :
  wxPanel(parent),
  mConnection(connection)
{
  mClient = std::make_shared<MQTT::Client>();
  Bind(Events::CONNECTED, &Client::onConnectedSync, this);
  Bind(Events::DISCONNECTED, &Client::onDisconnectedSync, this);
  mClient->attachObserver(this);
  Bind(wxEVT_CLOSE_WINDOW, &Client::onClose, this);
  Bind(wxEVT_COMMAND_MENU_SELECTED, &Client::onContextSelected, this);

  mPaneInfos.insert({Panes::History, {}});
  mPaneInfos.insert({Panes::Preview, {}});
  mPaneInfos.insert({Panes::Publish, {}});
  mPaneInfos.insert({Panes::Subscriptions, {}});
  mPaneInfos.insert({Panes::Snippets, {}});

  mPaneInfos.at(Panes::History).Caption("History");
  mPaneInfos.at(Panes::Preview).Caption("Preview");
  mPaneInfos.at(Panes::Publish).Caption("Publish");
  mPaneInfos.at(Panes::Subscriptions).Caption("Subscriptions");
  mPaneInfos.at(Panes::Snippets).Caption("Snippets");

  mPaneInfos.at(Panes::History).Movable(false);
  mPaneInfos.at(Panes::Preview).Movable(true);
  mPaneInfos.at(Panes::Publish).Movable(true);
  mPaneInfos.at(Panes::Subscriptions).Movable(true);
  mPaneInfos.at(Panes::Snippets).Movable(true);

  mPaneInfos.at(Panes::History).Floatable(false);
  mPaneInfos.at(Panes::Preview).Floatable(true);
  mPaneInfos.at(Panes::Publish).Floatable(true);
  mPaneInfos.at(Panes::Subscriptions).Floatable(true);
  mPaneInfos.at(Panes::Snippets).Floatable(true);

  mPaneInfos.at(Panes::History).Center();
  mPaneInfos.at(Panes::Subscriptions).Left();
  mPaneInfos.at(Panes::Snippets).Left();
  mPaneInfos.at(Panes::Preview).Bottom();
  mPaneInfos.at(Panes::Publish).Bottom();

  mPaneInfos.at(Panes::History).Layer(0);
  mPaneInfos.at(Panes::Subscriptions).Layer(1);
  mPaneInfos.at(Panes::Snippets).Layer(1);
  mPaneInfos.at(Panes::Preview).Layer(2);
  mPaneInfos.at(Panes::Publish).Layer(2);

  // mPaneInfos.at(Panes::History).Row(0);
  // mPaneInfos.at(Panes::Snippets).Row(1);
  // mPaneInfos.at(Panes::Preview).Row(2);
  // mPaneInfos.at(Panes::Publish).Row(2);

  mPaneInfos.at(Panes::History).MinSize(wxSize(100, 100));
  mPaneInfos.at(Panes::Subscriptions).MinSize(wxSize(300, 100));
  mPaneInfos.at(Panes::Snippets).MinSize(wxSize(300, 0));
  mPaneInfos.at(Panes::Preview).MinSize(wxSize(0, 200));
  mPaneInfos.at(Panes::Publish).MinSize(wxSize(0, 200));

  mPaneInfos.at(Panes::History).CloseButton(false);
  mPaneInfos.at(Panes::History).CloseButton(false);
  mPaneInfos.at(Panes::Subscriptions).CloseButton(false);
  mPaneInfos.at(Panes::Snippets).CloseButton(false);
  mPaneInfos.at(Panes::Preview).CloseButton(false);
  mPaneInfos.at(Panes::Publish).CloseButton(false);

  mPaneInfos.at(Panes::History).PaneBorder(false);
  mPaneInfos.at(Panes::History).PaneBorder(false);
  mPaneInfos.at(Panes::Subscriptions).PaneBorder(false);
  mPaneInfos.at(Panes::Snippets).PaneBorder(false);
  mPaneInfos.at(Panes::Preview).PaneBorder(false);
  mPaneInfos.at(Panes::Publish).PaneBorder(false);

  auto wrapper = new wxPanel(this, -1);

  setupPanelPublish(wrapper);
  setupPanelHistory(wrapper);
  setupPanelSubscriptions(wrapper);
  setupPanelPreview(wrapper);
  setupPanelSnippets(wrapper);
  setupPanelConnect(this);

  mMasterSizer = new wxBoxSizer(wxVERTICAL);
  mMasterSizer->Add(mConnectionBar, 0, wxEXPAND);
  mMasterSizer->Add(wrapper, 1, wxEXPAND);
  SetSizer(mMasterSizer);

  mAuiMan = new wxAuiManager();
  mAuiMan->SetManagedWindow(wrapper);
  for (const auto &pane : mPanes)
  {
    mAuiMan->AddPane(pane.second, mPaneInfos.at(pane.first));
  }
  mAuiMan->Update();

}

void Client::setupPanelHistory(wxWindow *parent)
{
  wxDataViewColumn* const icon = new wxDataViewColumn(
    L"icon",
    new wxDataViewBitmapRenderer(),
    (unsigned)Models::History::Column::Icon,
    wxCOL_WIDTH_AUTOSIZE,
    wxALIGN_LEFT
  );
  wxDataViewColumn* const topic = new wxDataViewColumn(
    L"topic",
    new wxDataViewTextRenderer(),
    (unsigned)Models::History::Column::Topic,
    wxCOL_WIDTH_AUTOSIZE,
    wxALIGN_LEFT
  );
  wxDataViewColumn* const qos = new wxDataViewColumn(
    L"qos",
    new wxDataViewBitmapRenderer(),
    (unsigned)Models::History::Column::Qos,
    wxCOL_WIDTH_AUTOSIZE
  );
  wxDataViewColumn* const retained = new wxDataViewColumn(
    L"retained",
    new wxDataViewBitmapRenderer(),
    (unsigned)Models::History::Column::Retained,
    25
  );

  wxFont font(wxFontInfo(9).FaceName("Consolas"));

  auto panel = new wxPanel(parent, -1, wxDefaultPosition);
  mPanes.insert({Panes::History, panel});

  mHistoryCtrl = new wxDataViewCtrl(
    panel,
    -1,
    wxDefaultPosition,
    wxDefaultSize,
    wxDV_NO_HEADER | wxDV_ROW_LINES
  );

  mHistoryModel = new Models::History;
  mHistoryModel->attachObserver(this);
  mHistoryCtrl->AssociateModel(mHistoryModel.get());

  mHistoryCtrl->SetFont(font);

  mHistoryCtrl->AppendColumn(icon);
  mHistoryCtrl->AppendColumn(qos);
  mHistoryCtrl->AppendColumn(retained);
  mHistoryCtrl->AppendColumn(topic);

  mAutoScroll = new wxCheckBox(panel, -1, "auto-scroll");
  mAutoScroll->SetValue(true);

  auto hsizer = new wxBoxSizer(wxOrientation::wxHORIZONTAL);
  hsizer->SetMinSize(0, OptionsHeight);
  hsizer->Add(mAutoScroll, 0, wxEXPAND);
  auto vsizer = new wxBoxSizer(wxOrientation::wxVERTICAL);
  vsizer->Add(mHistoryCtrl, 1, wxEXPAND);
  vsizer->Add(hsizer, 0, wxEXPAND);
  panel->SetSizer(vsizer);

  mHistoryCtrl->Bind(wxEVT_DATAVIEW_SELECTION_CHANGED, &Client::onHistorySelected, this);
  mHistoryCtrl->Bind(wxEVT_DATAVIEW_ITEM_CONTEXT_MENU, &Client::onHistoryContext, this);
}

void Client::setupPanelConnect(wxWindow *parent)
{
  mConnectionBar = new wxPanel(parent, -1);

  mConnect  = new wxButton(mConnectionBar, -1, "Connect");

  wxBoxSizer *hsizer = new wxBoxSizer(wxOrientation::wxHORIZONTAL);
  hsizer->SetMinSize(0, OptionsHeight);
  hsizer->Add(mConnect, 0, wxEXPAND);
  mConnectionBar->SetSizer(hsizer);

  mConnect->Bind(wxEVT_BUTTON, &Client::onConnectClicked, this);
}

void Client::setupPanelSubscriptions(wxWindow *parent)
{
  wxDataViewColumn* const icon = new wxDataViewColumn(
    "icon",
    new wxDataViewBitmapRenderer(),
    (unsigned)Models::Subscriptions::Column::Icon,
    wxCOL_WIDTH_AUTOSIZE
  );
  wxDataViewColumn* const topic = new wxDataViewColumn(
    "topic",
    new wxDataViewTextRenderer(),
    (unsigned)Models::Subscriptions::Column::Topic,
    wxCOL_WIDTH_AUTOSIZE,
    wxALIGN_LEFT
  );
  wxDataViewColumn* const qos = new wxDataViewColumn(
    "qos",
    new wxDataViewBitmapRenderer(),
    (unsigned)Models::Subscriptions::Column::Qos,
    wxCOL_WIDTH_AUTOSIZE
  );

  wxFont font(wxFontInfo(9).FaceName("Consolas"));

  auto panel = new wxPanel(parent, -1, wxDefaultPosition);
  mPanes.insert({Panes::Subscriptions, panel});

  mSubscriptionsCtrl = new wxDataViewListCtrl(panel, -1, wxDefaultPosition, wxDefaultSize, wxDV_NO_HEADER);
  mSubscriptionsCtrl->AppendColumn(icon);
  mSubscriptionsCtrl->AppendColumn(qos);
  mSubscriptionsCtrl->AppendColumn(topic);

  mSubscriptionsModel = new Models::Subscriptions(mClient, mHistoryModel);
  mSubscriptionsCtrl->AssociateModel(mSubscriptionsModel);

  mSubscriptionsCtrl->SetFont(font);

  mSubscribe = new wxBitmapButton(
    panel,
    -1,
    *bin2c_plus_18x18_png,
    wxDefaultPosition,
    wxSize(OptionsHeight, OptionsHeight)
  );

  mFilter = new Widgets::TopicCtrl(panel, -1);
  mFilter->SetFont(font);

  wxBoxSizer *vsizer = new wxBoxSizer(wxOrientation::wxVERTICAL);
  wxBoxSizer *hsizer = new wxBoxSizer(wxOrientation::wxHORIZONTAL);
  hsizer->SetMinSize(0, OptionsHeight);
  hsizer->Add(mFilter,  1, wxEXPAND);
  hsizer->Add(mSubscribe,  0, wxEXPAND);
  vsizer->Add(hsizer,  0, wxEXPAND);
  vsizer->Add(mSubscriptionsCtrl,  1, wxEXPAND);
  panel->SetSizer(vsizer);

  mSubscriptionsCtrl->Bind(wxEVT_DATAVIEW_SELECTION_CHANGED, &Client::onSubscriptionSelected, this);
  mSubscriptionsCtrl->Bind(wxEVT_DATAVIEW_ITEM_CONTEXT_MENU, &Client::onSubscriptionContext, this);
  mSubscribe->Bind(wxEVT_BUTTON, &Client::onSubscribeClicked, this);
  mFilter->Bind(wxEVT_KEY_DOWN, &Client::onSubscribeEnter, this);
}

void Client::setupPanelPreview(wxWindow *parent)
{
  auto panel = new Widgets::Edit(parent, -1, OptionsHeight);
  mPanes.insert({Panes::Preview, panel});
  panel->setReadOnly(true);
}

void Client::setupPanelPublish(wxWindow *parent)
{
  auto panel = new Widgets::Edit(parent, -1, OptionsHeight);
  mPanes.insert({Panes::Publish, panel});
  panel->Bind(wxEVT_BUTTON, &Client::onPublishClicked, this);
}

void Client::setupPanelSnippets(wxWindow *parent)
{
  wxDataViewColumn* const name = new wxDataViewColumn(
    L"name",
    new wxDataViewTextRenderer(),
    (unsigned)Models::Snippets::Column::Name,
    wxCOL_WIDTH_AUTOSIZE,
    wxALIGN_LEFT
  );

  auto panel = new wxPanel(parent, -1);
  mPanes.insert({Panes::Snippets, panel});

  mSnippetsCtrl = new wxDataViewListCtrl(panel, -1, wxDefaultPosition, wxDefaultSize, wxDV_NO_HEADER);
  mSnippetsCtrl->AppendColumn(name);

  mSnippetsModel = new Models::Snippets;
  mSnippetsModel->load(mConnection->getPath());
  mSnippetsCtrl->AssociateModel(mSnippetsModel.get());

  wxFont font(wxFontInfo(9).FaceName("Consolas"));
  mSnippetsCtrl->SetFont(font);

  wxBoxSizer *vsizer = new wxBoxSizer(wxOrientation::wxVERTICAL);
  vsizer->Add(mSnippetsCtrl, 1, wxEXPAND);
  panel->SetSizer(vsizer);
}

void Client::onPublishClicked(wxCommandEvent &event)
{
  auto publish = dynamic_cast<Widgets::Edit*>(mPanes.at(Panes::Publish));

  if (!mClient->connected()) { return; }
  auto topic = publish->getTopic();
  if (topic.empty()) { return; }

  auto payload  = publish->getPayload();
  auto qos      = publish->getQos();
  bool retained = publish->getRetained();

  mClient->publish(topic, payload, qos, retained);
}

void Client::onSubscribeClicked(wxCommandEvent &event)
{
  auto text = mFilter->GetValue();
  mFilter->SetValue("");
  auto topic = text.empty()
    ? "#"
    : text.ToStdString();
  mSubscriptionsModel->subscribe(topic, MQTT::QoS::ExactlyOnce);
}

void Client::onSubscribeEnter(wxKeyEvent &event)
{
  if (event.GetKeyCode() != WXK_RETURN)
  {
    event.Skip();
    return;
  }

  auto text = mFilter->GetValue();
  mFilter->SetValue("");
  auto topic = text.empty()
    ? "#"
    : text.ToStdString();
  mSubscriptionsModel->subscribe(topic, MQTT::QoS::ExactlyOnce);
  event.Skip();
}

void Client::onConnectClicked(wxCommandEvent &event)
{
  mConnect->Disable();

  if (mClient->connected())
  {
    mClient->disconnect();
  }
  else
  {
    mClient->setHostname(mConnection->getBrokerOptions().getHostname());
    mClient->setPort(mConnection->getBrokerOptions().getPort());
    mClient->connect();
  }
}

Client::~Client()
{
  mAuiMan->UnInit();
}

void Client::onHistorySelected(wxDataViewEvent &event)
{
  if (!event.GetItem().IsOk()) { return; }
  auto item = event.GetItem();

  auto preview = dynamic_cast<Widgets::Edit*>(mPanes.at(Panes::Preview));

  preview->setPayload(mHistoryModel->getPayload(item));
  preview->setTopic(mHistoryModel->getTopic(item));
  preview->setQos(mHistoryModel->getQos(item));
  preview->setRetained(mHistoryModel->getRetained(item));
}

void Client::onSubscriptionContext(wxDataViewEvent& dve)
{
  if (!dve.GetItem().IsOk()) { return; }

  auto item = dve.GetItem();

  mSubscriptionsCtrl->Select(item);
  bool muted = mSubscriptionsModel->getMuted(item);

  wxMenu menu;
  menu.Append((unsigned)ContextIDs::SubscriptionsUnsubscribe, "Unsubscribe");
  menu.Append((unsigned)ContextIDs::SubscriptionsChangeColor, "Color change");
  menu.Append((unsigned)ContextIDs::SubscriptionsSolo, "Solo");
  if (muted)
  {
    menu.Append((unsigned)ContextIDs::SubscriptionsUnmute, "Unmute");
  }
  else
  {
    menu.Append((unsigned)ContextIDs::SubscriptionsMute, "Mute");
  }
  PopupMenu(&menu);
}

void Client::onHistoryContext(wxDataViewEvent& dve)
{
  if (!dve.GetItem().IsOk()) { return; }

  mHistoryCtrl->Select(dve.GetItem());

  wxMenu menu;
  menu.Append((unsigned)ContextIDs::HistoryEdit, "Edit");
  menu.Append((unsigned)ContextIDs::HistoryResend, "Re-Send");
  menu.Append((unsigned)ContextIDs::HistoryRetainedClear, "Clear retained");
  PopupMenu(&menu);
}

void Client::onContextSelected(wxCommandEvent& event)
{
  switch ((ContextIDs)event.GetId())
  {
    case ContextIDs::HistoryRetainedClear: {
      wxLogMessage("Requesting clear retained");
      auto item = mHistoryCtrl->GetSelection();
      auto topic = mHistoryModel->getTopic(item);
      auto qos = mHistoryModel->getQos(item);
      mClient->publish(topic, "", qos, true);
    } break;
    case ContextIDs::SubscriptionsUnsubscribe: {
      wxLogMessage("Requesting unsubscribe");
      auto item = mSubscriptionsCtrl->GetSelection();
      mSubscriptionsModel->unsubscribe(item);
      auto selected = mHistoryCtrl->GetSelection();
      auto preview = dynamic_cast<Widgets::Edit*>(mPanes.at(Panes::Preview));
      preview->clear();
      mHistoryCtrl->Unselect(selected);
    } break;
    case ContextIDs::SubscriptionsSolo: {
      wxLogMessage("Requesting solo");
      auto item = mSubscriptionsCtrl->GetSelection();
      mSubscriptionsModel->solo(item);
    } break;
    case ContextIDs::SubscriptionsMute: {
      wxLogMessage("Requesting mute");
      auto item = mSubscriptionsCtrl->GetSelection();
      mSubscriptionsModel->mute(item);
    } break;
    case ContextIDs::SubscriptionsUnmute: {
      wxLogMessage("Requesting unmute");
      auto item = mSubscriptionsCtrl->GetSelection();
      mSubscriptionsModel->unmute(item);
    } break;
    case ContextIDs::SubscriptionsChangeColor: {
      wxLogMessage("Requesting new color");
      auto item = mSubscriptionsCtrl->GetSelection();
      wxColor color(
        (rand() % 100) + 100,
        (rand() % 100) + 100,
        (rand() % 100) + 100
      );
      mSubscriptionsModel->setColor(item, color);
      mSubscriptionsCtrl->Refresh();
      mHistoryCtrl->Refresh();
    } break;
    case ContextIDs::HistoryResend: {
      wxLogMessage("Requesting resend");
      auto item     = mHistoryCtrl->GetSelection();
      auto topic    = mHistoryModel->getTopic(item);
      auto retained = mHistoryModel->getRetained(item);
      auto qos      = mHistoryModel->getQos(item);
      auto payload  = mHistoryModel->getPayload(item);
      mClient->publish(topic, payload, qos, retained);
    } break;
    case ContextIDs::HistoryEdit: {
      wxLogMessage("Requesting edit");
      auto item = mHistoryCtrl->GetSelection();
      if (!item.IsOk()) { return; }
      auto publish = dynamic_cast<Widgets::Edit*>(mPanes.at(Panes::Publish));
      publish->clear();
      publish->setTopic(mHistoryModel->getTopic(item));
      publish->setQos(mHistoryModel->getQos(item));
      publish->setPayload(mHistoryModel->getPayload(item));
      publish->setRetained(mHistoryModel->getRetained(item));
    } break;
  }
  event.Skip(true);
}

void Client::onClose(wxCloseEvent &event)
{
  if (mClient->connected())
  {
    auto preview = dynamic_cast<Widgets::Edit*>(mPanes.at(Panes::Preview));
    preview->setPayload("Closing...");
    mClient->disconnect();
  }
  Destroy();
}

void Client::onMessage(wxDataViewItem item)
{
  if (!mAutoScroll->GetValue())
  {
    return;
  }

  mHistoryCtrl->Select(item);
  mHistoryCtrl->EnsureVisible(item);

  auto preview = dynamic_cast<Widgets::Edit*>(mPanes.at(Panes::Preview));
  preview->setPayload(mHistoryModel->getPayload(item));
  preview->setTopic(mHistoryModel->getTopic(item));
  preview->setQos(mHistoryModel->getQos(item));
  preview->setRetained(mHistoryModel->getRetained(item));
}

void Client::onConnected()
{
  auto e = new Events::Connection(Events::CONNECTED);
  wxQueueEvent(this, e);
}

void Client::onDisconnected()
{
  auto e = new Events::Connection(Events::DISCONNECTED);
  wxQueueEvent(this, e);
}

void Client::onConnectedSync(Events::Connection &e)
{
  wxLogInfo("Handling connection in GUI thread");
  mConnect->Enable();
  mConnect->SetLabelText("Disconnect");
}

void Client::onDisconnectedSync(Events::Connection &e)
{
  mConnect->Enable();
  mConnect->SetLabelText("Connect");
}

void Client::onSubscriptionSelected(wxDataViewEvent &event)
{
  auto item = mSubscriptionsCtrl->GetSelection();
  if (!item.IsOk()) { return; }
  auto f = mSubscriptionsModel->getFilter(item);
  mFilter->SetValue(f);
}

