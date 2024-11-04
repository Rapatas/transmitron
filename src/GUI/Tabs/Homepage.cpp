#include "Homepage.hpp"

#include <chrono>

#include <wx/artprov.h>
#include <wx/button.h>
#include <wx/propgrid/propgrid.h>
#include <wx/statline.h>
#include <wx/wx.h>

#include "Common/Log.hpp"
#include "Common/Version.hpp"
#include "GUI/Events/Connection.hpp"
#include "GUI/Events/Profile.hpp"
#include "GUI/Events/Recording.hpp"
#include "GUI/Models/Layouts.hpp"

using namespace Rapatas::Transmitron;
using namespace GUI::Tabs;
using namespace GUI;
using namespace GUI::Events;
using namespace Common;

constexpr size_t HomepageWidth = 700;
constexpr int HomepageHeight = 700;
constexpr size_t Margin = 10;

Homepage::Homepage(
  wxWindow *parent,
  const ArtProvider &artProvider,
  wxFontInfo labelFont,
  int optionsHeight,
  const wxObjectDataPtr<Models::Profiles> &profilesModel,
  const wxObjectDataPtr<Models::Layouts> &layoutsModel
) :
  wxPanel(
    parent,
    -1,
    wxDefaultPosition,
    wxDefaultSize,
    wxTAB_TRAVERSAL,
    "Homepage"
  ),
  mLogger(Log::create("GUI::Homepage")),
  mLabelFont(std::move(labelFont)),
  mOptionsHeight(optionsHeight),
  mArtProvider(artProvider),
  mProfilesModel(profilesModel),
  mProfilesModelWrapper(new Models::ProfilesWrapper(mProfilesModel)),
  mLayoutsModel(layoutsModel) //
{
  auto *master = new wxPanel(this);
  master->SetMinSize(wxSize(HomepageWidth, HomepageHeight));

  const auto versionStr = fmt::format(
    "Version: {}",
    Info::getProjectVersion()
  );
  auto *version = new wxStaticText(
    this,
    wxID_ANY,
    versionStr,
    wxDefaultPosition,
    wxDefaultSize,
    wxALIGN_CENTRE_HORIZONTAL
  );

  setupProfiles(master);
  setupQuickConnect(master);
  setupRecordings(master);

  auto *rsizer = new wxBoxSizer(wxVERTICAL);
  rsizer->Add(mQuickConnect, 0, wxEXPAND);
  rsizer->AddSpacer(Margin);
  rsizer->Add(mRecordings, 0, wxEXPAND);

  auto *hsizer = new wxBoxSizer(wxHORIZONTAL);
  hsizer->Add(mProfiles, 1, wxEXPAND);
  hsizer->AddSpacer(Margin);
  hsizer->Add(rsizer, 1, wxEXPAND);
  master->SetSizer(hsizer);

  auto *vsizer = new wxBoxSizer(wxVERTICAL);
  vsizer->Add(master, 1);
  vsizer->AddSpacer(Margin);
  vsizer->Add(version, 0, wxEXPAND);

  auto *sizer = new wxBoxSizer(wxHORIZONTAL);
  sizer->AddStretchSpacer(1);
  sizer->Add(vsizer, 0, wxEXPAND);
  sizer->AddStretchSpacer(1);
  SetSizer(sizer);

  Bind(wxEVT_COMMAND_MENU_SELECTED, &Homepage::onContextSelected, this);
}

void Homepage::focus() {
  mProfilesModelWrapper.reset(new Models::ProfilesWrapper(mProfilesModel));
  mProfilesCtrl->AssociateModel(mProfilesModelWrapper.get());

  wxDataViewItemArray children;
  mProfilesModelWrapper->GetChildren(wxDataViewItem(nullptr), children);
  if (children.empty()) { return; }

  const auto first = children.front();
  mProfileEdit->Enable();
  mProfileConnect->Enable();
  mProfilesCtrl->Select(first);
  mProfilesCtrl->SetFocus();
}

