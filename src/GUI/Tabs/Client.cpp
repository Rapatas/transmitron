#include "Client.hpp"

#include <memory>

#include <nlohmann/json.hpp>
#include <wx/artprov.h>
#include <wx/clipbrd.h>

#include "Common/Helpers.hpp"
#include "Common/Log.hpp"
#include "GUI/Events/Layout.hpp"
#include "GUI/Events/Recording.hpp"
#include "GUI/Resources/history/history-18x14.hpp"
#include "GUI/Resources/messages/messages-18x14.hpp"
#include "GUI/Resources/preview/preview-18x14.hpp"
#include "GUI/Resources/send/send-18x14.hpp"
#include "GUI/Resources/subscription/subscription-18x14.hpp"
#include "GUI/Widgets/Edit.hpp"
#include "MQTT/Message.hpp"

using namespace Rapatas::Transmitron;
using namespace GUI::Tabs;
using namespace GUI::Models;
using namespace GUI;
using namespace Common;

constexpr size_t FontSize = 9;
static constexpr size_t PaneMinWidth = 100;
static constexpr size_t PaneMinHeight = 100;
static constexpr size_t PaneBestWidth = 412;
static constexpr size_t MessagesBestWidth = 200;

Client::Client(
  wxWindow *parent,
  const MQTT::BrokerOptions &brokerOptions,
  Types::ClientOptions clientOptions,
  const wxObjectDataPtr<Models::Messages> &messages,
  const wxObjectDataPtr<Models::KnownTopics> &topicsSubscribed,
  const wxObjectDataPtr<Models::KnownTopics> &topicsPublished,
  const wxObjectDataPtr<Models::Layouts> &layoutsModel,
  const wxString &name,
  const ArtProvider &artProvider,
  bool darkMode,
  int optionsHeight
) :
  wxPanel(
    parent,
    -1,
    wxDefaultPosition,
    wxDefaultSize,
    wxTAB_TRAVERSAL,
    name //
  ),
  mName(name),
  mClientOptions(std::move(clientOptions)),
  mFont(wxFontInfo(FontSize).FaceName("Consolas")),
  mArtProvider(artProvider),
  mDarkMode(darkMode),
  mOptionsHeight(optionsHeight),
  mTopicsSubscribed(topicsSubscribed),
  mTopicsPublished(topicsPublished),
  mLayoutsModel(layoutsModel),
  mRandomGenerator(mRandomDev()),
  mRandomColor(0, 255 * 255 * 255), // NOLINT
  mMessagesModel(messages),
  mMessageColumns({}),
  mClient(std::make_shared<MQTT::Client>()),
  mMqttObserverId(mClient->attachObserver(this)) //
{
  Bind(Events::CONNECTION_CONNECTED, &Client::onConnectedSync, this);
  Bind(Events::CONNECTION_DISCONNECTED, &Client::onDisconnectedSync, this);
  Bind(Events::CONNECTION_LOST, &Client::onConnectionLostSync, this);
  Bind(Events::CONNECTION_FAILURE, &Client::onConnectionFailureSync, this);

  mClient->setBrokerOptions(brokerOptions);
  setupPanels();
  mClient->connect();
}

Client::Client(
  wxWindow *parent,
  const wxObjectDataPtr<Models::History> &historyModel,
  const wxObjectDataPtr<Models::Subscriptions> &subscriptionsModel,
  const wxObjectDataPtr<Models::Layouts> &layoutsModel,
  const wxString &name,
  const ArtProvider &artProvider,
  bool darkMode,
  int optionsHeight
) :
  wxPanel(
    parent,
    -1,
    wxDefaultPosition,
    wxDefaultSize,
    wxTAB_TRAVERSAL,
    name //
  ),
  mName(name),
  mFont(wxFontInfo(FontSize).FaceName("Consolas")),
  mArtProvider(artProvider),
  mDarkMode(darkMode),
  mOptionsHeight(optionsHeight),
  mLayoutsModel(layoutsModel),
  mRandomGenerator(mRandomDev()),
  mRandomColor(0, 255 * 255 * 255), // NOLINT
  mHistoryModel(historyModel),
  mSubscriptionsModel(subscriptionsModel),
  mMessageColumns({}) //
{
  setupPanels();
}

void Client::setupPanels() {
  Bind(wxEVT_CLOSE_WINDOW, &Client::onClose, this);
  Bind(wxEVT_COMMAND_MENU_SELECTED, &Client::onContextSelected, this);

  mLogger = Common::Log::create("GUI::Client");

  mPanes = {
    {
      Panes::History,
      {
        "History",
        {},
        nullptr,
        mArtProvider.bitmap(Icon::History),
        bin2cHistory18x14(),
        nullptr,
      },
    },
    {
      Panes::Preview,
      {
        "Preview",
        {},
        nullptr,
        mArtProvider.bitmap(Icon::FileFull),
        bin2cPreview18x14(),
        nullptr,
      },
    },
    {
      Panes::Subscriptions,
      {
        "Subscriptions",
        {},
        nullptr,
        mArtProvider.bitmap(Icon::Subscriptions),
        bin2cSubscription18x14(),
        nullptr,
      },
    },
  };

  mPanes.at(Panes::History).info.Center();
  mPanes.at(Panes::Subscriptions).info.Top();
  mPanes.at(Panes::Preview).info.Right();

  mPanes.at(Panes::History).info.Layer(0);
  mPanes.at(Panes::Subscriptions).info.Layer(1);
  mPanes.at(Panes::Preview).info.Layer(2);

  mPanes.at(Panes::Subscriptions).info.MinSize(PaneBestWidth, PaneMinHeight);
  mPanes.at(Panes::History).info.MinSize(PaneBestWidth, PaneMinHeight);
  mPanes.at(Panes::Preview).info.MinSize(PaneBestWidth, -1);

  if (mClient != nullptr) {
    mPanes.insert({
      Panes::Messages,
      {
        "Messages",
        {},
        nullptr,
        mArtProvider.bitmap(Icon::Archive),
        bin2cMessages18x14(),
        nullptr,
      },
    });

    mPanes.insert({
      Panes::Publish,
      {
        "Publish",
        {},
        nullptr,
        mArtProvider.bitmap(Icon::Publish),
        bin2cSend18x14(),
        nullptr,
      },
    });

    mPanes.at(Panes::Messages).info.Left();
    mPanes.at(Panes::Publish).info.Right();

    mPanes.at(Panes::Messages).info.Layer(2);
    mPanes.at(Panes::Publish).info.Layer(2);

    mPanes.at(Panes::Messages).info.MinSize(MessagesBestWidth, -1);
    mPanes.at(Panes::Publish).info.MinSize(PaneBestWidth, -1);
  }

  for (auto &pane : mPanes) {
    const auto fixed = pane.first == Panes::History;
    pane.second.info.Caption(pane.second.name);
    pane.second.info.CloseButton(false);
    pane.second.info.Floatable(!fixed);
    pane.second.info.Icon(*pane.second.icon18x14);
    pane.second.info.Movable(!fixed);
    pane.second.info.Name(pane.second.name);
    pane.second.info.PaneBorder(false);
  }

  auto *managed = new wxPanel(this);

  if (mClient != nullptr) {
    setupPanelMessages(managed);
    setupPanelPublish(managed);
  }
  setupPanelSubscriptions(managed);
  setupPanelPreview(managed);
  setupPanelHistory(managed);
  setupPanelConnect(this);

  auto *sizer = new wxBoxSizer(wxVERTICAL);
  sizer->Add(mProfileBar, 0, wxEXPAND);
  sizer->Add(managed, 1, wxEXPAND);
  SetSizer(sizer);

  mAuiMan.SetManagedWindow(managed);
  for (const auto &pane : mPanes) {
    mAuiMan.AddPane(pane.second.panel, pane.second.info);
  }

  mAuiMan.Update();

  for (auto &[window, pane] : mPanes) {
    auto &info = mAuiMan.GetPane(pane.panel);
    info.MinSize(PaneMinWidth, PaneMinHeight);
  }

  mAuiMan.Update();

  const auto layout = mClientOptions.getLayout();
  mLayouts->setSelectedLayout(layout);
}

