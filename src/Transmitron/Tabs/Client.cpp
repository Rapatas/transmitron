#include "Client.hpp"

#include <sstream>
#include <wx/artprov.h>
#include <nlohmann/json.hpp>

#include "Transmitron/Resources/plus/plus-18x18.hpp"
#include "Transmitron/Resources/history/history-18x14.hpp"
#include "Transmitron/Resources/history/history-18x18.hpp"
#include "Transmitron/Resources/preview/preview-18x14.hpp"
#include "Transmitron/Resources/preview/preview-18x18.hpp"
#include "Transmitron/Resources/send/send-18x14.hpp"
#include "Transmitron/Resources/send/send-18x18.hpp"
#include "Transmitron/Resources/snippets/snippets-18x14.hpp"
#include "Transmitron/Resources/snippets/snippets-18x18.hpp"
#include "Transmitron/Resources/subscription/subscription-18x14.hpp"
#include "Transmitron/Resources/subscription/subscription-18x18.hpp"

#define wxLOG_COMPONENT "Client"

using namespace Transmitron::Tabs;
using namespace Transmitron::Models;
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

  mPanes =
  {
    {Panes::History, {
      "History",
      {},
      nullptr,
      bin2c_history_18x18_png,
      bin2c_history_18x14_png
    }},
    {Panes::Preview, {
      "Preview",
      {},
      nullptr,
      bin2c_preview_18x18_png,
      bin2c_preview_18x14_png
    }},
    {Panes::Publish, {
      "Publish",
      {},
      nullptr,
      bin2c_send_18x18_png,
      bin2c_send_18x14_png
    }},
    {Panes::Snippets, {
      "Snippets",
      {},
      nullptr,
      bin2c_snippets_18x18_png,
      bin2c_snippets_18x14_png
    }},
    {Panes::Subscriptions, {
      "Subscriptions",
      {},
      nullptr,
      bin2c_subscription_18x18_png,
      bin2c_subscription_18x14_png
    }},
  };

  for (auto &pane : mPanes)
  {
    pane.second.info.Caption(pane.second.name);
    pane.second.info.CloseButton(false);
    pane.second.info.Icon(*pane.second.icon18x14);
    pane.second.info.PaneBorder(false);

    if (pane.first == Panes::History)
    {
      pane.second.info.Movable(false);
      pane.second.info.Floatable(false);
    }
    else
    {
      pane.second.info.Movable(true);
      pane.second.info.Floatable(true);
    }
  }

  mPanes.at(Panes::History).info.Center();
  mPanes.at(Panes::Subscriptions).info.Left();
  mPanes.at(Panes::Snippets).info.Left();
  mPanes.at(Panes::Preview).info.Bottom();
  mPanes.at(Panes::Publish).info.Bottom();

  mPanes.at(Panes::History).info.Layer(0);
  mPanes.at(Panes::Subscriptions).info.Layer(1);
  mPanes.at(Panes::Snippets).info.Layer(1);
  mPanes.at(Panes::Preview).info.Layer(2);
  mPanes.at(Panes::Publish).info.Layer(2);

  mPanes.at(Panes::History).info.MinSize(wxSize(100, 100));
  mPanes.at(Panes::Subscriptions).info.MinSize(wxSize(300, 100));
  mPanes.at(Panes::Snippets).info.MinSize(wxSize(150, 100));
  mPanes.at(Panes::Preview).info.MinSize(wxSize(200, 200));
  mPanes.at(Panes::Publish).info.MinSize(wxSize(200, 200));

  auto wrapper = new wxPanel(this, -1);

  setupPanelPublish(wrapper);
  setupPanelSubscriptions(wrapper);
  setupPanelHistory(wrapper);
  setupPanelPreview(wrapper);
  setupPanelSnippets(wrapper);
  setupPanelConnect(this);

  mMasterSizer = new wxBoxSizer(wxVERTICAL);
  mMasterSizer->Add(mConnectionBar, 0, wxEXPAND);
  mMasterSizer->Add(wrapper, 1, wxEXPAND);
  SetSizer(mMasterSizer);

  mAuiMan.SetManagedWindow(wrapper);
  for (const auto &pane : mPanes)
  {
    mAuiMan.AddPane(pane.second.panel, pane.second.info);
  }
  mAuiMan.Update();
}

