#pragma once

#include <spdlog/spdlog.h>
#include <wx/event.h>
#include <wx/listctrl.h>
#include <wx/splitter.h>
#include <wx/aui/aui.h>
#include <wx/tglbtn.h>
#include <wx/combobox.h>

#include "MQTT/Client.hpp"
#include "GUI/Events/Connection.hpp"
#include "GUI/Events/Layout.hpp"
#include "GUI/Models/KnownTopics.hpp"
#include "GUI/Models/Layouts.hpp"
#include "GUI/Types/ClientOptions.hpp"
#include "GUI/Widgets/TopicCtrl.hpp"
#include "GUI/Widgets/Layouts.hpp"
#include "GUI/Models/History.hpp"
#include "GUI/Models/Subscriptions.hpp"
#include "GUI/Models/Messages.hpp"
#include "GUI/Widgets/Edit.hpp"

namespace Rapatas::Transmitron::GUI::Tabs
{

class Client :
  public wxPanel,
  public Models::History::Observer,
  public MQTT::Client::Observer
{
public:

  Client(
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
  );

  Client(
    wxWindow* parent,
    const wxObjectDataPtr<Models::History> &historyModel,
    const wxObjectDataPtr<Models::Subscriptions> &subscriptionsModel,
    const wxObjectDataPtr<Models::Layouts> &layoutsModel,
    const wxString &name,
    bool darkMode,
    int optionsHeight
  );

  Client(const Client &other) = delete;
  Client(Client &&other) = delete;
  Client &operator=(const Client &other) = delete;
  Client &operator=(Client &&other) = delete;
  ~Client();

  void focus() const;

private:

  enum class ContextIDs : unsigned
  {
    SubscriptionsUnsubscribe,
    SubscriptionsChangeColor,
    SubscriptionsMute,
    SubscriptionsUnmute,
    SubscriptionsSolo,
    SubscriptionsClear,
    HistoryRetainedClear,
    HistoryResend,
    HistoryEdit,
    HistorySaveMessage,
    HistoryCopyTopic,
    HistoryCopyPayload,
    MessageRename,
    MessageDelete,
    MessageNewMessage,
    MessageNewFolder,
    MessagePublish,
    MessageOverwrite,
  };

  enum class Panes : unsigned
  {
    History = 0,
    Subscriptions = 1,
    Messages = 2,
    Publish = 3,
    Preview = 4,
  };

  struct Pane
  {
    std::string name;
    wxAuiPaneInfo info;
    wxPanel *panel = nullptr;
    const wxBitmap *icon18x18 = nullptr;
    const wxBitmap *icon18x14 = nullptr;
    wxButton *toggle = nullptr;
  };

  static constexpr size_t PaneMinWidth = 200;
  static constexpr size_t PaneMinHeight = 100;
  static constexpr size_t EditorMinHeight = 200;

  std::shared_ptr<spdlog::logger> mLogger;
  std::map<Panes, Pane> mPanes;
  const wxString mName;
  const Types::ClientOptions mClientOptions;
  const wxFont mFont;
  const bool mDarkMode;
  const int mOptionsHeight;
  wxObjectDataPtr<Models::KnownTopics> mTopicsSubscribed;
  wxObjectDataPtr<Models::KnownTopics> mTopicsPublished;
  wxObjectDataPtr<Models::Layouts> mLayoutsModel;

  wxBoxSizer *mMasterSizer = nullptr;

  Widgets::Layouts *mLayouts = nullptr;

  // Profile:
  wxPanel *mProfileBar = nullptr;
  wxBoxSizer *mProfileSizer = nullptr;
  wxButton *mConnect = nullptr;
  wxButton *mDisconnect = nullptr;
  wxButton *mCancel = nullptr;

  // History:
  wxObjectDataPtr<Models::History> mHistoryModel;
  wxDataViewCtrl *mHistoryCtrl = nullptr;
  wxCheckBox *mAutoScroll = nullptr;
  wxButton *mHistoryClear = nullptr;
  wxButton *mHistoryRecord = nullptr;
  Widgets::TopicCtrl *mHistorySearchFilter = nullptr;
  wxButton *mHistorySearchButton = nullptr;

  // Subscriptions:
  wxBitmapButton *mSubscribe = nullptr;
  Widgets::TopicCtrl *mFilter = nullptr;
  wxObjectDataPtr<Models::Subscriptions> mSubscriptionsModel;
  wxDataViewCtrl *mSubscriptionsCtrl = nullptr;

  // Messages:
  wxObjectDataPtr<Models::Messages> mMessagesModel;
  wxDataViewCtrl *mMessagesCtrl = nullptr;
  std::array<wxDataViewColumn*, Models::Messages::Column::Max> mMessageColumns {};
  bool mMessageExplicitEditRequest = false;
  bool mMessagesWasExpanded = false;
  std::pair<bool, wxDataViewItem> mMessagesPossible;

  std::shared_ptr<MQTT::Client> mClient;
  size_t mMqttObserverId = 0;

  void onClose(wxCloseEvent &event);

  // Connection.
  void allowCancel();
  void allowConnect();
  void allowDisconnect();
  void allowNothing();
  void onCancelClicked(wxCommandEvent &event);
  void onConnectClicked(wxCommandEvent &event);
  void onDisconnectClicked(wxCommandEvent &event);

  // Context.
  void onContextSelected(wxCommandEvent &event);
  void onContextSelectedHistoryEdit(wxCommandEvent &event);
  void onContextSelectedHistoryResend(wxCommandEvent &event);
  void onContextSelectedHistoryRetainedClear(wxCommandEvent &event);
  void onContextSelectedHistorySaveMessage(wxCommandEvent &event);
  void onContextSelectedHistoryCopyTopic(wxCommandEvent &event);
  void onContextSelectedHistoryCopyPayload(wxCommandEvent &event);
  void onContextSelectedMessageDelete(wxCommandEvent &event);
  void onContextSelectedMessageNewFolder(wxCommandEvent &event);
  void onContextSelectedMessageNewMessage(wxCommandEvent &event);
  void onContextSelectedMessageRename(wxCommandEvent &event);
  void onContextSelectedMessagePublish(wxCommandEvent &event);
  void onContextSelectedMessageOverwrite(wxCommandEvent &event);
  void onContextSelectedSubscriptionsChangeColor(wxCommandEvent &event);
  void onContextSelectedSubscriptionsClear(wxCommandEvent &event);
  void onContextSelectedSubscriptionsMute(wxCommandEvent &event);
  void onContextSelectedSubscriptionsSolo(wxCommandEvent &event);
  void onContextSelectedSubscriptionsUnmute(wxCommandEvent &event);
  void onContextSelectedSubscriptionsUnsubscribe(wxCommandEvent &event);
  void onHistoryContext(wxDataViewEvent &event);
  void onMessagesContext(wxDataViewEvent &event);
  void onSubscriptionContext(wxDataViewEvent &event);

  // History.
  void onHistoryClearClicked(wxCommandEvent &event);
  void onHistoryRecordClicked(wxCommandEvent &event);
  void onHistorySelected(wxDataViewEvent &event);
  void onHistoryDoubleClicked(wxDataViewEvent &event);
  void onHistorySearchKey(wxKeyEvent &event);
  void onHistorySearchButton(wxCommandEvent &event);

  // Preview.
  void onPreviewSaveMessage(Events::Edit &event);

  // Publish.
  void onPublishClicked(wxCommandEvent &event);
  void onPublishSaveMessage(Events::Edit &event);

  // Setup.
  void setupPanels();
  void setupPanelConnect(wxWindow *parent);
  void setupPanelHistory(wxWindow *parent);
  void setupPanelPreview(wxWindow *parent);
  void setupPanelPublish(wxWindow *parent);
  void setupPanelMessages(wxWindow *parent);
  void setupPanelSubscriptions(wxWindow *parent);

  // Messages.
  void onMessagesActivated(wxDataViewEvent &event);
  void onMessagesDrag(wxDataViewEvent &event);
  void onMessagesDrop(wxDataViewEvent &event);
  void onMessagesDropPossible(wxDataViewEvent &event);
  void onMessagesEdit(wxDataViewEvent &event);
  void onMessagesChanged(wxDataViewEvent &event);
  void onMessagesSelected(wxDataViewEvent &event);

  // Subscriptions.
  void onSubscribeClicked(wxCommandEvent &event);
  void onSubscribeEnter(wxKeyEvent &event);
  void onSubscriptionSelected(wxDataViewEvent &event);

  // Layouts.
  void onLayoutSelected(Events::Layout &event);
  void onLayoutResized(Events::Layout &event);

  // MQTT::Client::Observer interface.
  void onConnected() override;
  void onDisconnected() override;
  void onConnectionLost() override;
  void onConnectionFailure() override;

  // MQTT::Client::Observer on GUI thread.
  void onConnectedSync(Events::Connection &event);
  void onDisconnectedSync(Events::Connection &event);
  void onConnectionLostSync(Events::Connection &event);
  void onConnectionFailureSync(Events::Connection &event);

  // Models::History::Observer interface.
  void onMessage(wxDataViewItem item) override;

  wxAuiManager mAuiMan;

};

} // namespace Rapatas::Transmitron::GUI::Tabs

