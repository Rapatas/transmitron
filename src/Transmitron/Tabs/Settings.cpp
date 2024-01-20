#include <wx/artprov.h>
#include <wx/colour.h>
#include <wx/button.h>
#include <wx/wx.h>
#include <wx/propgrid/propgrid.h>

#include "Common/Log.hpp"
#include "Settings.hpp"
#include "Transmitron/Models/Layouts.hpp"
#include "Transmitron/Widgets/Container.hpp"
#include "Transmitron/Notifiers/Layouts.hpp"
#include "Transmitron/Events/Connection.hpp"

using namespace Transmitron::Tabs;
using namespace Transmitron;

Settings::Settings(
  wxWindow *parent,
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
    "Settings"
  ),
  mLabelFont(std::move(labelFont)),
  mOptionsHeight(optionsHeight),
  mProfilesModel(profilesModel),
  mLayoutsModel(layoutsModel)
{
  mLogger = Common::Log::create("Transmitron::Settings");

  auto *notifier = new Notifiers::Layouts;
  mLayoutsModel->AddNotifier(notifier);

  notifier->Bind(Events::LAYOUT_ADDED,   &Settings::onLayoutAdded,   this);
  notifier->Bind(Events::LAYOUT_REMOVED, &Settings::onLayoutRemoved, this);
  notifier->Bind(Events::LAYOUT_CHANGED, &Settings::onLayoutChanged, this);

  auto *container = new Widgets::Container(this);
  auto *master = new wxPanel(container);
  container->contain(master);

  mProfiles = new wxPanel(master);

  setupLayouts(master);
  setupProfiles(mProfiles);
  setupProfileButtons(master);
  setupProfileForm(mProfiles);

  auto *hsizer = new wxBoxSizer(wxHORIZONTAL);
  hsizer->Add(mProfilesLeft, 0, wxEXPAND);
  hsizer->Add(mProfileForm, 1, wxEXPAND);
  mProfiles->SetSizer(hsizer);

  auto *vsizer = new wxBoxSizer(wxVERTICAL);
  vsizer->Add(mLayouts, 0, wxEXPAND);
  vsizer->Add(mProfiles, 0, wxEXPAND);
  vsizer->Add(mProfileButtons, 0, wxEXPAND);
  vsizer->Layout();
  master->SetSizer(vsizer);

  auto *sizer = new wxBoxSizer(wxVERTICAL);
  sizer->Add(container, 1, wxEXPAND);
  SetSizer(sizer);

  Bind(wxEVT_COMMAND_MENU_SELECTED, &Settings::onContextSelected, this);
}

void Settings::setupLayouts(wxPanel *parent)
{
  auto *renderer = new wxDataViewTextRenderer(
    wxDataViewTextRenderer::GetDefaultType(),
    wxDATAVIEW_CELL_EDITABLE
  );

  mLayoutColumnName = new wxDataViewColumn(
    L"name",
    renderer,
    (unsigned)Models::Layouts::Column::Name,
    wxCOL_WIDTH_AUTOSIZE,
    wxALIGN_LEFT
  );

  mLayouts = new wxPanel(parent, -1);

  auto *label = new wxStaticText(mLayouts, -1, "Layouts");
  label->SetFont(mLabelFont);

  mLayoutsCtrl = new wxDataViewListCtrl(
    mLayouts,
    -1,
    wxDefaultPosition,
    wxDefaultSize,
    wxDV_NO_HEADER
  );
  mLayoutsCtrl->AppendColumn(mLayoutColumnName);
  mLayoutsCtrl->AssociateModel(mLayoutsModel.get());
  mLayoutsCtrl->Bind(
    wxEVT_DATAVIEW_ITEM_CONTEXT_MENU,
    &Settings::onLayoutsContext,
    this
  );

  auto *vsizer = new wxBoxSizer(wxVERTICAL);
  vsizer->Add(label,        0, wxEXPAND);
  vsizer->Add(mLayoutsCtrl, 1, wxEXPAND);
  mLayouts->SetSizer(vsizer);

  mLayoutsCtrl->Bind(
    wxEVT_DATAVIEW_ITEM_START_EDITING,
    &Settings::onLayoutsEdit,
    this
  );
}