Client::~Client()
{
  mAuiMan.UnInit();
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
    new wxDataViewIconTextRenderer(),
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

  wxFont font(wxFontInfo(9).FaceName("Consolas"));

  auto panel = new wxPanel(parent, -1, wxDefaultPosition);
  mPanes.at(Panes::History).panel = panel;

  mHistoryCtrl = new wxDataViewCtrl(
    panel,
    -1,
    wxDefaultPosition,
    wxDefaultSize,
    wxDV_NO_HEADER | wxDV_ROW_LINES
  );

  mHistoryModel = new Models::History(mSubscriptionsModel);
  mHistoryModel->attachObserver(this);
  mHistoryCtrl->AssociateModel(mHistoryModel.get());

  mHistoryCtrl->SetFont(font);

  mHistoryCtrl->AppendColumn(icon);
  mHistoryCtrl->AppendColumn(qos);
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

  mHistoryCtrl->Bind(
    wxEVT_DATAVIEW_SELECTION_CHANGED,
    &Client::onHistorySelected,
    this
  );
  mHistoryCtrl->Bind(
    wxEVT_DATAVIEW_ITEM_CONTEXT_MENU,
    &Client::onHistoryContext,
    this
  );
}

void Client::setupPanelConnect(wxWindow *parent)
{
  mConnectionBar = new wxPanel(parent, -1);

  mConnect  = new wxButton(mConnectionBar, -1, "Connect");

  auto cb = [this](Panes pane, wxCommandEvent &e)
  {
    auto widget = mPanes.at(pane);

    auto currentInfo = mAuiMan.GetPane(widget.panel);

    if (currentInfo.IsOk())
    {
      if (currentInfo.IsDocked())
      {
        mPanes.at(pane).info = currentInfo;
      }
      mAuiMan.DetachPane(widget.panel);
      widget.panel->Show(false);
      widget.toggle->SetBackgroundColour(wxColor(150, 150, 150));
    }
    else
    {
      widget.panel->Show(true);
      mAuiMan.AddPane(widget.panel, mPanes.at(pane).info);
      widget.toggle->SetBackgroundColour(wxNullColour);
    }
    mAuiMan.Update();
  };

  wxBoxSizer *hsizer = new wxBoxSizer(wxOrientation::wxHORIZONTAL);
  hsizer->SetMinSize(0, OptionsHeight);
  hsizer->Add(mConnect, 0, wxEXPAND);

  for (auto &pane : mPanes)
  {
    if (pane.first == Panes::History)
    {
      continue;
    }

    auto bitmap = mPanes.at(pane.first).icon18x18;
    auto button = new wxButton(
      mConnectionBar,
      -1,
      "",
      wxDefaultPosition,
      wxSize(OptionsHeight, OptionsHeight)
    );
    button->SetToolTip(pane.second.name);
    button->SetBitmap(*bitmap);
    button->Bind(
      wxEVT_BUTTON,
      std::bind(cb, pane.first, std::placeholders::_1)
    );
    hsizer->Add(button, 0, wxEXPAND);

    pane.second.toggle = button;
  }

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
  mPanes.at(Panes::Subscriptions).panel = panel;

  mSubscriptionsCtrl = new wxDataViewListCtrl(
    panel,
    -1,
    wxDefaultPosition,
    wxDefaultSize,
    wxDV_NO_HEADER
  );
  mSubscriptionsCtrl->AppendColumn(icon);
  mSubscriptionsCtrl->AppendColumn(qos);
  mSubscriptionsCtrl->AppendColumn(topic);

  mSubscriptionsModel = new Models::Subscriptions(mClient);
  mSubscriptionsCtrl->AssociateModel(mSubscriptionsModel.get());

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
  hsizer->Add(mFilter, 1, wxEXPAND);
  hsizer->Add(mSubscribe, 0, wxEXPAND);
  vsizer->Add(hsizer, 0, wxEXPAND);
  vsizer->Add(mSubscriptionsCtrl,  1, wxEXPAND);
  panel->SetSizer(vsizer);

  mSubscriptionsCtrl->Bind(
    wxEVT_DATAVIEW_SELECTION_CHANGED,
    &Client::onSubscriptionSelected,
    this
  );
  mSubscriptionsCtrl->Bind(
    wxEVT_DATAVIEW_ITEM_CONTEXT_MENU,
    &Client::onSubscriptionContext,
    this
  );
  mSubscribe->Bind(wxEVT_BUTTON, &Client::onSubscribeClicked, this);
  mFilter->Bind(wxEVT_KEY_DOWN, &Client::onSubscribeEnter, this);
}