Client::~Client() {
  if (mClient != nullptr) {
    mClient->disconnect();
    mClient->detachObserver(mMqttObserverId);
  }
  mAuiMan.UnInit();
}

void Client::focus() const { mFilter->SetFocus(); }

// Private {

// Setup {

void Client::setupPanelHistory(wxWindow *parent) {
  auto *const icon = new wxDataViewColumn(
    L"icon",
    new wxDataViewBitmapRenderer(),
    static_cast<unsigned>(Models::History::Column::Icon),
    wxCOL_WIDTH_AUTOSIZE,
    wxALIGN_LEFT
  );
  auto *const topic = new wxDataViewColumn(
    L"topic",
    new wxDataViewIconTextRenderer(),
    static_cast<unsigned>(Models::History::Column::Topic),
    wxCOL_WIDTH_AUTOSIZE,
    wxALIGN_LEFT
  );
  auto *const qos = new wxDataViewColumn(
    L"qos",
    new wxDataViewBitmapRenderer(),
    static_cast<unsigned>(Models::History::Column::Qos),
    wxCOL_WIDTH_AUTOSIZE
  );

  auto *panel = new wxPanel(parent);
  mPanes.at(Panes::History).panel = panel;

  mHistoryCtrl = new wxDataViewCtrl(
    panel,
    -1,
    wxDefaultPosition,
    wxDefaultSize,
    wxDV_NO_HEADER | wxDV_ROW_LINES
  );

  if (mClient != nullptr) {
    mHistoryModel = new Models::History(mSubscriptionsModel);
  }
  mHistoryModel->attachObserver(this);
  mHistoryCtrl->AssociateModel(mHistoryModel.get());

  mHistoryCtrl->SetFont(mFont);

  mHistoryCtrl->AppendColumn(icon);
  mHistoryCtrl->AppendColumn(qos);
  mHistoryCtrl->AppendColumn(topic);

  mHistorySearchFilter = new Widgets::TopicCtrl(panel, -1);
  mHistorySearchFilter->SetHint("Filter...");
  mHistorySearchFilter->Bind(wxEVT_KEY_DOWN, &Client::onHistorySearchKey, this);

  mHistorySearchButton = new wxButton(
    panel,
    -1,
    "",
    wxDefaultPosition,
    wxSize(mOptionsHeight, mOptionsHeight)
  );
  mHistorySearchButton->SetBitmap(mArtProvider.bitmap(Icon::Search));
  mHistorySearchButton->Bind(
    wxEVT_BUTTON,
    &Client::onHistorySearchButton,
    this //
  );

  mAutoScroll = new wxCheckBox(panel, -1, "auto-scroll");
  mAutoScroll->SetValue(true);

  mHistoryClear = new wxButton(
    panel,
    -1,
    "",
    wxDefaultPosition,
    wxSize(mOptionsHeight, mOptionsHeight)
  );
  mHistoryClear->SetToolTip("Clear history");
  mHistoryClear->SetBitmap(mArtProvider.bitmap(Icon::Delete));

  mHistoryRecord = new wxButton(
    panel,
    -1,
    "",
    wxDefaultPosition,
    wxSize(mOptionsHeight - 1, mOptionsHeight)
  );
  mHistoryRecord->SetToolTip("Store history recording");
  mHistoryRecord->SetBitmap(mArtProvider.bitmap(Icon::Save));

  auto *topSizer = new wxBoxSizer(wxOrientation::wxHORIZONTAL);
  topSizer->Add(mHistorySearchFilter, 1, wxEXPAND);
  topSizer->Add(mHistorySearchButton, 0, wxEXPAND);
  auto *hsizer = new wxBoxSizer(wxOrientation::wxHORIZONTAL);
  hsizer->SetMinSize(0, mOptionsHeight);
  hsizer->Add(mAutoScroll, 0, wxEXPAND);
  hsizer->AddStretchSpacer(1);
  hsizer->Add(mHistoryRecord, 0, wxEXPAND);
  hsizer->Add(mHistoryClear, 0, wxEXPAND);
  auto *vsizer = new wxBoxSizer(wxOrientation::wxVERTICAL);
  vsizer->Add(topSizer, 0, wxEXPAND);
  vsizer->Add(mHistoryCtrl, 1, wxEXPAND);
  vsizer->Add(hsizer, 0, wxEXPAND);
  panel->SetSizer(vsizer);

  mHistoryCtrl->Bind(
    wxEVT_DATAVIEW_SELECTION_CHANGED,
    &Client::onHistorySelected,
    this //
  );
  mHistoryCtrl->Bind(
    wxEVT_DATAVIEW_ITEM_ACTIVATED,
    &Client::onHistoryDoubleClicked,
    this
  );
  mHistoryCtrl->Bind(
    wxEVT_DATAVIEW_ITEM_CONTEXT_MENU,
    &Client::onHistoryContext,
    this //
  );
  mHistoryClear->Bind(wxEVT_BUTTON, &Client::onHistoryClearClicked, this);
  mHistoryRecord->Bind(wxEVT_BUTTON, &Client::onHistoryRecordClicked, this);

  if (mClient == nullptr) {
    vsizer->Hide(hsizer);
    vsizer->Layout();
  }
}

