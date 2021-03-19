#ifndef TRANSMITRON_TABS_CLIENT_HPP
#define TRANSMITRON_TABS_CLIENT_HPP

#include <wx/listctrl.h>
#include <wx/splitter.h>
#include <wx/aui/aui.h>
#include <wx/tglbtn.h>

#include "MQTT/Client.hpp"
#include "Transmitron/Events/Connection.hpp"
#include "Transmitron/Widgets/TopicCtrl.hpp"
#include "Transmitron/Models/History.hpp"
#include "Transmitron/Models/Subscriptions.hpp"
#include "Transmitron/Models/Snippets.hpp"
#include "Transmitron/Widgets/Edit.hpp"
#include "Transmitron/ValueObjects/BrokerOptions.hpp"

namespace Transmitron::Tabs
{

class Client :
  public wxPanel,
  public Models::History::Observer,
  public MQTT::Client::Observer
{
public:

  Client(
    wxWindow* parent,
    ValueObjects::BrokerOptions brokerOptions,
    wxObjectDataPtr<Models::Snippets> snippetsModel
  );
  ~Client();

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
    HistorySaveSnippet,
    SnippetRename,
    SnippetDelete,
    SnippetNewSnippet,
    SnippetNewFolder,
  };

  enum class Panes : unsigned
  {
    History = 0,
    Preview = 4,
    Publish = 3,
    Snippets = 2,
    Subscriptions = 1,
  };

  struct Pane
  {
    std::string name;
    wxAuiPaneInfo info;
    wxPanel *panel;
    wxBitmap *icon18x18;
    wxBitmap *icon18x14;
    wxButton *toggle;
  };

  std::map<Panes, Pane> mPanes;

  static const size_t OptionsHeight = 26;

  const ValueObjects::BrokerOptions mBrokerOptions;

  wxBoxSizer *mMasterSizer;

  // Profile:
  wxPanel *mProfileBar;
  wxButton *mConnect;

  // History:
  wxObjectDataPtr<Models::History> mHistoryModel;
  wxDataViewCtrl *mHistoryCtrl;
  wxCheckBox *mAutoScroll;
  wxButton *mHistoryClear;

  // Subscriptions:
  wxBitmapButton *mSubscribe;
  Widgets::TopicCtrl *mFilter;
  wxObjectDataPtr<Models::Subscriptions> mSubscriptionsModel;
  wxDataViewCtrl *mSubscriptionsCtrl;

  // Snippets:
  wxObjectDataPtr<Models::Snippets> mSnippetsModel;
  wxDataViewCtrl *mSnippetsCtrl;
  std::array<wxDataViewColumn*, Models::Snippets::Column::Max> mSnippetColumns;
  bool mSnippetExplicitEditRequest = false;
  bool mSnippetsWasExpanded = false;
  std::pair<bool, wxDataViewItem> mSnippetsPossible;

  std::shared_ptr<MQTT::Client> mClient;

  void setupPanelConnect(wxWindow *parent);
  void setupPanelSubscriptions(wxWindow *parent);
  void setupPanelHistory(wxWindow *parent);
  void setupPanelPreview(wxWindow *parent);
  void setupPanelPublish(wxWindow *parent);
  void setupPanelSnippets(wxWindow *parent);

  void onClose(wxCloseEvent &event);
  void onConnectClicked(wxCommandEvent &event);
  void onHistorySelected(wxDataViewEvent& event);
  void onPublishClicked(wxCommandEvent &event);
  void onPublishSaveSnippet(Events::Edit &e);
  void onPreviewSaveSnippet(Events::Edit &e);
  void onSubscribeClicked(wxCommandEvent &event);
  void onSubscribeEnter(wxKeyEvent &event);
  void onSubscriptionContext(wxDataViewEvent& event);
  void onHistoryContext(wxDataViewEvent& event);
  void onContextSelected(wxCommandEvent& event);
  void onSubscriptionSelected(wxDataViewEvent &event);
  void onSnippetsEdit(wxDataViewEvent &e);
  void onSnippetsContext(wxDataViewEvent &e);
  void onSnippetsSelected(wxDataViewEvent &e);
  void onSnippetsActivated(wxDataViewEvent &e);
  void onSnippetsDrag(wxDataViewEvent &e);
  void onSnippetsDrop(wxDataViewEvent &e);
  void onSnippetsDropPossible(wxDataViewEvent &e);

  void onConnectedSync(Events::Connection &e);
  void onDisconnectedSync(Events::Connection &e);

  void handleInserted(wxDataViewItem &inserted);

  // MQTT::Client::Observer interface.
  void onConnected() override;
  void onDisconnected() override;

  // Models::History::Observer interface.
  void onMessage(wxDataViewItem item) override;

  wxAuiManager mAuiMan;

};

}

#endif // TRANSMITRON_TABS_CLIENT_HPP
