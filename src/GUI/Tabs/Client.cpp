#include <fstream>
#include <iterator>
#include <thread>

#include <nlohmann/json.hpp>
#include <wx/artprov.h>
#include <wx/clipbrd.h>

#include "Client.hpp"
#include "Common/Helpers.hpp"
#include "Common/Log.hpp"
#include "Common/Url.hpp"
#include "MQTT/Message.hpp"
#include "GUI/Events/Layout.hpp"
#include "GUI/Events/Recording.hpp"
#include "GUI/Resources/history/history-18x14.hpp"
#include "GUI/Resources/history/history-18x18.hpp"
#include "GUI/Resources/plus/plus-18x18.hpp"
#include "GUI/Resources/preview/preview-18x14.hpp"
#include "GUI/Resources/preview/preview-18x18.hpp"
#include "GUI/Resources/send/send-18x14.hpp"
#include "GUI/Resources/send/send-18x18.hpp"
#include "GUI/Resources/messages/messages-18x14.hpp"
#include "GUI/Resources/messages/messages-18x18.hpp"
#include "GUI/Resources/subscription/subscription-18x14.hpp"
#include "GUI/Resources/subscription/subscription-18x18.hpp"

using namespace Rapatas::Transmitron;
using namespace GUI::Tabs;
using namespace GUI::Models;
using namespace GUI;
using namespace Common;

constexpr size_t FontSize = 9;