void Settings::setupProfiles(wxPanel *parent)
{
  wxDataViewColumn* const name = new wxDataViewColumn(
    "Name",
    new wxDataViewTextRenderer(),
    (unsigned)Models::Profiles::Column::Name,
    wxCOL_WIDTH_AUTOSIZE,
    wxALIGN_LEFT
  );

  mProfilesLeft = new wxPanel(parent, -1);
  mProfilesLeft->SetMinSize(wxSize(200, 200));

  auto *label = new wxStaticText(mProfilesLeft, -1, "Profiles");
  label->SetFont(mLabelFont);

  mProfilesCtrl = new wxDataViewCtrl(mProfilesLeft, -1);
  mProfilesCtrl->AssociateModel(mProfilesModel.get());
  mProfilesCtrl->AppendColumn(name);
  // mProfilesCtrl->Bind(
  //   wxEVT_DATAVIEW_ITEM_ACTIVATED,
  //   &Settings::onProfileActivated,
  //   this
  //  );
  mProfilesCtrl->Bind(
    wxEVT_DATAVIEW_SELECTION_CHANGED,
    &Settings::onProfileSelected,
    this
   );
  mProfilesCtrl->Bind(
    wxEVT_DATAVIEW_ITEM_CONTEXT_MENU,
    &Settings::onProfileContext,
    this
  );

  auto *vsizer = new wxBoxSizer(wxVERTICAL);
  vsizer->Add(label,         0, wxEXPAND);
  vsizer->Add(mProfilesCtrl, 1, wxEXPAND);
  mProfilesLeft->SetSizer(vsizer);
}

void Settings::setupProfileForm(wxPanel *parent)
{
  mProfileForm = new wxPanel(parent, -1);
  mProfileForm->SetMinSize(wxSize(200, 400));

  auto *label = new wxStaticText(mProfileForm, -1, "Profile Options");
  label->SetFont(mLabelFont);

  mProfileFormGrid = new wxPropertyGrid(
    mProfileForm,
    -1,
    wxDefaultPosition,
    wxDefaultSize,
    wxPG_SPLITTER_AUTO_CENTER
  );

  mProfileFormGrid->DedicateKey(WXK_UP);
  mProfileFormGrid->DedicateKey(WXK_DOWN);

  mProfileFormGrid->Enable(false);

  auto &pfp = mProfileFormProperties;
  auto &pfg = mProfileFormGrid;

  pfp.resize(Properties::Max);

  pfp.at(Properties::Name) =
    pfg->Append(new wxStringProperty("Name", "", {}));

  mGridCategoryBroker = new wxPropertyCategory("Broker");
  mProfileFormGrid->Append(mGridCategoryBroker);

  pfp.at(Properties::Hostname) =
    pfg->AppendIn(mGridCategoryBroker, new wxStringProperty("Hostname", "", {}));
  pfp.at(Properties::Port) =
    pfg->AppendIn(mGridCategoryBroker, new wxUIntProperty("Port", "", {}));
  pfp.at(Properties::Username) =
    pfg->AppendIn(mGridCategoryBroker, new wxStringProperty("Username", "", {}));
  pfp.at(Properties::Password) =
    pfg->AppendIn(mGridCategoryBroker, new wxStringProperty("Password", "", {}));
  pfp.at(Properties::ConnectTimeout) =
    pfg->AppendIn(mGridCategoryBroker, new wxUIntProperty("Connect Timeout (s)", "", {}));
  pfp.at(Properties::DisconnectTimeout) =
    pfg->AppendIn(mGridCategoryBroker, new wxUIntProperty("Disconnect Timeout (s)", "", {}));
  pfp.at(Properties::MaxInFlight) =
    pfg->AppendIn(mGridCategoryBroker, new wxUIntProperty("Max in flight", "", {}));
  pfp.at(Properties::KeepAlive) =
    pfg->AppendIn(mGridCategoryBroker, new wxUIntProperty("Keep alive interval", "", {}));
  pfp.at(Properties::ClientId) =
    pfg->AppendIn(mGridCategoryBroker, new wxStringProperty("Client ID", "", {}));
  pfp.at(Properties::AutoReconnect) =
    pfg->AppendIn(mGridCategoryBroker, new wxBoolProperty("Auto Reconnect", "", {}));
  pfp.at(Properties::MaxReconnectRetries) =
    pfg->AppendIn(mGridCategoryBroker, new wxUIntProperty("Max Reconnect Retries", "", {}));

  mGridCategoryClient = new wxPropertyCategory("Client");
  mProfileFormGrid->Append(mGridCategoryClient);

  const auto layoutLabels = mLayoutsModel->getLabelArray();
  auto *layoutPtr = new wxEnumProperty("Layout", "", layoutLabels);
  pfp.at(Properties::Layout) = pfg->AppendIn(mGridCategoryClient, layoutPtr);

  auto *sizer = new wxBoxSizer(wxVERTICAL);
  sizer->Add(label,            0, wxEXPAND);
  sizer->Add(mProfileFormGrid, 1, wxEXPAND);
  mProfileForm->SetSizer(sizer);

  mProfileFormGrid->Bind(wxEVT_PG_CHANGED, &Settings::onGridChanged, this);
  mProfileFormGrid->Bind(wxEVT_PG_CHANGING, &Settings::onGridChanged, this);
  mProfileFormGrid->Bind(wxEVT_PG_LABEL_EDIT_BEGIN, &Settings::onGridChanged, this);
  mProfileFormGrid->Bind(wxEVT_PG_SELECTED, &Settings::onGridChanged, this);

  propertyGridClear();
}

