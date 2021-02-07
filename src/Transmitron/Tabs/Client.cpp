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

#if not BUILD_DOCKING

  mSplitCenter = new wxSplitterWindow(this);
  mSplitTop = new wxSplitterWindow(mSplitCenter);
  mSplitBottom = new wxSplitterWindow(mSplitCenter);
  mSplitHistory = new wxSplitterWindow(mSplitTop);

  mSplitTop->SetMinimumPaneSize(100);
  mSplitCenter->SetMinimumPaneSize(100);
  mSplitBottom->SetMinimumPaneSize(100);
  mSplitHistory->SetMinimumPaneSize(100);

  mSplitCenter->SetSashGravity(0.5);
  mSplitTop->SetSashGravity(0.5);
  mSplitBottom->SetSashGravity(0.5);

  setupPanelConnect(this);
  setupPanelPublish(mSplitBottom);
  setupPanelHistory(mSplitHistory);
  setupPanelSubscriptions(mSplitHistory);
  setupPanelPreview(mSplitTop);
  setupPanelSnippets(mSplitBottom);

  auto s = new wxBoxSizer(wxVERTICAL);
  s->Add(mConnectionBar, 0, wxEXPAND);
  s->Add(mSplitCenter, 1, wxEXPAND);
  SetSizer(s);

#else

  setupPanelConnect(this);
  setupPanelPublish(this);
  setupPanelHistory(this);
  setupPanelSubscriptions(this);
  setupPanelPreview(this);
  setupPanelSnippets(this);

  wxAuiPaneInfo connectionInfo;
  wxAuiPaneInfo historyInfo;
  wxAuiPaneInfo previewInfo;
  wxAuiPaneInfo publishInfo;
  wxAuiPaneInfo subscriptionsInfo;
  wxAuiPaneInfo snippetsInfo;

  connectionInfo.Caption("Connection");
  historyInfo.Caption("History");
  previewInfo.Caption("Preview");
  publishInfo.Caption("Publish");
  subscriptionsInfo.Caption("Subscriptions");
  snippetsInfo.Caption("Snippets");

  connectionInfo.Movable(true);
  historyInfo.Movable(true);
  previewInfo.Movable(true);
  publishInfo.Movable(true);
  subscriptionsInfo.Movable(true);
  snippetsInfo.Movable(true);

  connectionInfo.Floatable(true);
  historyInfo.Floatable(true);
  previewInfo.Floatable(true);
  publishInfo.Floatable(true);
  subscriptionsInfo.Floatable(true);
  snippetsInfo.Floatable(true);

  previewInfo.Center();
  connectionInfo.Left();
  historyInfo.Left();
  publishInfo.Left();
  subscriptionsInfo.Left();
  snippetsInfo.Left();

  previewInfo.Layer(0);
  connectionInfo.Layer(1);
  historyInfo.Layer(1);
  publishInfo.Layer(2);
  subscriptionsInfo.Layer(1);
  snippetsInfo.Layer(3);

  publishInfo.BestSize(wxSize(300, 500));
  connectionInfo.MaxSize(wxSize(300, 80));
  subscriptionsInfo.BestSize(wxSize(500, 0));

  mAuiMan.SetManagedWindow(this);
  mAuiMan.AddPane(mConnectionBar,    connectionInfo);
  mAuiMan.AddPane(mSubscriptions, subscriptionsInfo);
  mAuiMan.AddPane(mPublish,    publishInfo);
  mAuiMan.AddPane(mHistory,       historyInfo);
  mAuiMan.AddPane(mPreview,       previewInfo);
  mAuiMan.AddPane(mSnippets,      snippetsInfo);
  mAuiMan.Update();

#endif
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

  mHistory = new wxPanel(parent, -1, wxDefaultPosition);

  mHistoryCtrl = new wxDataViewCtrl(
    mHistory,
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

  wxBoxSizer *vsizer = new wxBoxSizer(wxOrientation::wxVERTICAL);
  vsizer->Add(mHistoryCtrl, 1, wxEXPAND);
  mHistory->SetSizer(vsizer);

  mHistoryCtrl->Bind(wxEVT_DATAVIEW_SELECTION_CHANGED, &Client::onHistorySelected, this);
  mHistoryCtrl->Bind(wxEVT_DATAVIEW_ITEM_CONTEXT_MENU, &Client::onHistoryContext, this);
}