void Client::setupPanelConnect(wxWindow *parent) {
  mProfileBar = new wxPanel(parent);

  mIndicator = new wxStaticBitmap(
    mProfileBar,
    wxID_ANY,
    mArtProvider.bitmap(Icon::Connecting)
  );
  mIndicator->SetMinSize(wxSize(mOptionsHeight, mOptionsHeight));

  mConnect = new wxButton(
    mProfileBar,
    -1,
    "Connect",
    wxDefaultPosition,
    wxSize(-1, mOptionsHeight)
  );
  mConnect->SetToolTip("Attempt to reconnect");
  mDisconnect = new wxButton(
    mProfileBar,
    -1,
    "Disconnect",
    wxDefaultPosition,
    wxSize(-1, mOptionsHeight)
  );
  mCancel = new wxButton(
    mProfileBar,
    -1,
    "Cancel",
    wxDefaultPosition,
    wxSize(-1, mOptionsHeight)
  );
  mCancel->SetToolTip("Stop connection attempt");

  mLayouts = new Widgets::Layouts(
    mProfileBar,
    -1,
    mLayoutsModel,
    &mAuiMan,
    mArtProvider,
    mOptionsHeight
  );
  mLayouts->Bind(Events::LAYOUT_SELECTED, &Client::onLayoutSelected, this);
  mLayouts->Bind(Events::LAYOUT_RESIZED, &Client::onLayoutResized, this);

  auto callback = [this](Panes pane, wxCommandEvent &event) {
    (void)event;
    auto widget = mPanes.at(pane);

    auto currentInfo = mAuiMan.GetPane(widget.panel);

    // Pane missing.
    if (!currentInfo.IsOk()) {
      widget.panel->Show(true);
      mAuiMan.AddPane(widget.panel, mPanes.at(pane).info);
      widget.toggle->SetBackgroundColour(wxNullColour);
      mAuiMan.Update();
      return;
    }

    mPanes.at(pane).info = currentInfo;

    // Pane is visible.
    if (currentInfo.IsShown()) {
      mAuiMan.DetachPane(widget.panel);
      widget.panel->Show(false);
      constexpr uint8_t ColorChannelHalf = 255 / 2;
      widget.toggle->SetBackgroundColour(
        wxColor(ColorChannelHalf, ColorChannelHalf, ColorChannelHalf)
      );
      mAuiMan.Update();
      return;
    }

    // Pane is known but hidden.
    widget.info.Show(true);
    widget.toggle->SetBackgroundColour(wxNullColour);
    mAuiMan.DetachPane(widget.panel);
    mAuiMan.AddPane(widget.panel, widget.info);
    mAuiMan.Update();
  };

  mProfileSizer = new wxBoxSizer(wxOrientation::wxHORIZONTAL);
  mProfileSizer->SetMinSize(0, mOptionsHeight);
  mProfileSizer->Add(mIndicator, 0, wxEXPAND);
  mProfileSizer->Add(mConnect, 0, wxEXPAND);
  mProfileSizer->Add(mDisconnect, 0, wxEXPAND);
  mProfileSizer->Add(mCancel, 0, wxEXPAND);
  allowCancel();

  auto *panelSelector = new wxBoxSizer(wxHORIZONTAL);
  for (auto &pane : mPanes) {
    if (pane.first == Panes::History) { continue; }

    const auto &bitmap = mPanes.at(pane.first).icon18x18;
    auto *button = new wxButton(
      mProfileBar,
      -1,
      "",
      wxDefaultPosition,
      wxSize(mOptionsHeight, mOptionsHeight)
    );
    button->SetToolTip(pane.second.name);
    button->SetBitmap(bitmap);
    button->Bind(wxEVT_BUTTON, [callback, pane](wxCommandEvent &event) {
      callback(pane.first, event);
    });
    panelSelector->Add(button, 0, wxEXPAND);

    pane.second.toggle = button;
  }

  mProfileSizer->AddStretchSpacer(1);
  mProfileSizer->Add(panelSelector, 0, wxEXPAND);
  mProfileSizer->Add(mLayouts, 0, wxEXPAND);

  mProfileBar->SetSizer(mProfileSizer);

  mConnect->Bind(wxEVT_BUTTON, &Client::onConnectClicked, this);
  mDisconnect->Bind(wxEVT_BUTTON, &Client::onDisconnectClicked, this);
  mCancel->Bind(wxEVT_BUTTON, &Client::onCancelClicked, this);
}

void Client::setupPanelSubscriptions(wxWindow *parent) {
  auto *panel = new wxPanel(parent);
  mPanes.at(Panes::Subscriptions).panel = panel;

  mFilter = new Widgets::TopicCtrl(panel, -1);
  mFilter->addKnownTopics(mTopicsSubscribed);
  mFilter->SetFont(mFont);

  mSubscribe = new wxButton(
    panel,
    -1,
    "",
    wxDefaultPosition,
    wxSize(mOptionsHeight, mOptionsHeight)
  );
  mSubscribe->SetBitmap(mArtProvider.bitmap(Icon::Subscribe));
  mSubscribe->SetToolTip("Subscribe");

  auto *const icon = new wxDataViewColumn(
    "icon",
    new wxDataViewBitmapRenderer(),
    static_cast<unsigned>(Models::Subscriptions::Column::Icon),
    wxCOL_WIDTH_AUTOSIZE
  );
  auto *const topic = new wxDataViewColumn(
    "topic",
    new wxDataViewTextRenderer(),
    static_cast<unsigned>(Models::Subscriptions::Column::Topic),
    wxCOL_WIDTH_AUTOSIZE,
    wxALIGN_LEFT
  );
  auto *const qos = new wxDataViewColumn(
    "qos",
    new wxDataViewBitmapRenderer(),
    static_cast<unsigned>(Models::Subscriptions::Column::Qos),
    wxCOL_WIDTH_AUTOSIZE
  );

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

  if (mClient != nullptr) {
    mSubscriptionsModel = new Models::Subscriptions(mClient);
  }
  mSubscriptionsCtrl->AssociateModel(mSubscriptionsModel.get());

  mSubscriptionsCtrl->SetFont(mFont);

  auto *vsizer = new wxBoxSizer(wxOrientation::wxVERTICAL);
  auto *hsizer = new wxBoxSizer(wxOrientation::wxHORIZONTAL);
  hsizer->SetMinSize(0, mOptionsHeight);
  hsizer->Add(mFilter, 1, wxEXPAND);
  hsizer->Add(mSubscribe, 0, wxEXPAND);
  vsizer->Add(hsizer, 0, wxEXPAND);
  vsizer->Add(mSubscriptionsCtrl, 1, wxEXPAND);
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
  mFilter->Bind(wxEVT_KEY_UP, &Client::onSubscribeEnter, this);
}

void Client::setupPanelPreview(wxWindow *parent) {
  auto *panel = new Widgets::Edit(
    parent,
    -1,
    mArtProvider,
    mOptionsHeight,
    mDarkMode
  );
  mPanes.at(Panes::Preview).panel = panel;
  panel->setReadOnly(true);
  panel->Bind(Events::EDIT_SAVE_MESSAGE, &Client::onPreviewSaveMessage, this);
}

void Client::setupPanelPublish(wxWindow *parent) {
  auto *panel = new Widgets::Edit(
    parent,
    -1,
    mArtProvider,
    mOptionsHeight,
    mDarkMode
  );
  panel->addKnownTopics(mTopicsPublished);
  mPanes.at(Panes::Publish).panel = panel;
  panel->Bind(Events::EDIT_PUBLISH, &Client::onPublishClicked, this);
  panel->Bind(Events::EDIT_SAVE_MESSAGE, &Client::onPublishSaveMessage, this);
}