void Settings::setupProfileButtons(wxPanel *parent)
{
  mProfileButtons = new wxPanel(parent);

  auto *newProfile = new wxButton(
    mProfileButtons,
    -1,
    "New Profile",
    wxDefaultPosition,
    wxSize(-1, mOptionsHeight)
  );
  newProfile->SetBitmap(wxArtProvider::GetBitmap(wxART_NEW));
  newProfile->Bind(wxEVT_BUTTON, &Settings::onButtonClickedNewProfile, this);

  mSave = new wxButton(
    mProfileButtons,
    -1,
    "Save",
    wxDefaultPosition,
    wxSize(-1, mOptionsHeight)
  );
  mSave->SetBitmap(wxArtProvider::GetBitmap(wxART_FILE_SAVE));
  mSave->Enable(false);
  mSave->Bind(wxEVT_BUTTON, &Settings::onButtonClickedSave, this);

  mCancel = new wxButton(
    mProfileButtons,
    -1,
    "Cancel",
    wxDefaultPosition,
    wxSize(-1, mOptionsHeight)
  );
  mCancel->SetBitmap(wxArtProvider::GetBitmap(wxART_UNDO));
  mCancel->Enable(false);
  mCancel->Bind(wxEVT_BUTTON, &Settings::onButtonClickedCancel, this);

  mConnect = new wxButton(
    mProfileButtons,
    -1,
    "Connect",
    wxDefaultPosition,
    wxSize(-1, mOptionsHeight)
  );
  mConnect->Enable(false);
  mConnect->Bind(wxEVT_BUTTON, &Settings::onButtonClickedConnect, this);
  mConnect->SetBitmap(wxArtProvider::GetBitmap(wxART_TICK_MARK));

  mProfileButtonsSizer = new wxBoxSizer(wxHORIZONTAL);
  mProfileButtonsSizer->SetMinSize(0, mOptionsHeight);
  mProfileButtonsSizer->Add(newProfile, 0, wxEXPAND);
  mProfileButtonsSizer->AddStretchSpacer(1);
  mProfileButtonsSizer->Add(mConnect, 0, wxEXPAND);
  mProfileButtonsSizer->Add(mCancel, 0, wxEXPAND);
  mProfileButtonsSizer->Add(mSave, 0, wxEXPAND);
  mProfileButtonsSizer->Hide(mSave);
  mProfileButtonsSizer->Hide(mCancel);
  mProfileButtons->SetSizer(mProfileButtonsSizer);
}

void Settings::onLayoutsContext(wxDataViewEvent &event)
{
  const auto item = event.GetItem();
  if (!item.IsOk())
  {
    event.Skip();
    return;
  }

  if (mLayoutsModel->getDefault() == item)
  {
    event.Skip();
    return;
  }

  wxMenu menu;

  auto *rename = new wxMenuItem(
    nullptr,
    (unsigned)ContextIDs::LayoutsRename,
    "Rename"
  );
  rename->SetBitmap(wxArtProvider::GetBitmap(wxART_EDIT));
  menu.Append(rename);

  auto *del = new wxMenuItem(
    nullptr,
    (unsigned)ContextIDs::LayoutsDelete,
    "Delete"
  );
  del->SetBitmap(wxArtProvider::GetBitmap(wxART_DELETE));
  menu.Append(del);

  PopupMenu(&menu);
}

