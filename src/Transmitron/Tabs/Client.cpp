#include <nlohmann/json.hpp>
#include <wx/artprov.h>
#include <wx/clipbrd.h>

#include "Client.hpp"
#include "Helpers/Helpers.hpp"
#include "Helpers/Log.hpp"
#include "MQTT/Message.hpp"
#include "Transmitron/Events/Layout.hpp"
#include "Transmitron/Resources/history/history-18x14.hpp"
#include "Transmitron/Resources/history/history-18x18.hpp"
#include "Transmitron/Resources/plus/plus-18x18.hpp"
#include "Transmitron/Resources/preview/preview-18x14.hpp"
#include "Transmitron/Resources/preview/preview-18x18.hpp"
#include "Transmitron/Resources/send/send-18x14.hpp"
#include "Transmitron/Resources/send/send-18x18.hpp"
#include "Transmitron/Resources/snippets/snippets-18x14.hpp"
#include "Transmitron/Resources/snippets/snippets-18x18.hpp"
#include "Transmitron/Resources/subscription/subscription-18x14.hpp"
#include "Transmitron/Resources/subscription/subscription-18x18.hpp"

using namespace Transmitron::Tabs;
using namespace Transmitron::Models;
using namespace Transmitron;

constexpr size_t FontSize = 9;

wxDEFINE_EVENT(Events::CONNECTED, Events::Connection); // NOLINT
wxDEFINE_EVENT(Events::DISCONNECTED, Events::Connection); // NOLINT
wxDEFINE_EVENT(Events::LOST, Events::Connection); // NOLINT
wxDEFINE_EVENT(Events::FAILURE, Events::Connection); // NOLINT

Client::Client(
  wxWindow* parent,
  const MQTT::BrokerOptions &brokerOptions,
  const wxObjectDataPtr<Models::Snippets> &snippetsModel,
  const wxObjectDataPtr<Models::Layouts> &layoutsModel,
  const wxString &name,
  bool darkMode
) :
  wxPanel(
    parent,
    -1,
    wxDefaultPosition,
    wxDefaultSize,
    wxTAB_TRAVERSAL,
    name
  ),
  mName(name),
  mBrokerOptions(brokerOptions),
  mFont(wxFontInfo(FontSize).FaceName("Consolas")),
  mDarkMode(darkMode),
  mSnippetsModel(snippetsModel)
{
  mLogger = Helpers::Log::create("Transmitron::Client");

  mClient = std::make_shared<MQTT::Client>();
  Bind(Events::CONNECTED, &Client::onConnectedSync, this);
  Bind(Events::DISCONNECTED, &Client::onDisconnectedSync, this);
  Bind(Events::LOST, &Client::onConnectionLostSync, this);
  Bind(Events::FAILURE, &Client::onConnectionFailureSync, this);
  mMqttObserverId = mClient->attachObserver(this);
  Bind(wxEVT_CLOSE_WINDOW, &Client::onClose, this);
  Bind(wxEVT_COMMAND_MENU_SELECTED, &Client::onContextSelected, this);

  mPanes =
  {
    {Panes::History, {
      "History",
      {},
      nullptr,
      bin2cHistory18x18(),
      bin2cHistory18x14(),
      nullptr
    }},
    {Panes::Preview, {
      "Preview",
      {},
      nullptr,
      bin2cPreview18x18(),
      bin2cPreview18x14(),
      nullptr
    }},
    {Panes::Publish, {
      "Publish",
      {},
      nullptr,
      bin2cSend18x18(),
      bin2cSend18x14(),
      nullptr
    }},
    {Panes::Snippets, {
      "Snippets",
      {},
      nullptr,
      bin2cSnippets18x18(),
      bin2cSnippets18x14(),
      nullptr
    }},
    {Panes::Subscriptions, {
      "Subscriptions",
      {},
      nullptr,
      bin2cSubscription18x18(),
      bin2cSubscription18x14(),
      nullptr
    }},
  };

  mPanes.at(Panes::History).info.Center();
  mPanes.at(Panes::Subscriptions).info.Left();
  mPanes.at(Panes::Snippets).info.Left();
  mPanes.at(Panes::Publish).info.Bottom();
  mPanes.at(Panes::Preview).info.Bottom();

  mPanes.at(Panes::History).info.Layer(0);
  mPanes.at(Panes::Subscriptions).info.Layer(1);
  mPanes.at(Panes::Snippets).info.Layer(1);
  mPanes.at(Panes::Publish).info.Layer(2);
  mPanes.at(Panes::Preview).info.Layer(2);

  for (auto &pane : mPanes)
  {
    pane.second.info.Name(pane.second.name);

    const bool isEditor =
      pane.first == Panes::Preview
      || pane.first == Panes::Publish;

    const auto minSize = isEditor
      ? wxSize(PaneMinWidth, EditorMinHeight)
      : wxSize(PaneMinWidth, PaneMinHeight);

    pane.second.info.MinSize(minSize);
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

  auto *wrapper = new wxPanel(this, -1);

  setupPanelSnippets(wrapper);
  setupPanelPublish(wrapper);
  setupPanelSubscriptions(wrapper);
  setupPanelPreview(wrapper);
  setupPanelHistory(wrapper);
  setupPanelConnect(this, layoutsModel);

  mMasterSizer = new wxBoxSizer(wxVERTICAL);
  mMasterSizer->Add(mProfileBar, 0, wxEXPAND);
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
  if (mClient != nullptr)
  {
    mClient->disconnect();
    mClient->detachObserver(mMqttObserverId);
  }
  mAuiMan.UnInit();
}

// Private {

// Setup {

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

  auto *panel = new wxPanel(parent, -1, wxDefaultPosition);
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

  mHistoryCtrl->SetFont(mFont);

  mHistoryCtrl->AppendColumn(icon);
  mHistoryCtrl->AppendColumn(qos);
  mHistoryCtrl->AppendColumn(topic);

  mHistorySearchFilter = new Widgets::TopicCtrl(panel, -1);
  mHistorySearchFilter->SetHint("Filter...");
  mHistorySearchFilter->Bind(
    wxEVT_KEY_DOWN,
    &Client::onHistorySearchKey,
    this
  );

  mHistorySearchButton = new wxButton(
    panel,
    -1,
    "",
    wxDefaultPosition,
    wxSize(OptionsHeight, OptionsHeight)
  );
  mHistorySearchButton->SetBitmap(wxArtProvider::GetBitmap(wxART_FIND));
  mHistorySearchButton->Bind(
    wxEVT_BUTTON,
    &Client::onHistorySearchButton,
    this
  );

  mAutoScroll = new wxCheckBox(panel, -1, "auto-scroll");
  mAutoScroll->SetValue(true);

  mHistoryClear = new wxButton(panel, -1, "Clear");
  mHistoryClear->SetBitmap(wxArtProvider::GetBitmap(wxART_DELETE));

  auto *topSizer = new wxBoxSizer(wxOrientation::wxHORIZONTAL);
  topSizer->Add(mHistorySearchFilter, 1, wxEXPAND);
  topSizer->Add(mHistorySearchButton, 0, wxEXPAND);
  auto *hsizer = new wxBoxSizer(wxOrientation::wxHORIZONTAL);
  hsizer->SetMinSize(0, OptionsHeight);
  hsizer->Add(mAutoScroll, 0, wxEXPAND);
  hsizer->AddStretchSpacer(1);
  hsizer->Add(mHistoryClear, 0, wxEXPAND);
  auto *vsizer = new wxBoxSizer(wxOrientation::wxVERTICAL);
  vsizer->Add(topSizer, 0, wxEXPAND);
  vsizer->Add(mHistoryCtrl, 1, wxEXPAND);
  vsizer->Add(hsizer, 0, wxEXPAND);
  panel->SetSizer(vsizer);

  mHistoryCtrl->Bind(
    wxEVT_DATAVIEW_SELECTION_CHANGED,
    &Client::onHistorySelected,
    this
  );
  mHistoryCtrl->Bind(
    wxEVT_DATAVIEW_ITEM_ACTIVATED,
    &Client::onHistoryDoubleClicked,
    this
  );
  mHistoryCtrl->Bind(
    wxEVT_DATAVIEW_ITEM_CONTEXT_MENU,
    &Client::onHistoryContext,
    this
  );
  mHistoryClear->Bind(
    wxEVT_BUTTON,
    &Client::onHistoryClearClicked,
    this
  );
}