void Client::setupPanelMessages(wxWindow *parent) {
  auto *renderer = new wxDataViewIconTextRenderer(
    wxDataViewIconTextRenderer::GetDefaultType(),
    wxDATAVIEW_CELL_EDITABLE
  );

  mMessageColumns.at(Messages::Column::Name) = new wxDataViewColumn(
    L"name",
    renderer,
    static_cast<unsigned>(Models::Messages::Column::Name),
    wxCOL_WIDTH_AUTOSIZE,
    wxALIGN_LEFT
  );

  auto *panel = new wxPanel(parent);
  mPanes.at(Panes::Messages).panel = panel;

  mMessagesCtrl = new wxDataViewListCtrl(
    panel,
    -1,
    wxDefaultPosition,
    wxDefaultSize,
    wxDV_NO_HEADER
  );
  mMessagesCtrl->AppendColumn(mMessageColumns.at(Messages::Column::Name));
  mMessagesCtrl->AssociateModel(mMessagesModel.get());

  // Windows only works with unicode.
  mMessagesCtrl->EnableDropTarget(wxDF_UNICODETEXT);
  mMessagesCtrl->EnableDragSource(wxDF_UNICODETEXT);

  mMessagesCtrl->SetFont(mFont);

  auto *vsizer = new wxBoxSizer(wxOrientation::wxVERTICAL);
  vsizer->Add(mMessagesCtrl, 1, wxEXPAND);
  panel->SetSizer(vsizer);
  mMessagesCtrl->Bind(
    wxEVT_DATAVIEW_ITEM_CONTEXT_MENU,
    &Client::onMessagesContext,
    this //
  );
  mMessagesCtrl->Bind(
    wxEVT_DATAVIEW_ITEM_START_EDITING,
    &Client::onMessagesEdit,
    this //
  );
  mMessagesCtrl->Bind(
    wxEVT_DATAVIEW_ITEM_VALUE_CHANGED,
    &Client::onMessagesChanged,
    this //
  );
  mMessagesCtrl->Bind(
    wxEVT_DATAVIEW_SELECTION_CHANGED,
    &Client::onMessagesSelected,
    this //
  );
  mMessagesCtrl->Bind(
    wxEVT_DATAVIEW_ITEM_BEGIN_DRAG,
    &Client::onMessagesDrag,
    this //
  );
  mMessagesCtrl->Bind(
    wxEVT_COMMAND_DATAVIEW_ITEM_ACTIVATED,
    &Client::onMessagesActivated,
    this
  );
  mMessagesCtrl->Bind(
    wxEVT_DATAVIEW_ITEM_DROP,
    &Client::onMessagesDrop,
    this //
  );
  mMessagesCtrl->Bind(
    wxEVT_DATAVIEW_ITEM_DROP_POSSIBLE,
    &Client::onMessagesDropPossible,
    this
  );
}

// Setup }

// Messages {

void Client::onMessagesSelected(wxDataViewEvent &event) {
  auto item = event.GetItem();
  if (!item.IsOk()) {
    event.Skip();
    return;
  }

  auto *publish = mPanes.at(Panes::Publish).panel;
  auto *edit = dynamic_cast<Widgets::Edit *>(publish);

  if (!mMessagesModel->IsContainer(item)) {
    const auto &message = mMessagesModel->getMessage(item);
    const auto &name = mMessagesModel->getName(item);
    edit->setMessage(message);
    edit->setInfoLine(name);
  }
}

void Client::onMessagesEdit(wxDataViewEvent &event) {
  if (mMessageExplicitEditRequest) {
    mMessageExplicitEditRequest = false;
    event.Skip();
  } else {
    event.Veto();
  }
}

void Client::onMessagesChanged(wxDataViewEvent &event) {
  const auto item = event.GetItem();
  if (!item.IsOk()) {
    event.Skip();
    return;
  }

  const auto name = mMessagesModel->getName(item);

  auto *publish = mPanes.at(Panes::Publish).panel;
  auto *edit = dynamic_cast<Widgets::Edit *>(publish);
  edit->setInfoLine(name);
}

void Client::onMessagesActivated(wxDataViewEvent &event) {
  const auto item = event.GetItem();
  if (!item.IsOk()) { return; }

  if (mMessagesModel->IsContainer(item)) {
    if (mMessagesCtrl->IsExpanded(item)) {
      mMessagesCtrl->Collapse(item);
    } else {
      mMessagesCtrl->Expand(item);
    }
  } else {
    const auto message = mMessagesModel->getMessage(item);
    mClient->publish(message);
  }
}

void Client::onMessagesDrag(wxDataViewEvent &event) {
  auto item = event.GetItem();
  mMessagesWasExpanded = mMessagesCtrl->IsExpanded(item);

  const void *id = item.GetID();
  uintptr_t message = 0;
  std::memcpy(&message, &id, sizeof(uintptr_t));
  auto *object = new wxTextDataObject(std::to_string(message));

  event.SetDataFormat(object->GetFormat());
  event.SetDataSize(object->GetDataSize());
  event.SetDataObject(object);

  // Required for windows, ignored on all else.
  event.SetDragFlags(wxDrag_AllowMove);

  event.Skip(false);
}

void Client::onMessagesDrop(wxDataViewEvent &event) {
  const auto target = event.GetItem();

  wxTextDataObject object;
  object.SetData(
    event.GetDataFormat(),
    event.GetDataSize(),
    event.GetDataBuffer() //
  );
  const uintptr_t message = std::stoul(object.GetText().ToStdString());
  void *id = nullptr;
  std::memcpy(&id, &message, sizeof(uintptr_t));
  auto item = wxDataViewItem(id);

  wxDataViewItem moved;

#ifdef WIN32

  const auto pIndex = event.GetProposedDropIndex();

  if (pIndex == -1) {
    if (!mMessagesModel->IsContainer(target)) {
      moved = mMessagesModel->moveAfter(item, target);
    } else {
      if (target.IsOk()) {
        moved = mMessagesModel->moveInsideFirst(item, target);
      } else {
        moved = mMessagesModel->moveInsideLast(item, target);
      }
    }
  } else {
    const auto index = (size_t)pIndex;
    moved = mMessagesModel->moveInsideAtIndex(item, target, index);
  }

#else

  if (mMessagesPossible.first) {
    if (mMessagesModel->IsContainer(target)) {
      moved = mMessagesModel->moveInsideFirst(item, target);
    } else {
      moved = mMessagesModel->moveAfter(item, target);
    }
  } else {
    if (target.IsOk()) {
      moved = mMessagesModel->moveBefore(item, target);
    } else {
      moved = mMessagesModel->moveInsideLast(item, target);
    }
  }

#endif // WIN32

  mMessagesPossible = {false, wxDataViewItem(nullptr)};

  if (!moved.IsOk()) { return; }

  mMessagesCtrl->Refresh();
  mMessagesCtrl->EnsureVisible(moved);
  if (mMessagesWasExpanded) { mMessagesCtrl->Expand(moved); }
  mMessagesCtrl->Select(moved);
}

void Client::onMessagesDropPossible(wxDataViewEvent &event) {
  const auto item = event.GetItem();
  mMessagesPossible = {true, item};

#ifdef WIN32

  event.Skip(false);

#else

  // On Linux:
  // - This event contains the first element of the target directory.
  // - If this skips, the drop event target is the hovered item.
  // - If this does not skip, the drop event target is this event's target.

  event.Skip(true);

#endif // WIN32
}