void Client::setupPanelPreview(wxWindow *parent)
{
  auto panel = new Widgets::Edit(parent, -1, OptionsHeight);
  mPanes.at(Panes::Preview).panel = panel;
  panel->setReadOnly(true);
  panel->Bind(Events::EDIT_SAVE_SNIPPET, &Client::onPreviewSaveSnippet, this);
}

void Client::setupPanelPublish(wxWindow *parent)
{
  auto panel = new Widgets::Edit(parent, -1, OptionsHeight);
  mPanes.at(Panes::Publish).panel = panel;
  panel->Bind(Events::EDIT_PUBLISH, &Client::onPublishClicked, this);
  panel->Bind(Events::EDIT_SAVE_SNIPPET, &Client::onPublishSaveSnippet, this);
}

void Client::setupPanelSnippets(wxWindow *parent)
{
  auto renderer = new wxDataViewIconTextRenderer(
    wxDataViewIconTextRenderer::GetDefaultType(),
    wxDATAVIEW_CELL_EDITABLE
  );

  mSnippetColumns.at(Snippets::Column::Name) = new wxDataViewColumn(
    L"name",
    renderer,
    (unsigned)Models::Snippets::Column::Name,
    wxCOL_WIDTH_AUTOSIZE,
    wxALIGN_LEFT
  );

  auto panel = new wxPanel(parent, -1);
  mPanes.at(Panes::Snippets).panel = panel;

  mSnippetsCtrl = new wxDataViewListCtrl(
    panel,
    -1,
    wxDefaultPosition,
    wxDefaultSize,
    wxDV_NO_HEADER
  );
  mSnippetsCtrl->AppendColumn(mSnippetColumns.at(Snippets::Column::Name));

  mSnippetsModel = new Models::Snippets;
  mSnippetsModel->load(mConnection->getPath());
  mSnippetsCtrl->AssociateModel(mSnippetsModel.get());

  wxFont font(wxFontInfo(9).FaceName("Consolas"));
  mSnippetsCtrl->SetFont(font);

  wxBoxSizer *vsizer = new wxBoxSizer(wxOrientation::wxVERTICAL);
  vsizer->Add(mSnippetsCtrl, 1, wxEXPAND);
  panel->SetSizer(vsizer);
  mSnippetsCtrl->Bind(
    wxEVT_DATAVIEW_ITEM_CONTEXT_MENU,
    &Client::onSnippetsContext,
    this
  );
  mSnippetsCtrl->Bind(
    wxEVT_DATAVIEW_ITEM_START_EDITING,
    &Client::onSnippetsEdit,
    this
  );
  mSnippetsCtrl->Bind(
    wxEVT_DATAVIEW_SELECTION_CHANGED,
    &Client::onSnippetsSelected,
    this
  );
}

void Client::onPublishClicked(wxCommandEvent &event)
{
  auto publish = dynamic_cast<Widgets::Edit*>(mPanes.at(Panes::Publish).panel);

  if (!mClient->connected()) { return; }
  auto topic = publish->getTopic();
  if (topic.empty()) { return; }

  auto payload  = publish->getPayload();
  auto qos      = publish->getQos();
  bool retained = publish->getRetained();

  mClient->publish(topic, payload, qos, retained);
}

void Client::onPublishSaveSnippet(Events::Edit &e)
{
  wxLogInfo("Saving current message");
  auto snippetItem = mSnippetsCtrl->GetSelection();
  if (!mSnippetsModel->IsContainer(snippetItem))
  {
    wxLogInfo("Selecting parent");
    snippetItem = mSnippetsModel->GetParent(snippetItem);
  }

  auto publish = dynamic_cast<Widgets::Edit*>(
    mPanes.at(Panes::Publish).panel
  );

  auto message = std::make_shared<MQTT::Message>(publish->getMessage());
  wxLogInfo("Storing %s", message->topic);
  auto inserted = mSnippetsModel->createSnippet(snippetItem, message);
  if (inserted.IsOk())
  {
    auto publish = dynamic_cast<Widgets::Edit*>(
      mPanes.at(Panes::Publish).panel
    );
    publish->setMessage(mSnippetsModel->getMessage(inserted));

    mSnippetsCtrl->Select(inserted);
    mSnippetsCtrl->EnsureVisible(inserted);
    auto nameColumn = mSnippetColumns.at(Snippets::Column::Name);
    mSnippetExplicitEditRequest = true;
    mSnippetsCtrl->EditItem(inserted, nameColumn);
  }
}