void Client::setupPanelConnect(wxWindow *parent)
{
  mConnectionBar = new wxPanel(parent, -1, wxDefaultPosition, wxDefaultSize);

  mConnect  = new wxButton(mConnectionBar, -1, "Connect");

  wxBoxSizer *hsizer = new wxBoxSizer(wxOrientation::wxHORIZONTAL);
  hsizer->Add(mConnect,  0, wxEXPAND);
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

  mSubscriptions = new wxPanel(parent, -1, wxDefaultPosition);

  mSubscriptionsCtrl = new wxDataViewListCtrl(mSubscriptions, -1, wxDefaultPosition, wxDefaultSize, wxDV_NO_HEADER);
  mSubscriptionsCtrl->AppendColumn(icon);
  mSubscriptionsCtrl->AppendColumn(qos);
  mSubscriptionsCtrl->AppendColumn(topic);

  mSubscriptionsModel = new Models::Subscriptions(mClient, mHistoryModel);
  mSubscriptionsCtrl->AssociateModel(mSubscriptionsModel);

  mSubscriptionsCtrl->SetFont(font);

  mSubscribe = new wxBitmapButton(mSubscriptions, -1, *bin2c_plus_18x18_png);

  mFilter = new Widgets::TopicCtrl(mSubscriptions, -1);
  mFilter->SetHint("subscribe");
  mFilter->SetFont(font);

  wxBoxSizer *vsizer = new wxBoxSizer(wxOrientation::wxVERTICAL);
  wxBoxSizer *hsizer = new wxBoxSizer(wxOrientation::wxHORIZONTAL);
  hsizer->Add(mFilter,  1, wxEXPAND);
  hsizer->Add(mSubscribe,  0, wxEXPAND);
  vsizer->Add(hsizer,  0, wxEXPAND);
  vsizer->Add(mSubscriptionsCtrl,  1, wxEXPAND);
  mSubscriptions->SetSizer(vsizer);

  mSubscriptionsCtrl->Bind(wxEVT_DATAVIEW_SELECTION_CHANGED, &Client::onSubscriptionSelected, this);
  mSubscriptionsCtrl->Bind(wxEVT_DATAVIEW_ITEM_CONTEXT_MENU, &Client::onSubscriptionContext, this);
  mSubscribe->Bind(wxEVT_BUTTON, &Client::onSubscribeClicked, this);
  mFilter->Bind(wxEVT_KEY_DOWN, &Client::onSubscribeEnter, this);
}

void Client::setupPanelPreview(wxWindow *parent)
{
  mPreview = new Widgets::Edit(parent, -1);
  mPreview->setReadOnly(true);
}

void Client::setupPanelPublish(wxWindow *parent)
{
  mPublish = new Widgets::Edit(parent, -1);

  mPublish->Bind(wxEVT_BUTTON, &Client::onPublishClicked, this);
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

  mSnippets = new wxPanel(parent, -1);

  mSnippetsCtrl = new wxDataViewListCtrl(mSnippets, -1, wxDefaultPosition, wxDefaultSize, wxDV_NO_HEADER);
  mSnippetsCtrl->AppendColumn(name);

  mSnippetsModel = new Models::Snippets;
  mSnippetsModel->load(mConnection->getPath());
  mSnippetsCtrl->AssociateModel(mSnippetsModel.get());

  wxFont font(wxFontInfo(9).FaceName("Consolas"));
  mSnippetsCtrl->SetFont(font);

  wxBoxSizer *vsizer = new wxBoxSizer(wxOrientation::wxVERTICAL);
  vsizer->Add(mSnippetsCtrl, 1, wxEXPAND);
  mSnippets->SetSizer(vsizer);
}

void Client::onPublishClicked(wxCommandEvent &event)
{
  if (!mClient->connected()) { return; }
  auto topic = mPublish->getTopic();
  if (topic.empty()) { return; }

  auto payload  = mPublish->getPayload();
  auto qos      = mPublish->getQos();
  bool retained = mPublish->getRetained();

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
#if BUILD_DOCKING

  mAuiMan.UnInit();

#endif
}

void Client::onHistorySelected(wxDataViewEvent &event)
{
  if (!event.GetItem().IsOk()) { return; }
  auto item = event.GetItem();

  mPreview->setPayload(mHistoryModel->getPayload(item));
  mPreview->setTopic(mHistoryModel->getTopic(item));
  mPreview->setQos(mHistoryModel->getQos(item));
  mPreview->setRetained(mHistoryModel->getRetained(item));
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
      mPreview->clear();
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
      mPublish->clear();
      mPublish->setTopic(mHistoryModel->getTopic(item));
      mPublish->setQos(mHistoryModel->getQos(item));
      mPublish->setPayload(mHistoryModel->getPayload(item));
      mPublish->setRetained(mHistoryModel->getRetained(item));
    } break;
  }
  event.Skip(true);
}

void Client::onClose(wxCloseEvent &event)
{
  if (mClient->connected())
  {
    mPreview->setPayload("Closing...");
    mClient->disconnect();
  }
  Destroy();
}

void Client::onMessage(wxDataViewItem item)
{
  mHistoryCtrl->Select(item);
  mHistoryCtrl->EnsureVisible(item);

  mPreview->setPayload(mHistoryModel->getPayload(item));
  mPreview->setTopic(mHistoryModel->getTopic(item));
  mPreview->setQos(mHistoryModel->getQos(item));
  mPreview->setRetained(mHistoryModel->getRetained(item));
}

void Client::resize() const
{
#if not BUILD_DOCKING

  mSplitCenter->SplitHorizontally(mSplitTop, mSplitBottom);
  mSplitTop->SplitVertically(mSplitHistory, mPreview);
  mSplitBottom->SplitVertically(mSnippets, mPublish);
  mSplitHistory->SplitHorizontally(mSubscriptions, mHistory);

#endif
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