// Messages }

// Connection {

void Client::onConnectClicked(wxCommandEvent &event) {
  (void)event;
  if (mClient->connected()) {
    mLogger->info("Was connected");
    return;
  }

  allowCancel();
  mClient->connect();
}

void Client::onDisconnectClicked(wxCommandEvent &event) {
  (void)event;
  if (!mClient->connected()) {
    mLogger->info("Was not connected");
    return;
  }

  allowCancel();
  mClient->disconnect();
}

void Client::onCancelClicked(wxCommandEvent &event) {
  (void)event;
  mClient->cancel();
}

void Client::allowConnect() {
  if (mClient == nullptr) {
    allowNothing();
    return;
  }
  mProfileSizer->Show(mConnect);
  mProfileSizer->Hide(mDisconnect);
  mProfileSizer->Hide(mCancel);
  mIndicator->SetBitmap(mArtProvider.bitmap(Icon::Disconnected));
  const uint8_t brightness = mDarkMode ? 150 : 250;
  mIndicator->SetBackgroundColour(wxColor(brightness, 0, 0));
  mProfileSizer->Layout();
}

void Client::allowDisconnect() {
  if (mClient == nullptr) {
    allowNothing();
    return;
  }
  mProfileSizer->Hide(mConnect);
  mProfileSizer->Show(mDisconnect);
  mProfileSizer->Hide(mCancel);
  mIndicator->SetBitmap(mArtProvider.bitmap(Icon::Connected));
  const uint8_t brightness = mDarkMode ? 150 : 250;
  mIndicator->SetBackgroundColour(wxColor(0, brightness, 0));
  mProfileSizer->Layout();
}

void Client::allowCancel() {
  if (mClient == nullptr) {
    allowNothing();
    return;
  }
  mProfileSizer->Hide(mConnect);
  mProfileSizer->Hide(mDisconnect);
  mProfileSizer->Show(mCancel);
  mIndicator->SetBitmap(mArtProvider.bitmap(Icon::Connecting));
  const uint8_t brightness = mDarkMode ? 150 : 250;
  mIndicator->SetBackgroundColour(wxColor(brightness, brightness, 0));
  mProfileSizer->Layout();
}

void Client::allowNothing() {
  mProfileSizer->Hide(mConnect);
  mProfileSizer->Hide(mDisconnect);
  mProfileSizer->Hide(mCancel);
  mProfileSizer->Hide(mIndicator);
  mProfileSizer->Layout();
}

// Connection }

// Context {

void Client::onSubscriptionContext(wxDataViewEvent &event) {
  if (!event.GetItem().IsOk()) { return; }

  const auto item = event.GetItem();

  mSubscriptionsCtrl->Select(item);
  const bool muted = mSubscriptionsModel->getMuted(item);

  wxMenu menu;
  if (mClient != nullptr) {
    auto *unsubscribe = new wxMenuItem(
      nullptr,
      static_cast<unsigned>(ContextIDs::SubscriptionsUnsubscribe),
      "Unsubscribe"
    );
    unsubscribe->SetBitmap(mArtProvider.bitmap(Icon::Unsubscribe));
    menu.Append(unsubscribe);

    auto *clear = new wxMenuItem(
      nullptr,
      static_cast<unsigned>(ContextIDs::SubscriptionsClear),
      "Clear"
    );
    clear->SetBitmap(mArtProvider.bitmap(Icon::Clear));
    menu.Append(clear);
  }

  auto *color = new wxMenuItem(
    nullptr,
    static_cast<unsigned>(ContextIDs::SubscriptionsChangeColor),
    "Color change"
  );
  color->SetBitmap(mArtProvider.bitmap(Icon::NewColor));
  menu.Append(color);

  auto *solo = new wxMenuItem(
    nullptr,
    static_cast<unsigned>(ContextIDs::SubscriptionsSolo),
    "Solo"
  );
  solo->SetBitmap(mArtProvider.bitmap(Icon::Solo));
  menu.Append(solo);

  if (muted) {
    auto *unmute = new wxMenuItem(
      nullptr,
      static_cast<unsigned>(ContextIDs::SubscriptionsUnmute),
      "Unmute"
    );
    unmute->SetBitmap(mArtProvider.bitmap(Icon::Unmute));
    menu.Append(unmute);
  } else {
    auto *mute = new wxMenuItem(
      nullptr,
      static_cast<unsigned>(ContextIDs::SubscriptionsMute),
      "Mute"
    );
    mute->SetBitmap(mArtProvider.bitmap(Icon::Mute));
    menu.Append(mute);
  }
  PopupMenu(&menu);
}

void Client::onHistoryContext(wxDataViewEvent &event) {
  if (!event.GetItem().IsOk()) { return; }

  mHistoryCtrl->Select(event.GetItem());

  wxMenu menu;

  auto *copyTopic = new wxMenuItem(
    nullptr,
    static_cast<unsigned>(ContextIDs::HistoryCopyTopic),
    "Copy Topic"
  );
  copyTopic->SetBitmap(mArtProvider.bitmap(Icon::Copy));
  menu.Append(copyTopic);

  auto *copyPayload = new wxMenuItem(
    nullptr,
    static_cast<unsigned>(ContextIDs::HistoryCopyPayload),
    "Copy Payload"
  );
  copyPayload->SetBitmap(mArtProvider.bitmap(Icon::Copy));
  menu.Append(copyPayload);

  if (mClient != nullptr) {
    auto *edit = new wxMenuItem(
      nullptr,
      static_cast<unsigned>(ContextIDs::HistoryEdit),
      "Edit"
    );
    edit->SetBitmap(mArtProvider.bitmap(Icon::Edit));
    menu.Append(edit);

    auto *publish = new wxMenuItem(
      nullptr,
      static_cast<unsigned>(ContextIDs::HistoryResend),
      "Publish"
    );
    publish->SetBitmap(mArtProvider.bitmap(Icon::Publish));
    menu.Append(publish);

    auto *clearRetained = new wxMenuItem(
      nullptr,
      static_cast<unsigned>(ContextIDs::HistoryRetainedClear),
      "Clear retained"
    );
    clearRetained->SetBitmap(mArtProvider.bitmap(Icon::Delete));
    menu.Append(clearRetained);
  }

  auto *save = new wxMenuItem(
    nullptr,
    static_cast<unsigned>(ContextIDs::HistorySaveMessage),
    "Save Message"
  );
  save->SetBitmap(mArtProvider.bitmap(Icon::Save));
  menu.Append(save);

  PopupMenu(&menu);
}