void Client::setupPanelConnect(
  wxWindow *parent,
  const wxObjectDataPtr<Models::Layouts> &layoutsModel
) {
  mProfileBar = new wxPanel(parent, -1);

  mConnect    = new wxButton(mProfileBar, -1, "Connect");
  mDisconnect = new wxButton(mProfileBar, -1, "Disconnect");
  mCancel     = new wxButton(mProfileBar, -1, "Cancel");

  mLayouts = new Widgets::Layouts(mProfileBar, -1, layoutsModel, mAuiMan, OptionsHeight);
  mLayouts->Bind(Events::LAYOUT_SELECTED, &Client::onLayoutSelected, this);
  mLayouts->Bind(Events::LAYOUT_RESIZED,  &Client::onLayoutResized,  this);

  auto cb = [this](Panes pane, wxCommandEvent &/* event */)
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
      constexpr uint8_t ColorChannelHalf = 255 / 2;
      widget.toggle->SetBackgroundColour(wxColor(
        ColorChannelHalf,
        ColorChannelHalf,
        ColorChannelHalf
      ));
    }
    else
    {
      widget.panel->Show(true);
      mAuiMan.AddPane(widget.panel, mPanes.at(pane).info);
      widget.toggle->SetBackgroundColour(wxNullColour);
    }
    mAuiMan.Update();
  };

  mProfileSizer = new wxBoxSizer(wxOrientation::wxHORIZONTAL);
  mProfileSizer->SetMinSize(0, OptionsHeight);
  mProfileSizer->Add(mConnect, 0, wxEXPAND);
  mProfileSizer->Add(mDisconnect, 0, wxEXPAND);
  mProfileSizer->Add(mCancel, 0, wxEXPAND);
  allowConnect();

  for (auto &pane : mPanes)
  {
    if (pane.first == Panes::History)
    {
      continue;
    }

    const auto *bitmap = mPanes.at(pane.first).icon18x18;
    auto *button = new wxButton(
      mProfileBar,
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
    mProfileSizer->Add(button, 0, wxEXPAND);

    pane.second.toggle = button;
  }

  mProfileSizer->AddStretchSpacer(1);
  mProfileSizer->Add(mLayouts, 0, wxEXPAND);

  mProfileBar->SetSizer(mProfileSizer);

  mConnect->Bind(
    wxEVT_BUTTON,
    &Client::onConnectClicked,
    this
  );
  mDisconnect->Bind(
    wxEVT_BUTTON,
    &Client::onDisconnectClicked,
    this
  );
  mCancel->Bind(
    wxEVT_BUTTON,
    &Client::onCancelClicked,
    this
  );
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

  auto *panel = new wxPanel(parent, -1, wxDefaultPosition);
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

  mSubscriptionsCtrl->SetFont(mFont);

  mSubscribe = new wxBitmapButton(
    panel,
    -1,
    *bin2cPlus18x18(),
    wxDefaultPosition,
    wxSize(OptionsHeight, OptionsHeight)
  );

  mFilter = new Widgets::TopicCtrl(panel, -1);
  mFilter->SetFont(mFont);

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
  mSubscribe->Bind(
    wxEVT_BUTTON,
    &Client::onSubscribeClicked,
    this
  );
  mFilter->Bind(
    wxEVT_KEY_DOWN,
    &Client::onSubscribeEnter,
    this
  );
}