void Homepage::setupProfiles(wxPanel *parent) {
  auto *const name = new wxDataViewColumn(
    "Name",
    new wxDataViewIconTextRenderer(),
    static_cast<unsigned>(Models::Profiles::Column::Name),
    wxCOL_WIDTH_AUTOSIZE,
    wxALIGN_LEFT
  );
  auto *const url = new wxDataViewColumn(
    "Address",
    new wxDataViewIconTextRenderer(),
    static_cast<unsigned>(Models::Profiles::Column::URL),
    wxCOL_WIDTH_AUTOSIZE,
    wxALIGN_LEFT
  );

  mProfiles = new wxPanel(parent, -1);
  mProfiles->SetMinSize(wxSize(0, HomepageHeight));

  mProfilesCtrl = new wxDataViewCtrl(
    mProfiles,
    -1,
    wxDefaultPosition,
    wxDefaultSize,
    wxDV_NO_HEADER
  );
  mProfilesCtrl->AssociateModel(mProfilesModelWrapper.get());
  mProfilesCtrl->AppendColumn(name);
  mProfilesCtrl->AppendColumn(url);
  mProfilesCtrl->Bind(
    wxEVT_DATAVIEW_ITEM_ACTIVATED,
    &Homepage::onProfileActivated,
    this //
  );
  mProfilesCtrl->Bind(
    wxEVT_DATAVIEW_SELECTION_CHANGED,
    &Homepage::onProfileSelected,
    this
  );
  mProfilesCtrl->Bind(
    wxEVT_DATAVIEW_ITEM_CONTEXT_MENU,
    &Homepage::onProfileContext,
    this //
  );

  auto *label = new wxStaticText(mProfiles, -1, "Profiles");
  label->SetFont(mLabelFont);

  mProfileCreate = new wxButton(
    mProfiles,
    wxID_ANY,
    "",
    wxDefaultPosition,
    wxSize(mOptionsHeight, mOptionsHeight)
  );
  mProfileCreate->SetToolTip("Create a new profile");
  mProfileCreate->SetBitmap(mArtProvider.bitmap(Icon::NewProfile));
  mProfileCreate->Bind(wxEVT_BUTTON, &Homepage::onProfileCreate, this);

  mProfileEdit = new wxButton(
    mProfiles,
    wxID_ANY,
    "",
    wxDefaultPosition,
    wxSize(mOptionsHeight, mOptionsHeight)
  );
  mProfileEdit->SetToolTip("Edit selected profile");
  mProfileEdit->SetBitmap(mArtProvider.bitmap(Icon::Edit));
  mProfileEdit->Bind(wxEVT_BUTTON, &Homepage::onProfileEdit, this);
  mProfileEdit->Disable();

  mProfileConnect = new wxButton(
    mProfiles,
    wxID_ANY,
    "",
    wxDefaultPosition,
    wxSize(mOptionsHeight, mOptionsHeight)
  );
  mProfileConnect->SetToolTip("Connect to selected profile");
  mProfileConnect->SetBitmap(mArtProvider.bitmap(Icon::Connect));
  mProfileConnect->Bind(wxEVT_BUTTON, &Homepage::onProfileConnect, this);
  mProfileConnect->Disable();

  auto *rsizer = new wxBoxSizer(wxHORIZONTAL);
  rsizer->Add(label, 0);
  rsizer->AddStretchSpacer(1);
  rsizer->Add(mProfileCreate, 0);
  rsizer->Add(mProfileEdit, 0);
  rsizer->Add(mProfileConnect, 0);

  auto *vsizer = new wxBoxSizer(wxVERTICAL);
  vsizer->Add(rsizer, 0, wxEXPAND);
  vsizer->Add(mProfilesCtrl, 1, wxEXPAND);
  mProfiles->SetSizer(vsizer);
}

void Homepage::setupQuickConnect(wxPanel *parent) {
  mQuickConnect = new wxPanel(parent);

  auto item = mProfilesModel->getQuickConnect();
  const auto &options = mProfilesModel->getBrokerOptions(item);
  const auto hint = fmt::format(
    "{}:{}",
    options.getHostname(),
    options.getPort()
  );

  mQuickConnectUrl = new wxTextCtrl(mQuickConnect, wxID_ANY);
  mQuickConnectUrl->SetHint(hint);
  mQuickConnectUrl->Bind(wxEVT_KEY_DOWN, [&](wxKeyEvent &event) {
    const auto isEnter = event.GetKeyCode() == WXK_RETURN;
    if (isEnter) { onQuickConnect(); }
    event.Skip();
  });
  mQuickConnectBtn = new wxButton(
    mQuickConnect,
    wxID_ANY,
    "Connect",
    wxDefaultPosition,
    wxSize(-1, mOptionsHeight)
  );
  mQuickConnectBtn->SetBitmap(mArtProvider.bitmap(Icon::Connect));
  mQuickConnectBtn->SetToolTip("Connect to broker with default settings");
  mQuickConnectBtn->Bind(wxEVT_BUTTON, [&](wxCommandEvent &event) {
    onQuickConnect();
    event.Skip();
  });

  auto *label = new wxStaticText(mQuickConnect, -1, "Quick Connect");
  label->SetFont(mLabelFont);
  label->SetMinSize(wxSize(-1, mOptionsHeight));

  auto *hsizer = new wxBoxSizer(wxHORIZONTAL);
  hsizer->Add(mQuickConnectBtn, 0, wxEXPAND);
  hsizer->Add(mQuickConnectUrl, 1, wxEXPAND);

  auto *vsizer = new wxBoxSizer(wxVERTICAL);
  vsizer->Add(label, 0, wxEXPAND);
  vsizer->Add(hsizer, 0, wxEXPAND);
  mQuickConnect->SetSizer(vsizer);
}