void Client::onMessagesContext(wxDataViewEvent &event) {
  wxMenu menu;

  if (!event.GetItem().IsOk()) {
    mMessagesCtrl->UnselectAll();
  } else {
    const auto item = event.GetItem();

    if (!mMessagesModel->IsContainer(item)) {
      auto *publish = new wxMenuItem(
        nullptr,
        static_cast<unsigned>(ContextIDs::MessagePublish),
        "Publish"
      );
      publish->SetBitmap(mArtProvider.bitmap(Icon::Publish));
      menu.Append(publish);
    }

    if (!mMessagesModel->IsContainer(item)) {
      auto *overwrite = new wxMenuItem(
        nullptr,
        static_cast<unsigned>(ContextIDs::MessageOverwrite),
        "Overwrite"
      );
      overwrite->SetBitmap(mArtProvider.bitmap(Icon::SaveAs));
      menu.Append(overwrite);
    }

    auto *rename = new wxMenuItem(
      nullptr,
      static_cast<unsigned>(ContextIDs::MessageRename),
      "Rename"
    );
    rename->SetBitmap(mArtProvider.bitmap(Icon::Edit));
    menu.Append(rename);

    auto *del = new wxMenuItem(
      nullptr,
      static_cast<unsigned>(ContextIDs::MessageDelete),
      "Delete"
    );
    del->SetBitmap(mArtProvider.bitmap(Icon::Delete));
    menu.Append(del);
  }

  if (!event.GetItem().IsOk() || mMessagesModel->IsContainer(event.GetItem())) {
    auto *newFolder = new wxMenuItem(
      nullptr,
      static_cast<unsigned>(ContextIDs::MessageNewFolder),
      "New Folder"
    );
    newFolder->SetBitmap(mArtProvider.bitmap(Icon::NewDir));
    menu.Append(newFolder);

    auto *newMessage = new wxMenuItem(
      nullptr,
      static_cast<unsigned>(ContextIDs::MessageNewMessage),
      "New Message"
    );
    newMessage->SetBitmap(mArtProvider.bitmap(Icon::NewFile));
    menu.Append(newMessage);
  }

  PopupMenu(&menu);
}

void Client::onContextSelected(wxCommandEvent &event) {
  switch (static_cast<ContextIDs>(event.GetId())) {
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
    case ContextIDs::HistorySaveMessage: {
      onContextSelectedHistorySaveMessage(event);
    } break;
    case ContextIDs::HistoryCopyTopic: {
      onContextSelectedHistoryCopyTopic(event);
    } break;
    case ContextIDs::HistoryCopyPayload: {
      onContextSelectedHistoryCopyPayload(event);
    } break;
    case ContextIDs::MessageNewFolder: {
      onContextSelectedMessageNewFolder(event);
    } break;
    case ContextIDs::MessageNewMessage: {
      onContextSelectedMessageNewMessage(event);
    } break;
    case ContextIDs::MessageDelete: {
      onContextSelectedMessageDelete(event);
    } break;
    case ContextIDs::MessageRename: {
      onContextSelectedMessageRename(event);
    } break;
    case ContextIDs::MessagePublish: {
      onContextSelectedMessagePublish(event);
    } break;
    case ContextIDs::MessageOverwrite: {
      onContextSelectedMessageOverwrite(event);
    } break;
  }
  event.Skip();
}

void Client::onContextSelectedSubscriptionsUnsubscribe( //
  wxCommandEvent &event
) {
  (void)event;
  const auto item = mSubscriptionsCtrl->GetSelection();
  if (!item.IsOk()) { return; }

  const auto selected = mHistoryCtrl->GetSelection();
  if (selected.IsOk()) { mHistoryCtrl->Unselect(selected); }

  mSubscriptionsModel->unsubscribe(item);

  auto *preview = dynamic_cast<Widgets::Edit *>(mPanes.at(Panes::Preview).panel
  );
  preview->clear();
}

void Client::onContextSelectedSubscriptionsChangeColor(wxCommandEvent &event) {
  (void)event;
  const auto item = mSubscriptionsCtrl->GetSelection();
  if (!item.IsOk()) { return; }

  const auto color = Common::Helpers::colorFromNumber(
    static_cast<size_t>(mRandomColor(mRandomGenerator))
  );
  mSubscriptionsModel->setColor(item, color);

  mSubscriptionsCtrl->Refresh();
  mHistoryCtrl->Refresh();
}

void Client::onContextSelectedSubscriptionsMute(wxCommandEvent &event) {
  (void)event;
  const auto item = mSubscriptionsCtrl->GetSelection();
  if (!item.IsOk()) { return; }
  mSubscriptionsModel->mute(item);
}

void Client::onContextSelectedSubscriptionsUnmute(wxCommandEvent &event) {
  (void)event;
  const auto item = mSubscriptionsCtrl->GetSelection();
  if (!item.IsOk()) { return; }
  mSubscriptionsModel->unmute(item);
}

void Client::onContextSelectedSubscriptionsSolo(wxCommandEvent &event) {
  (void)event;
  const auto item = mSubscriptionsCtrl->GetSelection();
  if (!item.IsOk()) { return; }
  mSubscriptionsModel->solo(item);
}

void Client::onContextSelectedSubscriptionsClear(wxCommandEvent &event) {
  (void)event;
  const auto item = mSubscriptionsCtrl->GetSelection();
  if (!item.IsOk()) { return; }
  mSubscriptionsModel->clear(item);
}

void Client::onContextSelectedHistoryRetainedClear(wxCommandEvent &event) {
  (void)event;
  const auto item = mHistoryCtrl->GetSelection();
  if (!item.IsOk()) { return; }

  const auto topic = mHistoryModel->getTopic(item);
  const auto qos = mHistoryModel->getQos(item);
  const MQTT::Message message{topic, {}, qos, true, {}};
  mClient->publish(message);
}

void Client::onContextSelectedHistoryResend(wxCommandEvent &event) {
  (void)event;
  const auto item = mHistoryCtrl->GetSelection();
  if (!item.IsOk()) { return; }

  const auto &message = mHistoryModel->getMessage(item);
  mClient->publish(message);
}

void Client::onContextSelectedHistoryEdit(wxCommandEvent &event) {
  (void)event;
  const auto item = mHistoryCtrl->GetSelection();
  if (!item.IsOk()) { return; }

  auto *publish = dynamic_cast<Widgets::Edit *>(mPanes.at(Panes::Publish).panel
  );

  publish->clear();
  publish->setTopic(mHistoryModel->getTopic(item));
  publish->setQos(mHistoryModel->getQos(item));
  publish->setPayload(mHistoryModel->getPayload(item));
  publish->setRetained(mHistoryModel->getRetained(item));
}

void Client::onContextSelectedHistorySaveMessage(wxCommandEvent &event) {
  (void)event;
  auto messageItem = mMessagesCtrl->GetSelection();
  if (!mMessagesModel->IsContainer(messageItem)) {
    messageItem = mMessagesModel->GetParent(messageItem);
  }

  const auto historyItem = mHistoryCtrl->GetSelection();
  const auto &message = mHistoryModel->getMessage(historyItem);
  const auto inserted = mMessagesModel->createMessage(messageItem, message);

  if (!inserted.IsOk()) { return; }

  auto *publish = dynamic_cast<Widgets::Edit *>(mPanes.at(Panes::Publish).panel
  );
  publish->setMessage(mMessagesModel->getMessage(inserted));

  mMessagesCtrl->Select(inserted);
  mMessagesCtrl->EnsureVisible(inserted);

  auto *nameColumn = mMessageColumns.at(Messages::Column::Name);
  mMessageExplicitEditRequest = true;
  mMessagesCtrl->EditItem(inserted, nameColumn);
}