void Client::setupPanelPreview(wxWindow *parent)
{
  auto *panel = new Widgets::Edit(parent, -1, OptionsHeight, mDarkMode);
  mPanes.at(Panes::Preview).panel = panel;
  panel->setReadOnly(true);
  panel->Bind(
    Events::EDIT_SAVE_SNIPPET,
    &Client::onPreviewSaveSnippet,
    this
  );
}

void Client::setupPanelPublish(wxWindow *parent)
{
  auto *panel = new Widgets::Edit(parent, -1, OptionsHeight, mDarkMode);
  mPanes.at(Panes::Publish).panel = panel;
  panel->Bind(
    Events::EDIT_PUBLISH,
    &Client::onPublishClicked,
    this
  );
  panel->Bind(
    Events::EDIT_SAVE_SNIPPET,
    &Client::onPublishSaveSnippet,
    this
  );
}

void Client::setupPanelSnippets(wxWindow *parent)
{
  auto *renderer = new wxDataViewIconTextRenderer(
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

  auto *panel = new wxPanel(parent, -1);
  mPanes.at(Panes::Snippets).panel = panel;

  mSnippetsCtrl = new wxDataViewListCtrl(
    panel,
    -1,
    wxDefaultPosition,
    wxDefaultSize,
    wxDV_NO_HEADER
  );
  mSnippetsCtrl->AppendColumn(mSnippetColumns.at(Snippets::Column::Name));
  mSnippetsCtrl->AssociateModel(mSnippetsModel.get());

  mSnippetsCtrl->EnableDropTarget(wxDataFormat(wxDF_TEXT));
  mSnippetsCtrl->EnableDragSource(wxDataFormat(wxDF_TEXT));

  mSnippetsCtrl->SetFont(mFont);

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
  mSnippetsCtrl->Bind(
    wxEVT_COMMAND_DATAVIEW_ITEM_ACTIVATED,
    &Client::onSnippetsActivated,
    this
  );
  mSnippetsCtrl->Bind(
    wxEVT_DATAVIEW_ITEM_BEGIN_DRAG,
    &Client::onSnippetsDrag,
    this
  );
  mSnippetsCtrl->Bind(
    wxEVT_DATAVIEW_ITEM_DROP,
    &Client::onSnippetsDrop,
    this
  );
  mSnippetsCtrl->Bind(
    wxEVT_DATAVIEW_ITEM_DROP_POSSIBLE,
    &Client::onSnippetsDropPossible,
    this
  );
}

// Setup }

// Snippets {

void Client::onSnippetsSelected(wxDataViewEvent &e)
{
  auto item = e.GetItem();
  if (!item.IsOk())
  {
    e.Skip();
    return;
  }

  auto *publish = dynamic_cast<Widgets::Edit*>(
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

void Client::onSnippetsActivated(wxDataViewEvent &e)
{
  const auto item = e.GetItem();
  if (!item.IsOk()) { return; }

  if (mSnippetsModel->IsContainer(item))
  {
    if (mSnippetsCtrl->IsExpanded(item))
    {
      mSnippetsCtrl->Collapse(item);
    }
    else
    {
      mSnippetsCtrl->Expand(item);
    }
  }
  else
  {
    const auto message = mSnippetsModel->getMessage(item);
    mClient->publish(
      message.topic,
      message.payload,
      message.qos,
      message.retained
    );
  }
}

void Client::onSnippetsDrag(wxDataViewEvent &e)
{
  auto item = e.GetItem();
  mSnippetsWasExpanded = mSnippetsCtrl->IsExpanded(item);

  const void *id = item.GetID();
  uintptr_t message = 0;
  std::memcpy(&message, &id, sizeof(uintptr_t));
  auto *o = new wxTextDataObject(std::to_string(message));

  e.SetDataFormat(o->GetFormat());
  e.SetDataSize(o->GetDataSize());
  e.SetDataObject(o);

  e.Skip(false);
}

void Client::onSnippetsDrop(wxDataViewEvent &e)
{
  const auto target = e.GetItem();

  wxTextDataObject object;
  object.SetData(e.GetDataFormat(), e.GetDataSize(), e.GetDataBuffer());
  const uintptr_t message = std::stoul(object.GetText().ToStdString());
  void *id = nullptr;
  std::memcpy(&id, &message, sizeof(uintptr_t));
  auto item = wxDataViewItem(id);

  wxDataViewItem moved;

  if (
    mSnippetsPossible.first
    && (
      // Moving in an empty dir.
      mSnippetsPossible.second == wxDataViewItem(0)
      // Moving in a full dir.
      || mSnippetsModel->GetParent(mSnippetsPossible.second) == target
    )
  ) {
    moved = mSnippetsModel->moveInside(item, target);
  }
  else
  {
    moved = mSnippetsModel->moveBefore(item, target);
  }

  mSnippetsPossible = {false, wxDataViewItem(nullptr)};

  if (!moved.IsOk())
  {
    return;
  }

  mSnippetsCtrl->Refresh();
  mSnippetsCtrl->EnsureVisible(moved);
  if (mSnippetsWasExpanded)
  {
    mSnippetsCtrl->Expand(moved);
  }
  mSnippetsCtrl->Select(moved);
}

void Client::onSnippetsDropPossible(wxDataViewEvent &e)
{
  // This event contains the first element of the target directory.
  // If this skips, the drop event target is the hovered item.
  // If this does not skip, the drop event target is this event's target.

  // If possible is parent of drop target, the target is a folder.

  mSnippetsPossible = {true, e.GetItem()};
  e.Skip(true);
}

// Snippets }

// Connection {

void Client::onConnectClicked(wxCommandEvent &/* event */)
{
  if (mClient->connected())
  {
    mLogger->info("Was connected");
    return;
  }

  allowCancel();
  mClient->setBrokerOptions(mBrokerOptions);
  mClient->connect();
}

void Client::onDisconnectClicked(wxCommandEvent &/* event */)
{
  if (!mClient->connected())
  {
    mLogger->info("Was not connected");
    return;
  }

  allowCancel();
  mClient->disconnect();
}

void Client::onCancelClicked(wxCommandEvent &/* event */)
{
  mClient->cancel();
}

void Client::allowConnect()
{
  mProfileSizer->Show(mConnect);
  mProfileSizer->Hide(mDisconnect);
  mProfileSizer->Hide(mCancel);
  mProfileSizer->Layout();
}

void Client::allowDisconnect()
{
  mProfileSizer->Hide(mConnect);
  mProfileSizer->Show(mDisconnect);
  mProfileSizer->Hide(mCancel);
  mProfileSizer->Layout();
}

void Client::allowCancel()
{
  mProfileSizer->Hide(mConnect);
  mProfileSizer->Hide(mDisconnect);
  mProfileSizer->Show(mCancel);
  mProfileSizer->Layout();
}

// Connection }

// Context {

void Client::onSubscriptionContext(wxDataViewEvent& event)
{
  if (!event.GetItem().IsOk()) { return; }

  auto item = event.GetItem();

  mSubscriptionsCtrl->Select(item);
  bool muted = mSubscriptionsModel->getMuted(item);

  wxMenu menu;
  menu.Append((unsigned)ContextIDs::SubscriptionsUnsubscribe, "Unsubscribe");
  menu.Append((unsigned)ContextIDs::SubscriptionsChangeColor, "Color change");
  menu.Append((unsigned)ContextIDs::SubscriptionsSolo, "Solo");
  menu.Append((unsigned)ContextIDs::SubscriptionsClear, "Clear");
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

  auto *copyTopic = new wxMenuItem(
    nullptr,
    (unsigned)ContextIDs::HistoryCopyTopic,
    "Copy Topic"
  );
  copyTopic->SetBitmap(wxArtProvider::GetBitmap(wxART_COPY));
  menu.Append(copyTopic);

  auto *copyPayload = new wxMenuItem(
    nullptr,
    (unsigned)ContextIDs::HistoryCopyPayload,
    "Copy Payload"
  );
  copyPayload->SetBitmap(wxArtProvider::GetBitmap(wxART_COPY));
  menu.Append(copyPayload);

  auto *edit = new wxMenuItem(
    nullptr,
    (unsigned)ContextIDs::HistoryEdit,
    "Edit"
  );
  edit->SetBitmap(wxArtProvider::GetBitmap(wxART_EDIT));
  menu.Append(edit);

  auto *resend = new wxMenuItem(
    nullptr,
    (unsigned)ContextIDs::HistoryResend,
    "Re-Send"
  );
  menu.Append(resend);

  auto *clearRetained = new wxMenuItem(
    nullptr,
    (unsigned)ContextIDs::HistoryRetainedClear,
    "Clear retained"
  );
  clearRetained->SetBitmap(wxArtProvider::GetBitmap(wxART_DELETE));
  menu.Append(clearRetained);

  auto *save = new wxMenuItem(
    nullptr,
    (unsigned)ContextIDs::HistorySaveSnippet,
    "Save Snippet"
  );
  save->SetBitmap(wxArtProvider::GetBitmap(wxART_FILE_SAVE));
  menu.Append(save);

  PopupMenu(&menu);
}

void Client::onSnippetsContext(wxDataViewEvent& e)
{
  wxMenu menu;

  if (!e.GetItem().IsOk())
  {
    mSnippetsCtrl->UnselectAll();
  }
  else
  {
    const auto item = e.GetItem();

    if (!mSnippetsModel->IsContainer(item))
    {
      auto *publish = new wxMenuItem(
        nullptr,
        (unsigned)ContextIDs::SnippetPublish,
        "Publish"
      );
      menu.Append(publish);
    }

    auto *overwrite = new wxMenuItem(
      nullptr,
      (unsigned)ContextIDs::SnippetOverwrite,
      "Overwrite"
    );
    overwrite->SetBitmap(wxArtProvider::GetBitmap(wxART_FILE_SAVE_AS));
    menu.Append(overwrite);

    auto *rename = new wxMenuItem(
      nullptr,
      (unsigned)ContextIDs::SnippetRename,
      "Rename"
    );
    rename->SetBitmap(wxArtProvider::GetBitmap(wxART_EDIT));
    menu.Append(rename);

    auto *del = new wxMenuItem(
      nullptr,
      (unsigned)ContextIDs::SnippetDelete,
      "Delete"
    );
    del->SetBitmap(wxArtProvider::GetBitmap(wxART_DELETE));
    menu.Append(del);
  }

  if (
    !e.GetItem().IsOk()
    || mSnippetsModel->IsContainer(e.GetItem())
  ) {
    auto *newFolder = new wxMenuItem(
      nullptr,
      (unsigned)ContextIDs::SnippetNewFolder,
      "New Folder"
    );
    newFolder->SetBitmap(wxArtProvider::GetBitmap(wxART_NEW_DIR));
    menu.Append(newFolder);

    auto *newSnippet = new wxMenuItem(
      nullptr,
      (unsigned)ContextIDs::SnippetNewSnippet,
      "New Snippet"
    );
    newSnippet->SetBitmap(wxArtProvider::GetBitmap(wxART_NORMAL_FILE));
    menu.Append(newSnippet);
  }

  PopupMenu(&menu);
}

void Client::onContextSelected(wxCommandEvent& event)
{
  switch (static_cast<ContextIDs>(event.GetId()))
  {
    case ContextIDs::HistoryRetainedClear: {
      onContextSelectedHistoryRetainedClear(event);
    } break;
    case ContextIDs::SubscriptionsUnsubscribe: {
      onContextSelectedSubscriptionsUnsubscribe(event);
    } break;
    case ContextIDs::SubscriptionsSolo: {
      onContextSelectedSubscriptionsSolo(event);
    } break;
    case ContextIDs::SubscriptionsClear: {
      onContextSelectedSubscriptionsClear(event);
    } break;
    case ContextIDs::SubscriptionsMute: {
      onContextSelectedSubscriptionsMute(event);
    } break;
    case ContextIDs::SubscriptionsUnmute: {
      onContextSelectedSubscriptionsUnmute(event);
    } break;
    case ContextIDs::SubscriptionsChangeColor: {
      onContextSelectedSubscriptionsChangeColor(event);
    } break;
    case ContextIDs::HistoryResend: {
      onContextSelectedHistoryResend(event);
    } break;
    case ContextIDs::HistoryEdit: {
      onContextSelectedHistoryEdit(event);
    } break;
    case ContextIDs::HistorySaveSnippet: {
      onContextSelectedHistorySaveSnippet(event);
    } break;
    case ContextIDs::HistoryCopyTopic: {
      onContextSelectedHistoryCopyTopic(event);
    } break;
    case ContextIDs::HistoryCopyPayload: {
      onContextSelectedHistoryCopyPayload(event);
    } break;
    case ContextIDs::SnippetNewFolder: {
      onContextSelectedSnippetNewFolder(event);
    } break;
    case ContextIDs::SnippetNewSnippet: {
      onContextSelectedSnippetNewSnippet(event);
    } break;
    case ContextIDs::SnippetDelete: {
      onContextSelectedSnippetDelete(event);
    } break;
    case ContextIDs::SnippetRename: {
      onContextSelectedSnippetRename(event);
    } break;
    case ContextIDs::SnippetPublish: {
      onContextSelectedSnippetPublish(event);
    } break;
    case ContextIDs::SnippetOverwrite: {
      onContextSelectedSnippetOverwrite(event);
    } break;
  }
  event.Skip();
}

void Client::onContextSelectedSubscriptionsUnsubscribe(wxCommandEvent &/* event */)
{
  const auto item = mSubscriptionsCtrl->GetSelection();
  if (!item.IsOk()) { return; }

  const auto selected = mHistoryCtrl->GetSelection();
  if (selected.IsOk())
  {
    mHistoryCtrl->Unselect(selected);
  }

  mSubscriptionsModel->unsubscribe(item);

  auto *preview = dynamic_cast<Widgets::Edit*>(
    mPanes.at(Panes::Preview).panel
  );
  preview->clear();
}

void Client::onContextSelectedSubscriptionsChangeColor(wxCommandEvent &/* event */)
{
  const auto item = mSubscriptionsCtrl->GetSelection();
  if (!item.IsOk()) { return; }

  const auto color = Helpers::colorFromNumber((size_t)std::abs(rand()));
  mSubscriptionsModel->setColor(item, color);

  mSubscriptionsCtrl->Refresh();
  mHistoryCtrl->Refresh();
}

void Client::onContextSelectedSubscriptionsMute(wxCommandEvent &/* event */)
{
  const auto item = mSubscriptionsCtrl->GetSelection();
  if (!item.IsOk()) { return; }
  mSubscriptionsModel->mute(item);
}

void Client::onContextSelectedSubscriptionsUnmute(wxCommandEvent &/* event */)
{
  const auto item = mSubscriptionsCtrl->GetSelection();
  if (!item.IsOk()) { return; }
  mSubscriptionsModel->unmute(item);
}

void Client::onContextSelectedSubscriptionsSolo(wxCommandEvent &/* event */)
{
  const auto item = mSubscriptionsCtrl->GetSelection();
  if (!item.IsOk()) { return; }
  mSubscriptionsModel->solo(item);
}

void Client::onContextSelectedSubscriptionsClear(wxCommandEvent &/* event */)
{
  const auto item = mSubscriptionsCtrl->GetSelection();
  if (!item.IsOk()) { return; }
  mSubscriptionsModel->clear(item);
}

void Client::onContextSelectedHistoryRetainedClear(wxCommandEvent &/* event */)
{
  const auto item = mHistoryCtrl->GetSelection();
  if (!item.IsOk()) { return; }

  const auto topic = mHistoryModel->getTopic(item);
  const auto qos = mHistoryModel->getQos(item);
  mClient->publish(topic, "", qos, true);
}

void Client::onContextSelectedHistoryResend(wxCommandEvent &/* event */)
{
  const auto item     = mHistoryCtrl->GetSelection();
  if (!item.IsOk()) { return; }

  const auto topic    = mHistoryModel->getTopic(item);
  const auto retained = mHistoryModel->getRetained(item);
  const auto qos      = mHistoryModel->getQos(item);
  const auto payload  = mHistoryModel->getPayload(item);
  mClient->publish(topic, payload, qos, retained);
}

void Client::onContextSelectedHistoryEdit(wxCommandEvent &/* event */)
{
  const auto item = mHistoryCtrl->GetSelection();
  if (!item.IsOk()) { return; }

  auto *publish = dynamic_cast<Widgets::Edit*>(
    mPanes.at(Panes::Publish).panel
  );

  publish->clear();
  publish->setTopic(mHistoryModel->getTopic(item));
  publish->setQos(mHistoryModel->getQos(item));
  publish->setPayload(mHistoryModel->getPayload(item));
  publish->setRetained(mHistoryModel->getRetained(item));
}

void Client::onContextSelectedHistorySaveSnippet(wxCommandEvent &/* event */)
{
  auto snippetItem = mSnippetsCtrl->GetSelection();
  if (!mSnippetsModel->IsContainer(snippetItem))
  {
    snippetItem = mSnippetsModel->GetParent(snippetItem);
  }

  const auto historyItem = mHistoryCtrl->GetSelection();
  const auto &message = mHistoryModel->getMessage(historyItem);
  const auto inserted = mSnippetsModel->createSnippet(snippetItem, message);

  if (!inserted.IsOk())
  {
    return;
  }

  auto *publish = dynamic_cast<Widgets::Edit*>(
    mPanes.at(Panes::Publish).panel
  );
  publish->setMessage(mSnippetsModel->getMessage(inserted));

  mSnippetsCtrl->Select(inserted);
  mSnippetsCtrl->EnsureVisible(inserted);

  auto *nameColumn = mSnippetColumns.at(Snippets::Column::Name);
  mSnippetExplicitEditRequest = true;
  mSnippetsCtrl->EditItem(inserted, nameColumn);
}

void Client::onContextSelectedHistoryCopyTopic(wxCommandEvent &/* event */)
{
  const auto historyItem = mHistoryCtrl->GetSelection();
  const auto &message = mHistoryModel->getMessage(historyItem);

  if (wxTheClipboard->Open())
  {
    auto *dataObject = new wxTextDataObject(message.topic);
    wxTheClipboard->SetData(dataObject);
    wxTheClipboard->Close();
  }
}

void Client::onContextSelectedHistoryCopyPayload(wxCommandEvent &/* event */)
{
  const auto historyItem = mHistoryCtrl->GetSelection();
  const auto &message = mHistoryModel->getMessage(historyItem);

  if (wxTheClipboard->Open())
  {
    auto *dataObject = new wxTextDataObject(message.payload);
    wxTheClipboard->SetData(dataObject);
    wxTheClipboard->Close();
  }
}

void Client::onContextSelectedSnippetRename(wxCommandEvent &/* event */)
{
  const auto item = mSnippetsCtrl->GetSelection();
  if (!item.IsOk()) { return; }

  mSnippetExplicitEditRequest = true;
  mSnippetsCtrl->EditItem(item, mSnippetColumns.at(Snippets::Column::Name));
}

void Client::onContextSelectedSnippetPublish(wxCommandEvent &/* event */)
{
  const auto item = mSnippetsCtrl->GetSelection();
  if (!item.IsOk()) { return; }

  auto *publish = dynamic_cast<Widgets::Edit*>(
    mPanes.at(Panes::Publish).panel
  );

  const auto &message = mSnippetsModel->getMessage(item);
  publish->setMessage(message);

  mClient->publish(
    message.topic,
    message.payload,
    message.qos,
    message.retained
  );
}

void Client::onContextSelectedSnippetOverwrite(wxCommandEvent &/* event */)
{
  const auto item = mSnippetsCtrl->GetSelection();
  if (!item.IsOk()) { return; }

  auto *publish = dynamic_cast<Widgets::Edit*>(
    mPanes.at(Panes::Publish).panel
  );
  const auto message = publish->getMessage();

  mSnippetsModel->replace(item, message);
}

void Client::onContextSelectedSnippetDelete(wxCommandEvent &/* event */)
{
  const auto item = mSnippetsCtrl->GetSelection();
  if (!item.IsOk()) { return; }

  const bool done = mSnippetsModel->remove(item);
  if (!done)
  {
    return;
  }

  const auto root = mSnippetsModel->getRootItem();
  mSnippetsCtrl->Select(root);
}

void Client::onContextSelectedSnippetNewSnippet(wxCommandEvent &/* event */)
{
  auto item = mSnippetsCtrl->GetSelection();
  if (!mSnippetsModel->IsContainer(item))
  {
    item = mSnippetsModel->GetParent(item);
  }

  const auto inserted = mSnippetsModel->createSnippet(item, {});
  if (!inserted.IsOk())
  {
    return;
  }

  auto *publish = dynamic_cast<Widgets::Edit*>(
    mPanes.at(Panes::Publish).panel
  );
  publish->setMessage(mSnippetsModel->getMessage(inserted));

  mSnippetsCtrl->Select(inserted);
  mSnippetsCtrl->EnsureVisible(inserted);

  auto *nameColumn = mSnippetColumns.at(Snippets::Column::Name);
  mSnippetExplicitEditRequest = true;
  mSnippetsCtrl->EditItem(inserted, nameColumn);
}

void Client::onContextSelectedSnippetNewFolder(wxCommandEvent &/* event */)
{
  auto item = mSnippetsCtrl->GetSelection();
  if (!mSnippetsModel->IsContainer(item))
  {
    item = mSnippetsModel->GetParent(item);
  }

  const auto inserted = mSnippetsModel->createFolder(item);
  if (!inserted.IsOk())
  {
    return;
  }

  mSnippetsCtrl->Select(inserted);
  mSnippetsCtrl->EnsureVisible(inserted);

  auto *nameColumn = mSnippetColumns.at(Snippets::Column::Name);
  mSnippetExplicitEditRequest = true;
  mSnippetsCtrl->EditItem(inserted, nameColumn);
}

// Context }

// MQTT::Client::Observer {

void Client::onConnected()
{
  auto *e = new Events::Connection(Events::CONNECTED);
  wxQueueEvent(this, e);
}

void Client::onDisconnected()
{
  auto *e = new Events::Connection(Events::DISCONNECTED);
  wxQueueEvent(this, e);
}

void Client::onConnectionLost()
{
  auto *e = new Events::Connection(Events::LOST);
  wxQueueEvent(this, e);
}

void Client::onConnectionFailure()
{
  auto *e = new Events::Connection(Events::FAILURE);
  wxQueueEvent(this, e);
}

void Client::onConnectedSync(Events::Connection &/* event */)
{
  mLogger->info("Connected");
  allowDisconnect();
}

void Client::onDisconnectedSync(Events::Connection &/* event */)
{
  mLogger->info("Disconnected");
  allowConnect();
}

void Client::onConnectionLostSync(Events::Connection &/* event */)
{
  mLogger->info("Connection lost, reconnecting...");
  allowCancel();
}

void Client::onConnectionFailureSync(Events::Connection &/* event */)
{
  mLogger->info("Unable to connect to server");
  allowConnect();
}

// MQTT::Client::Observer }

// Models::History::Observer {

void Client::onMessage(wxDataViewItem item)
{
  if (!mAutoScroll->GetValue())
  {
    return;
  }

  mHistoryCtrl->Select(item);
  mHistoryCtrl->EnsureVisible(item);

  auto *preview = dynamic_cast<Widgets::Edit*>(mPanes.at(Panes::Preview).panel);
  preview->setMessage(mHistoryModel->getMessage(item));
}

// Models::History::Observer }

// Subscriptions {

void Client::onSubscribeClicked(wxCommandEvent &/* event */)
{
  const auto value = mFilter->GetValue();
  mFilter->SetValue({});
  const auto wxs = value.empty() ? "#" : value;
  const auto utf8 = wxs.ToUTF8();
  const std::string topic(utf8.data(), utf8.length());

  mSubscriptionsModel->subscribe(topic, MQTT::QoS::ExactlyOnce);
}

void Client::onSubscribeEnter(wxKeyEvent &event)
{
  if (event.GetKeyCode() != WXK_RETURN)
  {
    event.Skip();
    return;
  }

  const auto value = mFilter->GetValue();
  mFilter->SetValue({});
  const auto wxs = value.empty() ? "#" : value;
  const auto utf8 = wxs.ToUTF8();
  const std::string topic(utf8.data(), utf8.length());

  mSubscriptionsModel->subscribe(topic, MQTT::QoS::ExactlyOnce);
  event.Skip();
}

void Client::onSubscriptionSelected(wxDataViewEvent &/* event */)
{
  const auto item = mSubscriptionsCtrl->GetSelection();
  if (!item.IsOk()) { return; }
  const auto utf8 = mSubscriptionsModel->getFilter(item);
  const auto wxs = wxString::FromUTF8(utf8.data(), utf8.length());
  mFilter->SetValue(wxs);
}

// Subscriptions }

// Layouts {

void Client::onLayoutSelected(Events::Layout &event)
{
  mAuiMan.LoadPerspective(event.getPerspective(), false);

  for (auto &pane : mPanes)
  {
    auto &info = mAuiMan.GetPane(pane.second.panel);

    const bool isEditor =
      pane.first == Panes::Preview
      || pane.first == Panes::Publish;

    const auto minSize = isEditor
      ? wxSize(PaneMinWidth, EditorMinHeight)
      : wxSize(PaneMinWidth, PaneMinHeight);

    info.MinSize(minSize);
    info.Caption(pane.second.name);
    info.CloseButton(false);
    info.Icon(*pane.second.icon18x14);
    info.PaneBorder(false);
  }

  mAuiMan.Update();
}

void Client::onLayoutResized(Events::Layout &/* event */)
{
  mLayouts->Hide();
  mLayouts->Show();
  mProfileSizer->Layout();
}

// Layouts }

// Publish {

void Client::onPublishClicked(wxCommandEvent &/* event */)
{
  auto *publish = dynamic_cast<Widgets::Edit*>(mPanes.at(Panes::Publish).panel);

  if (!mClient->connected()) { return; }
  auto topic = publish->getTopic();
  if (topic.empty()) { return; }

  auto payload  = publish->getPayload();
  auto qos      = publish->getQos();
  bool retained = publish->getRetained();

  mClient->publish(topic, payload, qos, retained);
}

void Client::onPublishSaveSnippet(Events::Edit &/* event */)
{
  mLogger->info("Saving current message");
  auto snippetItem = mSnippetsCtrl->GetSelection();
  if (!mSnippetsModel->IsContainer(snippetItem))
  {
    mLogger->info("Selecting parent");
    snippetItem = mSnippetsModel->GetParent(snippetItem);
  }

  auto *publish = dynamic_cast<Widgets::Edit*>(
    mPanes.at(Panes::Publish).panel
  );

  const auto &message = publish->getMessage();
  mLogger->info("Storing {}", message.topic);
  auto inserted = mSnippetsModel->createSnippet(snippetItem, message);
  if (inserted.IsOk())
  {
    auto *publish = dynamic_cast<Widgets::Edit*>(
      mPanes.at(Panes::Publish).panel
    );
    publish->setMessage(mSnippetsModel->getMessage(inserted));

    mSnippetsCtrl->Select(inserted);
    mSnippetsCtrl->EnsureVisible(inserted);
    auto *nameColumn = mSnippetColumns.at(Snippets::Column::Name);
    mSnippetExplicitEditRequest = true;
    mSnippetsCtrl->EditItem(inserted, nameColumn);
  }
}

// Publish }

// History {

void Client::onHistorySelected(wxDataViewEvent &event)
{
  if (!event.GetItem().IsOk()) { return; }
  const auto item = event.GetItem();

  auto *preview = dynamic_cast<Widgets::Edit*>(mPanes.at(Panes::Preview).panel);
  preview->setMessage(mHistoryModel->getMessage(item));
}

void Client::onHistoryClearClicked(wxCommandEvent &/* event */)
{
  mHistoryModel->clear();

  auto *preview = dynamic_cast<Widgets::Edit*>(
    mPanes.at(Panes::Preview).panel
  );
  preview->clear();
}

void Client::onHistoryDoubleClicked(wxDataViewEvent &event)
{
  if (!event.GetItem().IsOk()) { return; }
  const auto item = event.GetItem();

  auto *publish = dynamic_cast<Widgets::Edit*>(mPanes.at(Panes::Publish).panel);
  publish->setMessage(mHistoryModel->getMessage(item));
}

void Client::onHistorySearchKey(wxKeyEvent &event)
{
  const auto filter = mHistorySearchFilter->GetValue().ToStdString();
  long from = 0;
  long to = 0;
  mHistorySearchFilter->GetSelection(&from, &to);

  const auto isBackspace = event.GetKeyCode() == WXK_BACK;
  const auto isEnter     = event.GetKeyCode() == WXK_RETURN;

  const auto isAllSelected = (from == 0 && to == (long)filter.size());
  const auto willBeEmpty   = filter.empty() || filter.size() == 1 || isAllSelected;

  if (isBackspace && willBeEmpty)
  {
    mHistoryModel->setFilter({});
  }
  else if (isEnter)
  {
    mHistoryModel->setFilter(filter);
  }

  event.Skip();
}

void Client::onHistorySearchButton(wxCommandEvent &/* event */)
{
  const auto filter = mHistorySearchFilter->GetValue().ToStdString();
  mHistoryModel->setFilter(filter);
}

// History }

// Preview {

void Client::onPreviewSaveSnippet(Events::Edit &/* event */)
{
  mLogger->info("Saving current message");
  auto snippetItem = mSnippetsCtrl->GetSelection();
  if (!mSnippetsModel->IsContainer(snippetItem))
  {
    mLogger->info("Selecting parent");
    snippetItem = mSnippetsModel->GetParent(snippetItem);
  }

  auto *preview = dynamic_cast<Widgets::Edit*>(
    mPanes.at(Panes::Preview).panel
  );

  const auto &message = preview->getMessage();
  mLogger->info("Storing {}", message.topic);
  auto inserted = mSnippetsModel->createSnippet(snippetItem, message);
  if (inserted.IsOk())
  {
    mSnippetsCtrl->Select(inserted);
    mSnippetsCtrl->EnsureVisible(inserted);
    auto *nameColumn = mSnippetColumns.at(Snippets::Column::Name);
    mSnippetExplicitEditRequest = true;
    mSnippetsCtrl->EditItem(inserted, nameColumn);
  }
}

// Preview }

void Client::onClose(wxCloseEvent &/* event */)
{
  if (mClient->connected())
  {
    auto *preview = dynamic_cast<Widgets::Edit*>(
      mPanes.at(Panes::Preview).panel
    );
    preview->setPayload("Closing...");
    mClient->disconnect();
  }
  Destroy();
}

//  }