void Settings::onContextSelected(wxCommandEvent &event)
{
  switch ((ContextIDs)event.GetId())
  {
    case ContextIDs::LayoutsDelete: onLayoutsDelete(event); break;
    case ContextIDs::LayoutsRename: onLayoutsRename(event); break;
    case ContextIDs::ProfilesDelete: onProfileDelete(event); break;
  }
  event.Skip();
}

void Settings::onLayoutsDelete(wxCommandEvent &/* event */)
{
  mLogger->info("Requesting delete");
  const auto item = mLayoutsCtrl->GetSelection();
  if (!item.IsOk()) { return; }
  if (mLayoutsModel->getDefault() == item) { return; }
  mLayoutsModel->remove(item);
}

void Settings::onLayoutsRename(wxCommandEvent &/* event */)
{
  mLogger->info("Requesting rename");
  const auto item = mLayoutsCtrl->GetSelection();
  if (!item.IsOk()) { return; }

  mLayoutsCtrl->EditItem(item, mLayoutColumnName);
}

void Settings::onLayoutsEdit(wxDataViewEvent &event)
{
  mLogger->info("Requesting edit");
  const auto item = event.GetItem();
  if (mLayoutsModel->getDefault() == item)
  {
    event.Veto();
  }
  else
  {
    event.Skip();
  }
}

void Settings::onProfileContext(wxDataViewEvent &event)
{
  if (!event.GetItem().IsOk())
  {
    event.Skip();
    return;
  }

  wxMenu menu;

  auto *del = new wxMenuItem(
    nullptr,
    (unsigned)ContextIDs::ProfilesDelete,
    "Delete"
  );
  del->SetBitmap(wxArtProvider::GetBitmap(wxART_DELETE));
  menu.Append(del);

  PopupMenu(&menu);
}

void Settings::onProfileDelete(wxCommandEvent & /* event */)
{
  const auto item = mProfilesCtrl->GetSelection();
  mProfilesModel->remove(item);
  propertyGridClear();
}

void Settings::onProfileSelected(wxDataViewEvent &event)
{
  const auto item = event.GetItem();
  if (!item.IsOk())
  {
    event.Skip();
    return;
  }

  propertyGridFill(
    mProfilesModel->getName(item),
    mProfilesModel->getBrokerOptions(item),
    mProfilesModel->getClientOptions(item)
  );
  event.Skip();
}

void Settings::onGridChanged(wxPropertyGridEvent& event)
{
  allowSave();
  event.Skip();
}

void Settings::onButtonClickedNewProfile(wxCommandEvent &event)
{
  (void)event;

  const auto item = mProfilesModel->createProfile();
  mProfilesCtrl->Select(item);
  mProfilesCtrl->EnsureVisible(item);
  propertyGridFill(
    mProfilesModel->getName(item),
    mProfilesModel->getBrokerOptions(item),
    mProfilesModel->getClientOptions(item)
  );

  const auto *namePtr = mProfileFormProperties.at(Properties::Name);
  const bool focus = true;
  mProfileFormGrid->SelectProperty(namePtr, focus);

  allowSave();
}

void Settings::onButtonClickedCancel(wxCommandEvent &event)
{
  (void)event;

  propertyGridClear();

  const auto item = mProfilesCtrl->GetSelection();
  if (!item.IsOk()) { return; }

  propertyGridFill(
    mProfilesModel->getName(item),
    mProfilesModel->getBrokerOptions(item),
    mProfilesModel->getClientOptions(item)
  );

  allowConnect();
}

void Settings::onButtonClickedSave(wxCommandEvent &event)
{
  (void)event;

  const auto item = mProfilesCtrl->GetSelection();

  if (!item.IsOk())
  {
    return;
  }

  const auto wxs = mProfileFormProperties.at(Properties::Name)->GetValue().GetString();
  const auto utf8 = wxs.ToUTF8();
  const std::string name(utf8.data(), utf8.length());
  mProfilesModel->rename(item, name);

  const auto brokerOptions = brokerOptionsFromPropertyGrid();
  mProfilesModel->updateBrokerOptions(item, brokerOptions);

  const auto clientOptions = clientOptionsFromPropertyGrid();
  mProfilesModel->updateClientOptions(item, clientOptions);

  allowConnect();
}