void Client::onContextSelectedHistoryCopyTopic(wxCommandEvent &event) {
  (void)event;
  const auto historyItem = mHistoryCtrl->GetSelection();
  const auto &message = mHistoryModel->getMessage(historyItem);

  if (wxTheClipboard->Open()) {
    auto *dataObject = new wxTextDataObject(message.topic);
    wxTheClipboard->SetData(dataObject);
    wxTheClipboard->Close();
  }
}

void Client::onContextSelectedHistoryCopyPayload(wxCommandEvent &event) {
  (void)event;
  const auto historyItem = mHistoryCtrl->GetSelection();
  const auto &message = mHistoryModel->getMessage(historyItem);

  if (wxTheClipboard->Open()) {
    auto *dataObject = new wxTextDataObject(message.payload);
    wxTheClipboard->SetData(dataObject);
    wxTheClipboard->Close();
  }
}

void Client::onContextSelectedMessageRename(wxCommandEvent &event) {
  (void)event;
  const auto item = mMessagesCtrl->GetSelection();
  if (!item.IsOk()) { return; }

  mMessageExplicitEditRequest = true;
  mMessagesCtrl->EditItem(item, mMessageColumns.at(Messages::Column::Name));
}

void Client::onContextSelectedMessagePublish(wxCommandEvent &event) {
  (void)event;
  const auto item = mMessagesCtrl->GetSelection();
  if (!item.IsOk()) { return; }

  auto *publish = dynamic_cast<Widgets::Edit *>(mPanes.at(Panes::Publish).panel
  );

  const auto &message = mMessagesModel->getMessage(item);
  const auto &name = mMessagesModel->getName(item);
  publish->setMessage(message);
  publish->setInfoLine(name);

  mClient->publish(message);
}

void Client::onContextSelectedMessageOverwrite(wxCommandEvent &event) {
  (void)event;
  const auto item = mMessagesCtrl->GetSelection();
  if (!item.IsOk()) { return; }

  auto *publish = dynamic_cast<Widgets::Edit *>(mPanes.at(Panes::Publish).panel
  );
  const auto message = publish->getMessage();
  mMessagesModel->replace(item, message);
}

void Client::onContextSelectedMessageDelete(wxCommandEvent &event) {
  (void)event;
  const auto item = mMessagesCtrl->GetSelection();
  if (!item.IsOk()) { return; }

  const bool done = mMessagesModel->remove(item);
  if (!done) { return; }

  const auto root = mMessagesModel->getRootItem();
  mMessagesCtrl->Select(root);

  auto *publish = dynamic_cast<Widgets::Edit *>(mPanes.at(Panes::Publish).panel
  );
  publish->setInfoLine({});
}

void Client::onContextSelectedMessageNewMessage(wxCommandEvent &event) {
  (void)event;
  auto item = mMessagesCtrl->GetSelection();
  if (!mMessagesModel->IsContainer(item)) {
    item = mMessagesModel->GetParent(item);
  }

  const auto inserted = mMessagesModel->createMessage(item, {});
  if (!inserted.IsOk()) { return; }

  auto *publish = dynamic_cast<Widgets::Edit *>(mPanes.at(Panes::Publish).panel
  );
  const auto &message = mMessagesModel->getMessage(item);
  const auto &name = mMessagesModel->getName(item);
  publish->setMessage(message);
  publish->setInfoLine(name);

  mMessagesCtrl->Select(inserted);
  mMessagesCtrl->EnsureVisible(inserted);

  auto *nameColumn = mMessageColumns.at(Messages::Column::Name);
  mMessageExplicitEditRequest = true;
  mMessagesCtrl->EditItem(inserted, nameColumn);
}

void Client::onContextSelectedMessageNewFolder(wxCommandEvent &event) {
  (void)event;
  auto item = mMessagesCtrl->GetSelection();
  if (!mMessagesModel->IsContainer(item)) {
    item = mMessagesModel->GetParent(item);
  }

  const auto inserted = mMessagesModel->createFolder(item);
  if (!inserted.IsOk()) { return; }

  mMessagesCtrl->Select(inserted);
  mMessagesCtrl->EnsureVisible(inserted);

  auto *nameColumn = mMessageColumns.at(Messages::Column::Name);
  mMessageExplicitEditRequest = true;
  mMessagesCtrl->EditItem(inserted, nameColumn);
}

// Context }

// MQTT::Client::Observer {

void Client::onConnected() {
  auto *event = new Events::Connection(Events::CONNECTION_CONNECTED);
  wxQueueEvent(this, event);
}

void Client::onDisconnected() {
  auto *event = new Events::Connection(Events::CONNECTION_DISCONNECTED);
  wxQueueEvent(this, event);
}

void Client::onConnectionLost() {
  auto *event = new Events::Connection(Events::CONNECTION_LOST);
  wxQueueEvent(this, event);
}

void Client::onConnectionFailure() {
  auto *event = new Events::Connection(Events::CONNECTION_FAILURE);
  wxQueueEvent(this, event);
}

void Client::onConnectedSync(Events::Connection &event) {
  (void)event;
  mLogger->info("Connected");
  allowDisconnect();
}

void Client::onDisconnectedSync(Events::Connection &event) {
  (void)event;
  mLogger->info("Disconnected");
  allowConnect();
}

void Client::onConnectionLostSync(Events::Connection &event) {
  (void)event;
  mLogger->info("Connection lost, reconnecting...");
  allowCancel();
}

void Client::onConnectionFailureSync(Events::Connection &event) {
  (void)event;
  mLogger->info("Unable to connect to server");
  allowConnect();
}

// MQTT::Client::Observer }

// Models::History::Observer {

void Client::onMessage(wxDataViewItem item) {
  if (!mAutoScroll->GetValue()) { return; }

  mHistoryCtrl->Select(item);
  mHistoryCtrl->EnsureVisible(item);

  auto *preview = dynamic_cast<Widgets::Edit *>(mPanes.at(Panes::Preview).panel
  );
  preview->setMessage(mHistoryModel->getMessage(item));
}

// Models::History::Observer }

// Subscriptions {

void Client::onSubscribeClicked(wxCommandEvent &event) {
  (void)event;
  const auto value = mFilter->GetValue();
  mFilter->SetValue({});
  const auto wxs = value.empty() ? "#" : value;
  const auto utf8 = wxs.ToUTF8();
  const std::string topic(utf8.data(), utf8.length());

  mTopicsSubscribed->append(topic);
  mSubscriptionsModel->subscribe(topic, MQTT::QoS::ExactlyOnce);
}

void Client::onSubscribeEnter(wxKeyEvent &event) {
  if (event.GetKeyCode() != WXK_RETURN) {
    event.Skip();
    return;
  }

  const auto value = mFilter->GetValue();
  mFilter->SetValue({});
  const auto wxs = value.empty() ? "#" : value;
  const auto utf8 = wxs.ToUTF8();
  const std::string topic(utf8.data(), utf8.length());

  mTopicsSubscribed->append(topic);
  mSubscriptionsModel->subscribe(topic, MQTT::QoS::ExactlyOnce);
  event.Skip();
}