void Homepage::setupRecordings(wxPanel *parent) {
  mRecordings = new wxPanel(parent, -1);

  auto *label = new wxStaticText(mRecordings, -1, "Recordings");
  label->SetFont(mLabelFont);

  auto *recordingOpen = new wxButton(
    mRecordings,
    -1,
    "Browse...",
    wxDefaultPosition,
    wxSize(-1, mOptionsHeight)
  );
  recordingOpen->SetToolTip("Open a .tmrc file to review MQTT history");
  recordingOpen->Bind(wxEVT_BUTTON, &Homepage::onRecordingOpen, this);
  recordingOpen->SetBitmap(mArtProvider.bitmap(Icon::History));

  auto *vsizer = new wxBoxSizer(wxVERTICAL);
  vsizer->Add(label, 0, wxEXPAND);
  vsizer->Add(recordingOpen, 0);
  mRecordings->SetSizer(vsizer);
}

void Homepage::onProfileActivated(wxDataViewEvent &event) {
  const auto &profileItem = event.GetItem();
  connectTo(profileItem);
  event.Skip();
}

void Homepage::onProfileSelected(wxDataViewEvent &event) {
  const auto item = event.GetItem();
  if (!item.IsOk()) {
    mProfileEdit->Disable();
    mProfileConnect->Disable();
    event.Skip();
    return;
  }

  mProfileEdit->Enable();
  mProfileConnect->Enable();

  event.Skip();
}

void Homepage::onRecordingOpen(wxCommandEvent &event) {
  auto *recordingEvent = new Events::Recording(RECORDING_OPEN);
  wxQueueEvent(this, recordingEvent);
  event.Skip();
}

void Homepage::onConnectClicked(wxCommandEvent & /* event */) {
  const auto item = mProfilesCtrl->GetSelection();
  if (!item.IsOk()) { return; }

  const auto &brokerOptions = mProfilesModel->getBrokerOptions(item);

  mLogger->info(
    "Queueing event for profile at {}:{}",
    brokerOptions.getHostname(),
    brokerOptions.getPort()
  );

  auto *connectionEvent = new Events::Connection(Events::CONNECTION_REQUESTED);
  connectionEvent->setProfile(item);
  wxQueueEvent(this, connectionEvent);
}

void Homepage::onProfileContext(wxDataViewEvent &event) {
  wxMenu menu;

  if (event.GetItem().IsOk()) {
    auto *connect = new wxMenuItem(
      nullptr,
      static_cast<unsigned>(ContextIDs::ProfilesConnect),
      "Connect"
    );
    connect->SetBitmap(mArtProvider.bitmap(Icon::Connect));
    menu.Append(connect);

    auto *edit = new wxMenuItem(
      nullptr,
      static_cast<unsigned>(ContextIDs::ProfilesEdit),
      "Edit"
    );
    edit->SetBitmap(mArtProvider.bitmap(Icon::Edit));
    menu.Append(edit);

  } else {
    auto *create = new wxMenuItem(
      nullptr,
      static_cast<unsigned>(ContextIDs::ProfilesCreate),
      "Create profile"
    );
    create->SetBitmap(mArtProvider.bitmap(Icon::NewProfile));
    menu.Append(create);
  }

  PopupMenu(&menu);
}

void Homepage::onContextSelected(wxCommandEvent &event) {
  switch (static_cast<ContextIDs>(event.GetId())) {
    case ContextIDs::ProfilesConnect: onProfileConnect(event); break;
    case ContextIDs::ProfilesCreate: onProfileCreate(event); break;
    case ContextIDs::ProfilesEdit: onProfileEdit(event); break;
  }
  event.Skip();
}

void Homepage::onProfileCreate(wxCommandEvent & /* event */) {
  auto *newEvent = new Events::Profile(PROFILE_CREATE);
  wxQueueEvent(this, newEvent);
}

void Homepage::onProfileEdit(wxCommandEvent &event) {
  (void)event;
  auto *newEvent = new Events::Profile(PROFILE_EDIT);
  newEvent->setProfile(mProfilesCtrl->GetSelection());
  wxQueueEvent(this, newEvent);
}

void Homepage::onProfileConnect(wxCommandEvent & /* event */) {
  connectTo(mProfilesCtrl->GetSelection());
}

void Homepage::onQuickConnect() {
  const auto wxs = mQuickConnectUrl->GetValue();
  const auto utf8 = wxs.ToUTF8();
  const std::string url(utf8.data(), utf8.length());

  if (!url.empty()) { mProfilesModel->updateQuickConnect(url); }

  const auto &profileItem = mProfilesModel->getQuickConnect();
  const auto &brokerOptions = mProfilesModel->getBrokerOptions(profileItem);
  mLogger->info(
    "Queueing event for profile at {}:{}",
    brokerOptions.getHostname(),
    brokerOptions.getPort()
  );

  auto *connectionEvent = new Events::Connection(Events::CONNECTION_REQUESTED);
  connectionEvent->setProfile(profileItem);
  wxQueueEvent(this, connectionEvent);
}

void Homepage::connectTo(wxDataViewItem profile) {
  const auto &brokerOptions = mProfilesModel->getBrokerOptions(profile);
  mLogger->info(
    "Queueing event for profile at {}:{}",
    brokerOptions.getHostname(),
    brokerOptions.getPort()
  );

  auto *connectionEvent = new Events::Connection(Events::CONNECTION_REQUESTED);
  connectionEvent->setProfile(profile);
  wxQueueEvent(this, connectionEvent);
}