void Settings::onButtonClickedConnect(wxCommandEvent &event)
{
  (void)event;

  const auto item = mProfilesCtrl->GetSelection();
  if (!item.IsOk())
  {
    return;
  }

  const auto &brokerOptions = mProfilesModel->getBrokerOptions(item);

  mLogger->info(
    "Queueing event for profile at {}:{}",
    brokerOptions.getHostname(),
    brokerOptions.getPort()
  );

  auto *connectionEvent = new Events::Connection();
  connectionEvent->setProfile(item);
  wxQueueEvent(this, connectionEvent);
}

void Settings::allowSave()
{
  mSave->Enable(true);
  mCancel->Enable(true);
  mProfileButtonsSizer->Show(mSave);
  mProfileButtonsSizer->Show(mCancel);

  mConnect->Enable(false);
  mProfileButtonsSizer->Hide(mConnect);

  mProfileButtonsSizer->Layout();
}

void Settings::allowConnect()
{
  mSave->Enable(false);
  mCancel->Enable(false);
  mProfileButtonsSizer->Hide(mSave);
  mProfileButtonsSizer->Hide(mCancel);

  mConnect->Enable(true);
  mProfileButtonsSizer->Show(mConnect);

  mProfileButtonsSizer->Layout();
}

void Settings::propertyGridClear()
{
  mSave->Enable(false);
  mConnect->Enable(false);
  mProfileFormGrid->Enable(false);

  auto &pfp = mProfileFormProperties;

  pfp.at(Properties::Name)->SetValue({});
  pfp.at(Properties::AutoReconnect)->SetValue({});
  pfp.at(Properties::ClientId)->SetValue({});
  pfp.at(Properties::ConnectTimeout)->SetValue({});
  pfp.at(Properties::DisconnectTimeout)->SetValue({});
  pfp.at(Properties::Hostname)->SetValue({});
  pfp.at(Properties::KeepAlive)->SetValue({});
  pfp.at(Properties::MaxInFlight)->SetValue({});
  pfp.at(Properties::MaxReconnectRetries)->SetValue({});
  pfp.at(Properties::Password)->SetValue({});
  pfp.at(Properties::Port)->SetValue({});
  pfp.at(Properties::Username)->SetValue({});
  pfp.at(Properties::Layout)->SetValue({});
}

void Settings::propertyGridFill(
  const wxString &name,
  const MQTT::BrokerOptions &brokerOptions,
  const Types::ClientOptions &clientOptions
) {
  auto &pfp = mProfileFormProperties;

  pfp.at(Properties::Name)->SetValue(name);

  pfp.at(Properties::AutoReconnect)->SetValue(brokerOptions.getAutoReconnect());
  pfp.at(Properties::ClientId)->SetValue(brokerOptions.getClientId());
  pfp.at(Properties::ConnectTimeout)->SetValue((int)brokerOptions.getConnectTimeout().count());
  pfp.at(Properties::DisconnectTimeout)->SetValue((int)brokerOptions.getDisconnectTimeout().count());
  pfp.at(Properties::Hostname)->SetValue(brokerOptions.getHostname());
  pfp.at(Properties::KeepAlive)->SetValue((int)brokerOptions.getKeepAliveInterval().count());
  pfp.at(Properties::MaxInFlight)->SetValue((int)brokerOptions.getMaxInFlight());
  pfp.at(Properties::MaxReconnectRetries)->SetValue((int)brokerOptions.getMaxReconnectRetries());
  pfp.at(Properties::Password)->SetValue(brokerOptions.getPassword());
  pfp.at(Properties::Port)->SetValue((int)brokerOptions.getPort());
  pfp.at(Properties::Username)->SetValue(brokerOptions.getUsername());

  (void)clientOptions;
  auto &pfpLayout = pfp.at(Properties::Layout);
  wxVariant layoutValue = pfpLayout->GetValue();
  const bool hasValue = pfpLayout->StringToValue(layoutValue, clientOptions.getLayout());
  if (hasValue)
  {
    pfpLayout->SetValue(layoutValue);
  }

  mSave->Enable(true);
  mConnect->Enable(true);
  mProfileFormGrid->Enable(true);
}