Client::Client(
  wxWindow* parent,
  const MQTT::BrokerOptions &brokerOptions,
  const Types::ClientOptions &clientOptions,
  const wxObjectDataPtr<Models::Messages> &messages,
  const wxObjectDataPtr<Models::KnownTopics> &topicsSubscribed,
  const wxObjectDataPtr<Models::KnownTopics> &topicsPublished,
  const wxObjectDataPtr<Models::Layouts> &layoutsModel,
  const wxString &name,
  bool darkMode,
  int optionsHeight
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
  mClientOptions(clientOptions),
  mFont(wxFontInfo(FontSize).FaceName("Consolas")),
  mDarkMode(darkMode),
  mOptionsHeight(optionsHeight),
  mTopicsSubscribed(topicsSubscribed),
  mTopicsPublished(topicsPublished),
  mLayoutsModel(layoutsModel),
  mMasterSizer(new wxBoxSizer(wxVERTICAL)),
  mMessagesModel(messages),
  mClient(std::make_shared<MQTT::Client>()),
  mMqttObserverId(mClient->attachObserver(this))
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
  wxWindow* parent,
  const wxObjectDataPtr<Models::History> &historyModel,
  const wxObjectDataPtr<Models::Subscriptions> &subscriptionsModel,
  const wxObjectDataPtr<Models::Layouts> &layoutsModel,
  const wxString &name,
  bool darkMode,
  int optionsHeight
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
  mFont(wxFontInfo(FontSize).FaceName("Consolas")),
  mDarkMode(darkMode),
  mOptionsHeight(optionsHeight),
  mLayoutsModel(layoutsModel),
  mMasterSizer(new wxBoxSizer(wxVERTICAL)),
  mHistoryModel(historyModel),
  mSubscriptionsModel(subscriptionsModel)
{
  setupPanels();
}

void Client::setupPanels()
{
  Bind(wxEVT_CLOSE_WINDOW, &Client::onClose, this);
  Bind(wxEVT_COMMAND_MENU_SELECTED, &Client::onContextSelected, this);

  mLogger = Common::Log::create("GUI::Client");

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
  mPanes.at(Panes::Preview).info.Bottom();

  mPanes.at(Panes::History).info.Layer(0);
  mPanes.at(Panes::Subscriptions).info.Layer(1);
  mPanes.at(Panes::Preview).info.Layer(2);

  if (mClient != nullptr)
  {
    mPanes[Panes::Messages] = {
      "Messages",
      {},
      nullptr,
      bin2cMessages18x18(),
      bin2cMessages18x14(),
      nullptr
    };

    mPanes[Panes::Publish] = {
      "Publish",
      {},
      nullptr,
      bin2cSend18x18(),
      bin2cSend18x14(),
      nullptr
    };

    mPanes.at(Panes::Messages).info.Left();
    mPanes.at(Panes::Publish).info.Bottom();

    mPanes.at(Panes::Messages).info.Layer(1);
    mPanes.at(Panes::Publish).info.Layer(2);
  }

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

  if (mClient != nullptr)
  {
    setupPanelMessages(wrapper);
    setupPanelPublish(wrapper);
  }
  setupPanelSubscriptions(wrapper);
  setupPanelPreview(wrapper);
  setupPanelHistory(wrapper);
  setupPanelConnect(this);

  mMasterSizer->Add(mProfileBar, 0, wxEXPAND);
  mMasterSizer->Add(wrapper, 1, wxEXPAND);
  SetSizer(mMasterSizer);

  mAuiMan.SetManagedWindow(wrapper);
  for (const auto &pane : mPanes)
  {
    mAuiMan.AddPane(pane.second.panel, pane.second.info);
  }

  const auto layout = mClientOptions.getLayout();
  mLayouts->setSelectedLayout(layout);
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

void Client::focus() const
{
  mFilter->SetFocus();
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

  if (mClient != nullptr)
  {
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
    wxSize(mOptionsHeight, mOptionsHeight)
  );
  mHistorySearchButton->SetBitmap(wxArtProvider::GetBitmap(wxART_FIND));
  mHistorySearchButton->Bind(
    wxEVT_BUTTON,
    &Client::onHistorySearchButton,
    this
  );

  mAutoScroll = new wxCheckBox(panel, -1, "auto-scroll");
  mAutoScroll->SetValue(true);

  mHistoryClear = new wxButton(
    panel,
    -1,
    "",
    wxDefaultPosition,
    wxSize(-1, mOptionsHeight)
  );
  mHistoryClear->SetToolTip("Clear history");
  mHistoryClear->SetBitmap(wxArtProvider::GetBitmap(wxART_DELETE));

  mHistoryRecord = new wxButton(
    panel,
    -1,
    "",
    wxDefaultPosition,
    wxSize(-1, mOptionsHeight)
  );
  mHistoryRecord->SetToolTip("Store history recording");
  mHistoryRecord->SetBitmap(wxArtProvider::GetBitmap(wxART_FILE_SAVE));

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
  mHistoryRecord->Bind(
    wxEVT_BUTTON,
    &Client::onHistoryRecordClicked,
    this
  );

  if (mClient == nullptr)
  {
    vsizer->Hide(hsizer);
    vsizer->Layout();
  }
}

void Client::setupPanelConnect(wxWindow *parent)
{
  mProfileBar = new wxPanel(parent, -1);

  mConnect    = new wxButton(
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
  mCancel     = new wxButton(
    mProfileBar,
    -1,
    "Cancel",
    wxDefaultPosition,
    wxSize(-1, mOptionsHeight)
  );
  mCancel->SetToolTip("Stop connection attempt");

  mLayouts = new Widgets::Layouts(mProfileBar, -1, mLayoutsModel, mAuiMan, mOptionsHeight);
  mLayouts->Bind(Events::LAYOUT_SELECTED, &Client::onLayoutSelected, this);
  mLayouts->Bind(Events::LAYOUT_RESIZED,  &Client::onLayoutResized,  this);

  auto callback = [this](Panes pane, wxCommandEvent &/* event */)
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
  mProfileSizer->SetMinSize(0, mOptionsHeight);
  mProfileSizer->Add(mConnect, 0, wxEXPAND);
  mProfileSizer->Add(mDisconnect, 0, wxEXPAND);
  mProfileSizer->Add(mCancel, 0, wxEXPAND);
  allowCancel();

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
      wxSize(mOptionsHeight, mOptionsHeight)
    );
    button->SetToolTip(pane.second.name);
    button->SetBitmap(*bitmap);
    button->Bind(
      wxEVT_BUTTON,
      std::bind(callback, pane.first, std::placeholders::_1)
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
  auto *panel = new wxPanel(parent, -1, wxDefaultPosition);
  mPanes.at(Panes::Subscriptions).panel = panel;

  mFilter = new Widgets::TopicCtrl(panel, -1);
  mFilter->addKnownTopics(mTopicsSubscribed);
  mFilter->SetFont(mFont);

  mSubscribe = new wxBitmapButton(
    panel,
    -1,
    *bin2cPlus18x18(),
    wxDefaultPosition,
    wxSize(mOptionsHeight, mOptionsHeight)
  );
  mSubscribe->SetToolTip("Subscribe");

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

  if (mClient != nullptr)
  {
    mSubscriptionsModel = new Models::Subscriptions(mClient);
  }
  mSubscriptionsCtrl->AssociateModel(mSubscriptionsModel.get());

  mSubscriptionsCtrl->SetFont(mFont);

  wxBoxSizer *vsizer = new wxBoxSizer(wxOrientation::wxVERTICAL);
  wxBoxSizer *hsizer = new wxBoxSizer(wxOrientation::wxHORIZONTAL);
  hsizer->SetMinSize(0, mOptionsHeight);
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
    wxEVT_KEY_UP,
    &Client::onSubscribeEnter,
    this
  );
}

void Client::setupPanelPreview(wxWindow *parent)
{
  auto *panel = new Widgets::Edit(parent, -1, mOptionsHeight, mDarkMode);
  mPanes.at(Panes::Preview).panel = panel;
  panel->setReadOnly(true);
  panel->Bind(
    Events::EDIT_SAVE_MESSAGE,
    &Client::onPreviewSaveMessage,
    this
  );
}

void Client::setupPanelPublish(wxWindow *parent)
{
  auto *panel = new Widgets::Edit(
    parent,
    -1,
    mOptionsHeight,
    mDarkMode
  );
  panel->addKnownTopics(mTopicsPublished);
  mPanes.at(Panes::Publish).panel = panel;
  panel->Bind(
    Events::EDIT_PUBLISH,
    &Client::onPublishClicked,
    this
  );
  panel->Bind(
    Events::EDIT_SAVE_MESSAGE,
    &Client::onPublishSaveMessage,
    this
  );
}

void Client::setupPanelMessages(wxWindow *parent)
{
  auto *renderer = new wxDataViewIconTextRenderer(
    wxDataViewIconTextRenderer::GetDefaultType(),
    wxDATAVIEW_CELL_EDITABLE
  );

  mMessageColumns.at(Messages::Column::Name) = new wxDataViewColumn(
    L"name",
    renderer,
    (unsigned)Models::Messages::Column::Name,
    wxCOL_WIDTH_AUTOSIZE,
    wxALIGN_LEFT
  );

  auto *panel = new wxPanel(parent, -1);
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

  wxBoxSizer *vsizer = new wxBoxSizer(wxOrientation::wxVERTICAL);
  vsizer->Add(mMessagesCtrl, 1, wxEXPAND);
  panel->SetSizer(vsizer);
  mMessagesCtrl->Bind(
    wxEVT_DATAVIEW_ITEM_CONTEXT_MENU,
    &Client::onMessagesContext,
    this
  );
  mMessagesCtrl->Bind(
    wxEVT_DATAVIEW_ITEM_START_EDITING,
    &Client::onMessagesEdit,
    this
  );
  mMessagesCtrl->Bind(
    wxEVT_DATAVIEW_ITEM_VALUE_CHANGED,
    &Client::onMessagesChanged,
    this
  );
  mMessagesCtrl->Bind(
    wxEVT_DATAVIEW_SELECTION_CHANGED,
    &Client::onMessagesSelected,
    this
  );
  mMessagesCtrl->Bind(
    wxEVT_COMMAND_DATAVIEW_ITEM_ACTIVATED,
    &Client::onMessagesActivated,
    this
  );
  mMessagesCtrl->Bind(
    wxEVT_DATAVIEW_ITEM_BEGIN_DRAG,
    &Client::onMessagesDrag,
    this
  );
  mMessagesCtrl->Bind(
    wxEVT_DATAVIEW_ITEM_DROP,
    &Client::onMessagesDrop,
    this
  );
  mMessagesCtrl->Bind(
    wxEVT_DATAVIEW_ITEM_DROP_POSSIBLE,
    &Client::onMessagesDropPossible,
    this
  );
}

// Setup }

// Messages {

void Client::onMessagesSelected(wxDataViewEvent &event)
{
  auto item = event.GetItem();
  if (!item.IsOk())
  {
    event.Skip();
    return;
  }

  auto *publish = dynamic_cast<Widgets::Edit*>(
    mPanes.at(Panes::Publish).panel
  );

  if (!mMessagesModel->IsContainer(item))
  {
    const auto &message = mMessagesModel->getMessage(item);
    const auto &name = mMessagesModel->getName(item);
    publish->setMessage(message);
    publish->setInfoLine(name);
  }
}

void Client::onMessagesEdit(wxDataViewEvent &event)
{
  if (mMessageExplicitEditRequest)
  {
    mMessageExplicitEditRequest = false;
    event.Skip();
  }
  else
  {
    event.Veto();
  }
}

void Client::onMessagesChanged(wxDataViewEvent &event)
{
  const auto item = event.GetItem();
  if (!item.IsOk())
  {
    event.Skip();
    return;
  }

  const auto name = mMessagesModel->getName(item);

  auto *publish = dynamic_cast<Widgets::Edit*>(
    mPanes.at(Panes::Publish).panel
  );
  publish->setInfoLine(name);
}

void Client::onMessagesActivated(wxDataViewEvent &event)
{
  const auto item = event.GetItem();
  if (!item.IsOk()) { return; }

  if (mMessagesModel->IsContainer(item))
  {
    if (mMessagesCtrl->IsExpanded(item))
    {
      mMessagesCtrl->Collapse(item);
    }
    else
    {
      mMessagesCtrl->Expand(item);
    }
  }
  else
  {
    const auto message = mMessagesModel->getMessage(item);
    mClient->publish(message);
  }
}

void Client::onMessagesDrag(wxDataViewEvent &event)
{
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

void Client::onMessagesDrop(wxDataViewEvent &event)
{
  const auto target = event.GetItem();

  wxTextDataObject object;
  object.SetData(event.GetDataFormat(), event.GetDataSize(), event.GetDataBuffer());
  const uintptr_t message = std::stoul(object.GetText().ToStdString());
  void *id = nullptr;
  std::memcpy(&id, &message, sizeof(uintptr_t));
  auto item = wxDataViewItem(id);

  wxDataViewItem moved;

#ifdef WIN32

  const auto pIndex = event.GetProposedDropIndex();

  if (pIndex == -1)
  {
    if (!mMessagesModel->IsContainer(target))
    {
      moved = mMessagesModel->moveAfter(item, target);
    }
    else
    {
      if (target.IsOk())
      {
        moved = mMessagesModel->moveInsideFirst(item, target);
      }
      else
      {
        moved = mMessagesModel->moveInsideLast(item, target);
      }
    }
  }
  else
  {
    const auto index = (size_t)pIndex;
    moved = mMessagesModel->moveInsideAtIndex(item, target, index);
  }

#else

  if (mMessagesPossible.first)
  {
    if (mMessagesModel->IsContainer(target))
    {
      moved = mMessagesModel->moveInsideFirst(item, target);
    }
    else
    {
      moved = mMessagesModel->moveAfter(item, target);
    }
  }
  else
  {
    if (target.IsOk())
    {
      moved = mMessagesModel->moveBefore(item, target);
    }
    else
    {
      moved = mMessagesModel->moveInsideLast(item, target);
    }
  }

#endif // WIN32

  mMessagesPossible = {false, wxDataViewItem(nullptr)};

  if (!moved.IsOk())
  {
    return;
  }

  mMessagesCtrl->Refresh();
  mMessagesCtrl->EnsureVisible(moved);
  if (mMessagesWasExpanded)
  {
    mMessagesCtrl->Expand(moved);
  }
  mMessagesCtrl->Select(moved);
}

void Client::onMessagesDropPossible(wxDataViewEvent &event)
{
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

void Client::onConnectClicked(wxCommandEvent &/* event */)
{
  if (mClient->connected())
  {
    mLogger->info("Was connected");
    return;
  }

  allowCancel();
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
  if (mClient == nullptr)
  {
    allowNothing();
    return;
  }
  mProfileSizer->Show(mConnect);
  mProfileSizer->Hide(mDisconnect);
  mProfileSizer->Hide(mCancel);
  mProfileSizer->Layout();
}

void Client::allowDisconnect()
{
  if (mClient == nullptr)
  {
    allowNothing();
    return;
  }
  mProfileSizer->Hide(mConnect);
  mProfileSizer->Show(mDisconnect);
  mProfileSizer->Hide(mCancel);
  mProfileSizer->Layout();
}

void Client::allowCancel()
{
  if (mClient == nullptr)
  {
    allowNothing();
    return;
  }
  mProfileSizer->Hide(mConnect);
  mProfileSizer->Hide(mDisconnect);
  mProfileSizer->Show(mCancel);
  mProfileSizer->Layout();
}

void Client::allowNothing()
{
  mProfileSizer->Hide(mConnect);
  mProfileSizer->Hide(mDisconnect);
  mProfileSizer->Hide(mCancel);
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
  if (mClient != nullptr)
  {
    menu.Append((unsigned)ContextIDs::SubscriptionsUnsubscribe, "Unsubscribe");
    menu.Append((unsigned)ContextIDs::SubscriptionsClear, "Clear");
  }
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

void Client::onHistoryContext(wxDataViewEvent& event)
{
  if (!event.GetItem().IsOk()) { return; }

  mHistoryCtrl->Select(event.GetItem());

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

  if (mClient != nullptr)
  {
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
  }

  auto *save = new wxMenuItem(
    nullptr,
    (unsigned)ContextIDs::HistorySaveMessage,
    "Save Message"
  );
  save->SetBitmap(wxArtProvider::GetBitmap(wxART_FILE_SAVE));
  menu.Append(save);

  PopupMenu(&menu);
}

void Client::onMessagesContext(wxDataViewEvent& event)
{
  wxMenu menu;

  if (!event.GetItem().IsOk())
  {
    mMessagesCtrl->UnselectAll();
  }
  else
  {
    const auto item = event.GetItem();

    if (!mMessagesModel->IsContainer(item))
    {
      auto *publish = new wxMenuItem(
        nullptr,
        (unsigned)ContextIDs::MessagePublish,
        "Publish"
      );
      menu.Append(publish);
    }

    if (!mMessagesModel->IsContainer(item))
    {
      auto *overwrite = new wxMenuItem(
        nullptr,
        (unsigned)ContextIDs::MessageOverwrite,
        "Overwrite"
      );
      overwrite->SetBitmap(wxArtProvider::GetBitmap(wxART_FILE_SAVE_AS));
      menu.Append(overwrite);
    }

    auto *rename = new wxMenuItem(
      nullptr,
      (unsigned)ContextIDs::MessageRename,
      "Rename"
    );
    rename->SetBitmap(wxArtProvider::GetBitmap(wxART_EDIT));
    menu.Append(rename);

    auto *del = new wxMenuItem(
      nullptr,
      (unsigned)ContextIDs::MessageDelete,
      "Delete"
    );
    del->SetBitmap(wxArtProvider::GetBitmap(wxART_DELETE));
    menu.Append(del);
  }

  if (
    !event.GetItem().IsOk()
    || mMessagesModel->IsContainer(event.GetItem())
  ) {
    auto *newFolder = new wxMenuItem(
      nullptr,
      (unsigned)ContextIDs::MessageNewFolder,
      "New Folder"
    );
    newFolder->SetBitmap(wxArtProvider::GetBitmap(wxART_NEW_DIR));
    menu.Append(newFolder);

    auto *newMessage = new wxMenuItem(
      nullptr,
      (unsigned)ContextIDs::MessageNewMessage,
      "New Message"
    );
    newMessage->SetBitmap(wxArtProvider::GetBitmap(wxART_NORMAL_FILE));
    menu.Append(newMessage);
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

  const auto color = Common::Helpers::colorFromNumber((size_t)std::abs(rand()));
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
  MQTT::Message message {topic, {}, qos, true, {}};
  mClient->publish(message);
}

void Client::onContextSelectedHistoryResend(wxCommandEvent &/* event */)
{
  const auto item = mHistoryCtrl->GetSelection();
  if (!item.IsOk()) { return; }

  const auto &message = mHistoryModel->getMessage(item);
  mClient->publish(message);
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

void Client::onContextSelectedHistorySaveMessage(wxCommandEvent &/* event */)
{
  auto messageItem = mMessagesCtrl->GetSelection();
  if (!mMessagesModel->IsContainer(messageItem))
  {
    messageItem = mMessagesModel->GetParent(messageItem);
  }

  const auto historyItem = mHistoryCtrl->GetSelection();
  const auto &message = mHistoryModel->getMessage(historyItem);
  const auto inserted = mMessagesModel->createMessage(messageItem, message);

  if (!inserted.IsOk())
  {
    return;
  }

  auto *publish = dynamic_cast<Widgets::Edit*>(
    mPanes.at(Panes::Publish).panel
  );
  publish->setMessage(mMessagesModel->getMessage(inserted));

  mMessagesCtrl->Select(inserted);
  mMessagesCtrl->EnsureVisible(inserted);

  auto *nameColumn = mMessageColumns.at(Messages::Column::Name);
  mMessageExplicitEditRequest = true;
  mMessagesCtrl->EditItem(inserted, nameColumn);
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

void Client::onContextSelectedMessageRename(wxCommandEvent &/* event */)
{
  const auto item = mMessagesCtrl->GetSelection();
  if (!item.IsOk()) { return; }

  mMessageExplicitEditRequest = true;
  mMessagesCtrl->EditItem(item, mMessageColumns.at(Messages::Column::Name));
}

void Client::onContextSelectedMessagePublish(wxCommandEvent &/* event */)
{
  const auto item = mMessagesCtrl->GetSelection();
  if (!item.IsOk()) { return; }

  auto *publish = dynamic_cast<Widgets::Edit*>(
    mPanes.at(Panes::Publish).panel
  );

  const auto &message = mMessagesModel->getMessage(item);
  const auto &name = mMessagesModel->getName(item);
  publish->setMessage(message);
  publish->setInfoLine(name);

  mClient->publish(message);
}

void Client::onContextSelectedMessageOverwrite(wxCommandEvent &/* event */)
{
  const auto item = mMessagesCtrl->GetSelection();
  if (!item.IsOk()) { return; }

  auto *publish = dynamic_cast<Widgets::Edit*>(
    mPanes.at(Panes::Publish).panel
  );
  const auto message = publish->getMessage();

  mMessagesModel->replace(item, message);
}

void Client::onContextSelectedMessageDelete(wxCommandEvent &/* event */)
{
  const auto item = mMessagesCtrl->GetSelection();
  if (!item.IsOk()) { return; }

  const bool done = mMessagesModel->remove(item);
  if (!done)
  {
    return;
  }

  const auto root = mMessagesModel->getRootItem();
  mMessagesCtrl->Select(root);

  auto *publish = dynamic_cast<Widgets::Edit*>(
    mPanes.at(Panes::Publish).panel
  );
  publish->setInfoLine({});
}

void Client::onContextSelectedMessageNewMessage(wxCommandEvent &/* event */)
{
  auto item = mMessagesCtrl->GetSelection();
  if (!mMessagesModel->IsContainer(item))
  {
    item = mMessagesModel->GetParent(item);
  }

  const auto inserted = mMessagesModel->createMessage(item, {});
  if (!inserted.IsOk())
  {
    return;
  }

  auto *publish = dynamic_cast<Widgets::Edit*>(
    mPanes.at(Panes::Publish).panel
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

void Client::onContextSelectedMessageNewFolder(wxCommandEvent &/* event */)
{
  auto item = mMessagesCtrl->GetSelection();
  if (!mMessagesModel->IsContainer(item))
  {
    item = mMessagesModel->GetParent(item);
  }

  const auto inserted = mMessagesModel->createFolder(item);
  if (!inserted.IsOk())
  {
    return;
  }

  mMessagesCtrl->Select(inserted);
  mMessagesCtrl->EnsureVisible(inserted);

  auto *nameColumn = mMessageColumns.at(Messages::Column::Name);
  mMessageExplicitEditRequest = true;
  mMessagesCtrl->EditItem(inserted, nameColumn);
}

// Context }

// MQTT::Client::Observer {

void Client::onConnected()
{
  auto *event = new Events::Connection(Events::CONNECTION_CONNECTED);
  wxQueueEvent(this, event);
}

void Client::onDisconnected()
{
  auto *event = new Events::Connection(Events::CONNECTION_DISCONNECTED);
  wxQueueEvent(this, event);
}

void Client::onConnectionLost()
{
  auto *event = new Events::Connection(Events::CONNECTION_LOST);
  wxQueueEvent(this, event);
}

void Client::onConnectionFailure()
{
  auto *event = new Events::Connection(Events::CONNECTION_FAILURE);
  wxQueueEvent(this, event);
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

  mTopicsSubscribed->append(topic);
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

  mTopicsSubscribed->append(topic);
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
  mProfileSizer->Layout();
}

// Layouts }

// Publish {

void Client::onPublishClicked(wxCommandEvent &/* event */)
{
  auto *publish = dynamic_cast<Widgets::Edit*>(mPanes.at(Panes::Publish).panel);
  const auto &message = publish->getMessage();
  if (message.topic.empty()) { return; }
  mTopicsPublished->append(message.topic);
  mClient->publish(message);
}

void Client::onPublishSaveMessage(Events::Edit &/* event */)
{
  mLogger->info("Saving current message");
  auto messageItem = mMessagesCtrl->GetSelection();
  if (!mMessagesModel->IsContainer(messageItem))
  {
    mLogger->info("Selecting parent");
    messageItem = mMessagesModel->GetParent(messageItem);
  }

  auto *publish = dynamic_cast<Widgets::Edit*>(
    mPanes.at(Panes::Publish).panel
  );

  const auto &message = publish->getMessage();
  mLogger->info("Storing {}", message.topic);
  auto inserted = mMessagesModel->createMessage(messageItem, message);
  if (inserted.IsOk())
  {
    auto *publish = dynamic_cast<Widgets::Edit*>(
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

void Client::onHistorySelected(wxDataViewEvent &event)
{
  if (!event.GetItem().IsOk()) { return; }
  const auto item = event.GetItem();

  auto *preview = dynamic_cast<Widgets::Edit*>(mPanes.at(Panes::Preview).panel);
  const auto message = mHistoryModel->getMessage(item);
  preview->setMessage(message);
}

void Client::onHistoryClearClicked(wxCommandEvent &/* event */)
{
  mHistoryModel->clear();

  auto *preview = dynamic_cast<Widgets::Edit*>(
    mPanes.at(Panes::Preview).panel
  );
  preview->clear();
}

void Client::onHistoryRecordClicked(wxCommandEvent &/* event */)
{
  const nlohmann::json contents {
    {"subscriptions", mSubscriptionsModel->toJson() },
    {"messages", mHistoryModel->toJson() },
  };

  mLogger->info("Queuing up recording event");
  auto *event = new Events::Recording(Events::RECORDING_SAVE);
  event->setContents(contents.dump());
  event->setName(mName);
  wxQueueEvent(this, event);
}

void Client::onHistoryDoubleClicked(wxDataViewEvent &event)
{
  if (!event.GetItem().IsOk()) { return; }

  const auto messageItem = mMessagesCtrl->GetSelection();
  if (messageItem.IsOk())
  {
    mMessagesCtrl->Unselect(messageItem);
  }

  const auto historyItem = event.GetItem();
  auto *publish = dynamic_cast<Widgets::Edit*>(mPanes.at(Panes::Publish).panel);
  auto message = mHistoryModel->getMessage(historyItem);
  message.retained = false;
  publish->setMessage(message);
}

void Client::onHistorySearchKey(wxKeyEvent &event)
{
  const auto filter = mHistorySearchFilter->GetValue().ToStdString();
  long since = 0;
  long until = 0;
  mHistorySearchFilter->GetSelection(&since, &until);

  const auto isBackspace = event.GetKeyCode() == WXK_BACK;
  const auto isEnter     = event.GetKeyCode() == WXK_RETURN;

  const auto isAllSelected = (since == 0 && until == (long)filter.size());
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

void Client::onPreviewSaveMessage(Events::Edit &/* event */)
{
  mLogger->info("Saving current message");
  auto messageItem = mMessagesCtrl->GetSelection();
  if (!mMessagesModel->IsContainer(messageItem))
  {
    mLogger->info("Selecting parent");
    messageItem = mMessagesModel->GetParent(messageItem);
  }

  auto *preview = dynamic_cast<Widgets::Edit*>(
    mPanes.at(Panes::Preview).panel
  );

  const auto &message = preview->getMessage();
  mLogger->info("Storing {}", message.topic);
  auto inserted = mMessagesModel->createMessage(messageItem, message);
  if (inserted.IsOk())
  {
    mMessagesCtrl->Select(inserted);
    mMessagesCtrl->EnsureVisible(inserted);
    auto *nameColumn = mMessageColumns.at(Messages::Column::Name);
    mMessageExplicitEditRequest = true;
    mMessagesCtrl->EditItem(inserted, nameColumn);
  }
}

// Preview }

void Client::onClose(wxCloseEvent &/* event */)
{
  if (mClient != nullptr && mClient->connected())
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