void Client::onPreviewSaveSnippet(Events::Edit &e)
{
  wxLogInfo("Saving current message");
  auto snippetItem = mSnippetsCtrl->GetSelection();
  if (!mSnippetsModel->IsContainer(snippetItem))
  {
    wxLogInfo("Selecting parent");
    snippetItem = mSnippetsModel->GetParent(snippetItem);
  }

  auto preview = dynamic_cast<Widgets::Edit*>(
    mPanes.at(Panes::Preview).panel
  );

  auto message = std::make_shared<MQTT::Message>(preview->getMessage());
  wxLogInfo("Storing %s", message->topic);
  auto inserted = mSnippetsModel->createSnippet(snippetItem, message);
  if (inserted.IsOk())
  {
    mSnippetsCtrl->Select(inserted);
    mSnippetsCtrl->EnsureVisible(inserted);
    auto nameColumn = mSnippetColumns.at(Snippets::Column::Name);
    mSnippetExplicitEditRequest = true;
    mSnippetsCtrl->EditItem(inserted, nameColumn);
  }
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
    mClient->setUsername(mConnection->getBrokerOptions().getUsername());
    mClient->setPassword(mConnection->getBrokerOptions().getPassword());
    mClient->connect();
  }
}

void Client::onHistorySelected(wxDataViewEvent &event)
{
  if (!event.GetItem().IsOk()) { return; }
  auto item = event.GetItem();

  auto preview = dynamic_cast<Widgets::Edit*>(mPanes.at(Panes::Preview).panel);

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

void Client::onHistoryContext(wxDataViewEvent& e)
{
  if (!e.GetItem().IsOk()) { return; }

  mHistoryCtrl->Select(e.GetItem());

  wxMenu menu;
  menu.Append((unsigned)ContextIDs::HistoryEdit, "Edit");
  menu.Append((unsigned)ContextIDs::HistoryResend, "Re-Send");
  menu.Append((unsigned)ContextIDs::HistoryRetainedClear, "Clear retained");
  menu.Append((unsigned)ContextIDs::HistorySaveSnippet, "Save snippet");
  PopupMenu(&menu);
}

void Client::onSnippetsContext(wxDataViewEvent& e)
{
  wxMenu menu;

  auto newFolder = new wxMenuItem(
    nullptr,
    (unsigned)ContextIDs::SnippetNewFolder,
    "New Folder"
  );
  newFolder->SetBitmap(wxArtProvider::GetBitmap(wxART_NEW_DIR));
  menu.Append(newFolder);

  auto newSnippet = new wxMenuItem(
    nullptr,
    (unsigned)ContextIDs::SnippetNewSnippet,
    "New Snippet"
  );
  newSnippet->SetBitmap(wxArtProvider::GetBitmap(wxART_NORMAL_FILE));
  menu.Append(newSnippet);

  if (e.GetItem().IsOk())
  {
    auto item = mSnippetsCtrl->GetSelection();

    if (item != mSnippetsModel->getRootItem())
    {
      auto rename = new wxMenuItem(
        nullptr,
        (unsigned)ContextIDs::SnippetRename,
        "Rename"
      );
      rename->SetBitmap(wxArtProvider::GetBitmap(wxART_EDIT));
      menu.Append(rename);

      auto del = new wxMenuItem(
        nullptr,
        (unsigned)ContextIDs::SnippetDelete,
        "Delete"
      );
      del->SetBitmap(wxArtProvider::GetBitmap(wxART_DELETE));
      menu.Append(del);
    }
  }
  else
  {
    mSnippetsCtrl->UnselectAll();
  }

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
      auto preview = dynamic_cast<Widgets::Edit*>(
        mPanes.at(Panes::Preview).panel
      );
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
      auto publish = dynamic_cast<Widgets::Edit*>(
        mPanes.at(Panes::Publish).panel
      );
      publish->clear();
      publish->setTopic(mHistoryModel->getTopic(item));
      publish->setQos(mHistoryModel->getQos(item));
      publish->setPayload(mHistoryModel->getPayload(item));
      publish->setRetained(mHistoryModel->getRetained(item));
    } break;
    case ContextIDs::HistorySaveSnippet: {
      wxLogMessage("Requesting save snippet");
      auto historyItem = mHistoryCtrl->GetSelection();
      auto snippetItem = mSnippetsCtrl->GetSelection();
      if (!mSnippetsModel->IsContainer(snippetItem))
      {
        wxLogInfo("Selecting parent");
        snippetItem = mSnippetsModel->GetParent(snippetItem);
      }
      auto message = mHistoryModel->getMessage(historyItem);
      auto inserted = mSnippetsModel->createSnippet(snippetItem, message);
      if (inserted.IsOk())
      {
        auto publish = dynamic_cast<Widgets::Edit*>(
          mPanes.at(Panes::Publish).panel
        );
        publish->setMessage(mSnippetsModel->getMessage(inserted));

        mSnippetsCtrl->Select(inserted);
        mSnippetsCtrl->EnsureVisible(inserted);
        auto nameColumn = mSnippetColumns.at(Snippets::Column::Name);
        mSnippetExplicitEditRequest = true;
        mSnippetsCtrl->EditItem(inserted, nameColumn);
      }

    } break;
    case ContextIDs::SnippetNewFolder: {
      wxLogMessage("Requesting new folder");
      auto item = mSnippetsCtrl->GetSelection();
      if (!mSnippetsModel->IsContainer(item))
      {
        wxLogInfo("Selecting parent");
        item = mSnippetsModel->GetParent(item);
      }
      auto inserted = mSnippetsModel->createFolder(item);
      if (inserted.IsOk())
      {
        mSnippetsCtrl->Select(inserted);
        mSnippetsCtrl->EnsureVisible(inserted);
        auto nameColumn = mSnippetColumns.at(Snippets::Column::Name);
        mSnippetExplicitEditRequest = true;
        mSnippetsCtrl->EditItem(inserted, nameColumn);
      }
    } break;
    case ContextIDs::SnippetNewSnippet: {
      wxLogMessage("Requesting new snippet");
      auto item = mSnippetsCtrl->GetSelection();
      if (!mSnippetsModel->IsContainer(item))
      {
        wxLogInfo("Selecting parent");
        item = mSnippetsModel->GetParent(item);
      }
      auto inserted = mSnippetsModel->createSnippet(item, nullptr);
      if (inserted.IsOk())
      {
        auto publish = dynamic_cast<Widgets::Edit*>(
          mPanes.at(Panes::Publish).panel
        );
        publish->setMessage(mSnippetsModel->getMessage(inserted));

        mSnippetsCtrl->Select(inserted);
        mSnippetsCtrl->EnsureVisible(inserted);
        auto nameColumn = mSnippetColumns.at(Snippets::Column::Name);
        mSnippetExplicitEditRequest = true;
        mSnippetsCtrl->EditItem(inserted, nameColumn);
      }
    } break;
    case ContextIDs::SnippetDelete: {
      wxLogMessage("Requesting delete");
      auto item = mSnippetsCtrl->GetSelection();
      bool done = mSnippetsModel->remove(item);
      if (done)
      {
        auto root = mSnippetsModel->getRootItem();
        mSnippetsCtrl->Select(root);
      }
    } break;
    case ContextIDs::SnippetRename: {
      wxLogMessage("Requesting rename");
      mSnippetExplicitEditRequest = true;
      auto item = mSnippetsCtrl->GetSelection();
      mSnippetsCtrl->EditItem(item, mSnippetColumns.at(Snippets::Column::Name));
    } break;
  }
  event.Skip(true);
}

void Client::onSnippetsSelected(wxDataViewEvent &e)
{
  auto item = e.GetItem();
  if (!item.IsOk())
  {
    e.Skip();
    return;
  }

  auto publish = dynamic_cast<Widgets::Edit*>(
    mPanes.at(Panes::Publish).panel
  );

  if (!mSnippetsModel->IsContainer(item))
  {
    const auto &message = mSnippetsModel->getMessage(item);
    publish->setMessage(message);
  }
}

void Client::onSnippetsEdit(wxDataViewEvent &e)
{
  if (mSnippetExplicitEditRequest)
  {
    mSnippetExplicitEditRequest = false;
    e.Skip();
  }
  else
  {
    e.Veto();
  }
}

void Client::onClose(wxCloseEvent &event)
{
  if (mClient->connected())
  {
    auto preview = dynamic_cast<Widgets::Edit*>(
      mPanes.at(Panes::Preview).panel
    );
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

  auto preview = dynamic_cast<Widgets::Edit*>(mPanes.at(Panes::Preview).panel);
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