MQTT::BrokerOptions Settings::brokerOptionsFromPropertyGrid() const
{
  const auto &pfp = mProfileFormProperties;

  const bool autoReconnect       = pfp.at(Properties::AutoReconnect)->GetValue();
  const auto maxInFlight         = (size_t)pfp.at(Properties::MaxInFlight)->GetValue().GetInteger();
  const auto maxReconnectRetries = (size_t)pfp.at(Properties::MaxReconnectRetries)->GetValue().GetInteger();
  const auto port                = (size_t)pfp.at(Properties::Port)->GetValue().GetInteger();
  const auto connectTimeout      = (size_t)pfp.at(Properties::ConnectTimeout)->GetValue().GetInteger();
  const auto disconnectTimeout   = (size_t)pfp.at(Properties::DisconnectTimeout)->GetValue().GetInteger();
  const auto keepAliveInterval   = (size_t)pfp.at(Properties::KeepAlive)->GetValue().GetLong();
  const auto clientId            = pfp.at(Properties::ClientId)->GetValue();
  const auto hostname            = pfp.at(Properties::Hostname)->GetValue();
  const auto password            = pfp.at(Properties::Password)->GetValue();
  const auto username            = pfp.at(Properties::Username)->GetValue();

  return MQTT::BrokerOptions {
    autoReconnect,
    maxInFlight,
    maxReconnectRetries,
    port,
    std::chrono::seconds(connectTimeout),
    std::chrono::seconds(disconnectTimeout),
    std::chrono::seconds(keepAliveInterval),
    clientId,
    hostname,
    password,
    username,
  };
}

Types::ClientOptions Settings::clientOptionsFromPropertyGrid() const
{
  const auto &pfp = mProfileFormProperties;

  const auto &pfpLayout = pfp.at(Properties::Layout);
  const auto layoutValue = pfpLayout->GetValue();
  const auto layoutIndex = (size_t)layoutValue.GetInteger();
  const auto layout = mLayoutsModel->getLabelArray()[layoutIndex];

  return Types::ClientOptions {
    layout.ToStdString(),
  };
}

void Settings::onLayoutAdded(Events::Layout &/* event */)
{
  refreshLayouts();
}

void Settings::onLayoutRemoved(Events::Layout &/* event */)
{
  refreshLayouts();
}

void Settings::onLayoutChanged(Events::Layout &/* event */)
{
  auto &pfp = mProfileFormProperties;
  auto &pfg = mProfileFormGrid;

  auto &pfpLayout = pfp.at(Properties::Layout);
  wxVariant layoutValue = pfpLayout->GetValue();

  pfg->RemoveProperty(pfpLayout);

  const auto layoutLabels = mLayoutsModel->getLabelArray();
  auto *layoutPtr = new wxEnumProperty("Layout", "", layoutLabels);
  pfpLayout = pfg->AppendIn(mGridCategoryClient, layoutPtr);

  pfpLayout->SetValue(layoutValue);
}

void Settings::refreshLayouts()
{
  auto &pfp = mProfileFormProperties;
  auto &pfg = mProfileFormGrid;

  auto &pfpLayout = pfp.at(Properties::Layout);
  wxVariant layoutValue = pfpLayout->GetValue();
  const auto layoutName = pfpLayout->ValueToString(layoutValue);
  mLogger->info("Previous layout was: {}", layoutName);

  pfg->RemoveProperty(pfpLayout);

  const auto layoutLabels = mLayoutsModel->getLabelArray();
  auto *layoutPtr = new wxEnumProperty("Layout", "", layoutLabels);
  pfpLayout = pfg->AppendIn(mGridCategoryClient, layoutPtr);

  wxVariant newLayoutValue = pfpLayout->GetValue();
  const bool hasValue = pfpLayout->StringToValue(newLayoutValue, layoutName);
  if (!hasValue)
  {
    mLogger->info("Previous layout not found");
    wxVariant layoutValue = pfpLayout->GetValue();
    const wxString layoutName = std::string(Models::Layouts::DefaultName);
    pfpLayout->StringToValue(layoutValue, layoutName);
    pfpLayout->SetValue(layoutValue);
    return;
  }

  mLogger->info("Previous layout was found ");
  pfpLayout->SetValue(newLayoutValue);
}