void Client::onSubscriptionSelected(wxDataViewEvent &event) {
  (void)event;
  const auto item = mSubscriptionsCtrl->GetSelection();
  if (!item.IsOk()) { return; }
  const auto utf8 = mSubscriptionsModel->getFilter(item);
  const auto wxs = wxString::FromUTF8(utf8.data(), utf8.length());
  mFilter->SetValue(wxs);
}

// Subscriptions }

// Layouts {

void Client::onLayoutSelected(Events::Layout &event) {
  mAuiMan.LoadPerspective(event.getPerspective(), false);

  constexpr uint8_t ColorChannelHalf = 255 / 2;
  for (auto &[type, pane] : mPanes) {
    if (pane.toggle == nullptr) { continue; }
    const auto info = mAuiMan.GetPane(pane.panel);
    const auto color = (info.IsOk() && info.IsShown())
      ? wxNullColour
      : wxColor(ColorChannelHalf, ColorChannelHalf, ColorChannelHalf);
    pane.toggle->SetBackgroundColour(color);
  }

  mAuiMan.Update();
}

void Client::onLayoutResized(Events::Layout &event) {
  (void)event;
  mProfileSizer->Layout();
}

// Layouts }

// Publish {

void Client::onPublishClicked(wxCommandEvent &event) {
  (void)event;
  auto *publish = dynamic_cast<Widgets::Edit *>(mPanes.at(Panes::Publish).panel
  );
  const auto &message = publish->getMessage();
  if (message.topic.empty()) { return; }
  mTopicsPublished->append(message.topic);
  mClient->publish(message);
}

void Client::onPublishSaveMessage(Events::Edit &event) {
  (void)event;
  mLogger->info("Saving current message");
  auto messageItem = mMessagesCtrl->GetSelection();
  if (!mMessagesModel->IsContainer(messageItem)) {
    mLogger->info("Selecting parent");
    messageItem = mMessagesModel->GetParent(messageItem);
  }

  auto *publish = dynamic_cast<Widgets::Edit *>(mPanes.at(Panes::Publish).panel
  );

  const auto &message = publish->getMessage();
  mLogger->info("Storing {}", message.topic);
  auto inserted = mMessagesModel->createMessage(messageItem, message);
  if (inserted.IsOk()) {
    auto *publish = dynamic_cast<Widgets::Edit *>(
      mPanes.at(Panes::Publish).panel
    );
    const auto &message = mMessagesModel->getMessage(inserted);
    const auto &name = mMessagesModel->getName(inserted);
    publish->setMessage(message);
    publish->setInfoLine(name);

    mMessagesCtrl->Select(inserted);
    mMessagesCtrl->EnsureVisible(inserted);
    auto *nameColumn = mMessageColumns.at(Messages::Column::Name);
    mMessageExplicitEditRequest = true;
    mMessagesCtrl->EditItem(inserted, nameColumn);
  }
}

// Publish }

// History {

void Client::onHistorySelected(wxDataViewEvent &event) {
  if (!event.GetItem().IsOk()) { return; }
  const auto item = event.GetItem();

  auto *preview = dynamic_cast<Widgets::Edit *>(mPanes.at(Panes::Preview).panel
  );
  const auto message = mHistoryModel->getMessage(item);
  preview->setMessage(message);
}

void Client::onHistoryClearClicked(wxCommandEvent &event) {
  (void)event;
  mHistoryModel->clear();

  auto *preview = dynamic_cast<Widgets::Edit *>(mPanes.at(Panes::Preview).panel
  );
  preview->clear();
}

void Client::onHistoryRecordClicked(wxCommandEvent &event) {
  (void)event;
  const nlohmann::json contents{
    {"subscriptions", mSubscriptionsModel->toJson()},
    {"messages", mHistoryModel->toJson()},
  };

  mLogger->info("Queuing up recording event");
  auto *recording = new Events::Recording(Events::RECORDING_SAVE);
  recording->setContents(contents.dump());
  recording->setName(mName);
  wxQueueEvent(this, recording);
}

void Client::onHistoryDoubleClicked(wxDataViewEvent &event) {
  if (!event.GetItem().IsOk()) { return; }

  const auto messageItem = mMessagesCtrl->GetSelection();
  if (messageItem.IsOk()) { mMessagesCtrl->Unselect(messageItem); }

  const auto historyItem = event.GetItem();
  auto *publish = dynamic_cast<Widgets::Edit *>(mPanes.at(Panes::Publish).panel
  );
  auto message = mHistoryModel->getMessage(historyItem);
  message.retained = false;
  publish->setMessage(message);
}

void Client::onHistorySearchKey(wxKeyEvent &event) {
  const auto filter = mHistorySearchFilter->GetValue().ToStdString();
  long since = 0;
  long until = 0;
  mHistorySearchFilter->GetSelection(&since, &until);

  const auto isBackspace = event.GetKeyCode() == WXK_BACK;
  const auto isEnter = event.GetKeyCode() == WXK_RETURN;

  const auto
    isAllSelected = (since == 0 && until == static_cast<long>(filter.size()));
  const auto willBeEmpty = filter.empty() || filter.size() == 1
    || isAllSelected;

  if (isBackspace && willBeEmpty) {
    mHistoryModel->setFilter({});
  } else if (isEnter) {
    mHistoryModel->setFilter(filter);
  }

  event.Skip();
}

void Client::onHistorySearchButton(wxCommandEvent &event) {
  (void)event;
  const auto filter = mHistorySearchFilter->GetValue().ToStdString();
  mHistoryModel->setFilter(filter);
}

// History }

// Preview {

void Client::onPreviewSaveMessage(Events::Edit &event) {
  (void)event;
  mLogger->info("Saving current message");
  auto messageItem = mMessagesCtrl->GetSelection();
  if (!mMessagesModel->IsContainer(messageItem)) {
    mLogger->info("Selecting parent");
    messageItem = mMessagesModel->GetParent(messageItem);
  }

  auto *preview = dynamic_cast<Widgets::Edit *>(mPanes.at(Panes::Preview).panel
  );

  const auto &message = preview->getMessage();
  mLogger->info("Storing {}", message.topic);
  auto inserted = mMessagesModel->createMessage(messageItem, message);
  if (inserted.IsOk()) {
    mMessagesCtrl->Select(inserted);
    mMessagesCtrl->EnsureVisible(inserted);
    auto *nameColumn = mMessageColumns.at(Messages::Column::Name);
    mMessageExplicitEditRequest = true;
    mMessagesCtrl->EditItem(inserted, nameColumn);
  }
}

// Preview }

void Client::onClose(wxCloseEvent &event) {
  (void)event;
  if (mClient != nullptr && mClient->connected()) {
    auto *preview = dynamic_cast<Widgets::Edit *>(
      mPanes.at(Panes::Preview).panel
    );
    preview->setPayload("Closing...");
    mClient->disconnect();
  }
  Destroy();
}

//  }
