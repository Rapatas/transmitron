#include "Common/Filesystem.hpp"
#include <fstream>
#include <stdexcept>

#include <fmt/core.h>
#include <wx/aui/auibook.h>
#include <wx/image.h>
#include <wx/notebook.h>
#include <wx/settings.h>

#include "App.hpp"
#include "Common/Helpers.hpp"
#include "Common/Log.hpp"
#include "Common/Url.hpp"
#include "Common/XdgBaseDir.hpp"
#include "Events/Connection.hpp"
#include "Common/Info.hpp"
#include "MQTT/Client.hpp"
#include "Resources/gear/gear-18x18.hpp"
#include "Resources/history/history-18x14.hpp"
#include "Resources/history/history-18x18.hpp"
#include "Resources/pin/not-pinned-18x18.hpp"
#include "Resources/pin/pinned-18x18.hpp"
#include "Resources/plus/plus-18x18.hpp"
#include "Resources/preview/preview-18x14.hpp"
#include "Resources/preview/preview-18x18.hpp"
#include "Resources/qos/qos-0.hpp"
#include "Resources/qos/qos-1.hpp"
#include "Resources/qos/qos-2.hpp"
#include "Resources/send/send-18x14.hpp"
#include "Resources/send/send-18x18.hpp"
#include "Resources/messages/messages-18x14.hpp"
#include "Resources/messages/messages-18x18.hpp"
#include "Resources/subscription/subscription-18x14.hpp"
#include "Resources/subscription/subscription-18x18.hpp"
#include "Tabs/Client.hpp"
#include "Tabs/Homepage.hpp"
#include "Tabs/Settings.hpp"
#include "GUI/Events/Profile.hpp"
#include "GUI/Events/Recording.hpp"
#include "GUI/Models/History.hpp"
#include "GUI/Models/Layouts.hpp"
#include "GUI/Models/Subscriptions.hpp"

using namespace Rapatas::Transmitron;
using namespace GUI;
using namespace Common;

constexpr size_t DefaultWindowWidth = 600;
constexpr size_t DefaultWindowHeight = 500;
constexpr size_t MinWindowWidth = 550;
constexpr size_t MinWindowHeight = 400;
constexpr size_t LabelFontSize = 15;

App::App(bool verbose) :
  LabelFontInfo(LabelFontSize)
{
  Common::Log::instance().initialize(verbose);
  mLogger = Common::Log::create("GUI::App");
}

bool App::OnInit()
{
  wxImage::AddHandler(new wxPNGHandler);
  wxImage::AddHandler(new wxICOHandler);

  mFrame = new wxFrame(
    nullptr,
    -1,
    "Homepage - Transmitron",
    wxDefaultPosition,
    wxSize(DefaultWindowWidth, DefaultWindowHeight)
  );
  mFrame->SetMinSize(wxSize(MinWindowWidth, MinWindowHeight));
  mOptionsHeight = calculateOptionHeight();

  setupIcon();

  const int noteStyle = wxAUI_NB_DEFAULT_STYLE & ~(0
    | wxAUI_NB_TAB_MOVE
    | wxAUI_NB_TAB_EXTERNAL_MOVE
    | wxAUI_NB_TAB_SPLIT
  );

  mNote = new wxAuiNotebook(
    mFrame,
    -1,
    wxDefaultPosition,
    wxDefaultSize,
    noteStyle
  );

  const auto configDir = createConfigDir().string();
  const auto cacheDir  = createCacheDir().string();

  mLayoutsModel = new Models::Layouts();
  mLayoutsModel->load(configDir);

  mProfilesModel = new Models::Profiles(mLayoutsModel);
  mProfilesModel->load(configDir, cacheDir);

  const auto appearance = wxSystemSettings::GetAppearance();
  mDarkMode = appearance.IsDark() || appearance.IsUsingDarkBackground();

  createSettingsTab();

  createHomepageTab(1);

  auto *newProfilesTab = new wxPanel(mNote);
  mNote->AddPage(newProfilesTab, "", false, *bin2cPlus18x18());
  ++mCount;

  mNote->Bind(wxEVT_AUINOTEBOOK_PAGE_CHANGING, &App::onPageSelected, this);
  mNote->Bind(wxEVT_AUINOTEBOOK_PAGE_CLOSE, &App::onPageClosing, this);

  mFrame->Show();
  return true;
}

int App::FilterEvent(wxEvent &event)
{
  if (event.GetEventType() != wxEVT_KEY_DOWN)
  {
    return wxEventFilter::Event_Skip;
  }

  const auto keyEvent = dynamic_cast<wxKeyEvent&>(event);

  if (keyEvent.GetKeyCode() == 'W' && keyEvent.ControlDown())
  {
    onKeyDownControlW();
    return wxEventFilter::Event_Processed;
  }

  if (keyEvent.GetKeyCode() == 'T' && keyEvent.ControlDown())
  {
    onKeyDownControlT();
    return wxEventFilter::Event_Processed;
  }

  return wxEventFilter::Event_Skip;
}

bool App::openProfile(const std::string &profileName)
{
  const auto profileItem = mProfilesModel->getItemFromName(profileName);
  if (!profileItem.IsOk())
  {
    mLogger->warn("Profile '{}' not found", profileName);
    return false;
  }

  openProfile(profileItem);
  return true;
}

void App::onPageSelected(wxBookCtrlEvent& event)
{
  if (event.GetSelection() == wxNOT_FOUND)
  {
    event.Skip();
    return;
  }

  if ((size_t)event.GetSelection() == mCount - 1)
  {
    createHomepageTab(mCount - 1);
    event.Veto();
    return;
  }

  const auto style = mNote->GetWindowStyle();
  constexpr int Closable = 0
    | wxAUI_NB_CLOSE_BUTTON
    | wxAUI_NB_CLOSE_ON_ACTIVE_TAB;

  if ((size_t)event.GetSelection() == 0)
  {
    if ((style & Closable) != 0)
    {
      mNote->SetWindowStyle(style & ~Closable);
    }
  }
  else
  {
    if ((style & Closable) != Closable)
    {
      mNote->SetWindowStyle(style | Closable);
    }
  }

  const auto windowName = fmt::format(
    "{} - Transmitron",
    mNote->GetPage((size_t)event.GetSelection())->GetName()
  );
  mFrame->SetTitle(windowName);

  event.Skip();
}

void App::onPageClosing(wxBookCtrlEvent& event)
{
  const auto closingIndex = (size_t)event.GetSelection();

  // Settings and Plus.
  if (closingIndex == 0 || closingIndex == mCount - 1)
  {
    event.Veto();
    return;
  }

  --mCount;

  if (mCount == 2)
  {
    createHomepageTab(mCount - 1);
  }

  if ((size_t)event.GetOldSelection() == mCount - 1)
  {
    mNote->ChangeSelection(mCount - 2);
  }

  event.Skip();
}

void App::onKeyDownControlW()
{
  const auto closingIndex = (size_t)mNote->GetSelection();

  // Settings and Plus.
  if (closingIndex == 0 || closingIndex == mCount - 1)
  {
    return;
  }

  mNote->RemovePage(closingIndex);

  --mCount;

  if (mCount == 2)
  {
    createHomepageTab(mCount - 1);
  }

  if (closingIndex == mCount - 1)
  {
    mNote->ChangeSelection(mCount - 2);
  }
}

void App::onKeyDownControlT()
{
  createHomepageTab(mCount - 1);
}

void App::onRecordingSave(Events::Recording &event)
{
  const auto name = event.getName();
  const auto nameUtf8 = name.ToUTF8();
  const std::string nameStr(nameUtf8.data(), nameUtf8.length());
  mLogger->info("Storing recording for {}", nameStr);
  const std::string encoded = Url::encode(nameStr);

  const auto now = std::chrono::system_clock::now();
  const auto timestamp = Helpers::timeToFilename(now);
  const auto filename = encoded + "-" + timestamp + ".tmrc";

  wxFileDialog saveFileDialog(
    mFrame,
    _("Save TMRC file"),
    "",
    filename,
    "TMRC files (*.tmrc)|*.tmrc",
    wxFD_SAVE | wxFD_OVERWRITE_PROMPT
  );

  if (saveFileDialog.ShowModal() == wxID_CANCEL)
  {
    return;
  }

  const auto filepath = saveFileDialog.GetPath().ToStdString();
  std::ofstream out(filepath);
  if (!out.is_open())
  {
    mLogger->error("Cannot save current contents in file '{}'.", filepath);
    return;
  }

  out << event.getContents();
}

void App::onProfileCreate(Events::Profile &event)
{
  (void)event;
  mNote->ChangeSelection(0);
  mSettingsTab->createProfile();
}

void App::onProfileEdit(Events::Profile &event)
{
  mNote->ChangeSelection(0);
  mSettingsTab->selectProfile(event.getProfile());
}

void App::onRecordingOpen(Events::Recording &/* event */)
{
  wxFileDialog openFileDialog(
    mFrame,
    _("Open tmrc file"),
    "",
    "",
    "TMRC files (*.tmrc)|*.tmrc",
    wxFD_OPEN | wxFD_FILE_MUST_EXIST
  );

  if (openFileDialog.ShowModal() == wxID_CANCEL)
  {
    return;
  }

  const auto filename = openFileDialog.GetPath();
  const auto utf8 = filename.ToUTF8();
  const std::string encodedStr(utf8.data(), utf8.length());
  openRecording(encodedStr);
}


void App::createHomepageTab(size_t index)
{
  auto *homepage = new Tabs::Homepage(
    mNote,
    LabelFontInfo,
    mOptionsHeight,
    mProfilesModel,
    mLayoutsModel
  );
  mNote->InsertPage(index, homepage, "Homepage");
  ++mCount;
  mNote->SetSelection(index);
  homepage->focus();

  homepage->Bind(Events::RECORDING_OPEN, &App::onRecordingOpen, this);
  homepage->Bind(Events::PROFILE_CREATE, &App::onProfileCreate, this);
  homepage->Bind(Events::PROFILE_EDIT, &App::onProfileEdit, this);

  homepage->Bind(Events::CONNECTION_REQUESTED, [this](Events::Connection e){
    if (e.GetSelection() == wxNOT_FOUND)
    {
      e.Skip();
      return;
    }

    const auto profileItem = e.getProfile();
    openProfile(profileItem);
  });
}

void App::createSettingsTab()
{
  mSettingsTab = new Tabs::Settings(mNote, LabelFontInfo, mOptionsHeight, mProfilesModel, mLayoutsModel);
  mSettingsTab->Bind(Events::CONNECTION_REQUESTED, [this](Events::Connection e){
    if (e.GetSelection() == wxNOT_FOUND)
    {
      e.Skip();
      return;
    }

    const auto profileItem = e.getProfile();
    openProfile(profileItem);
  });
  mNote->AddPage(mSettingsTab, "", false, *bin2cGear18x18());
  ++mCount;
}

Common::fs::path App::createConfigDir()
{
  const auto configHome = Common::XdgBaseDir::configHome();
  const auto config = fmt::format("{}/{}", configHome.string(), Info::getProjectName());

  if (!fs::is_directory(config) || !fs::exists(config))
  {
    if (!fs::create_directory(config))
    {
      mLogger->warn("Could not create directory '%s'", config);
      return {};
    }
  }

  mLogger->info("Config dir: {}", config);

  return config;
}

Common::fs::path App::createCacheDir()
{
  const auto configHome = Common::XdgBaseDir::cacheHome();
  const auto config = fmt::format("{}/{}", configHome.string(), Info::getProjectName());

  if (!fs::is_directory(config) || !fs::exists(config))
  {
    if (!fs::create_directory(config))
    {
      mLogger->warn("Could not create directory '%s'", config);
      return {};
    }
  }

  mLogger->info("Cache dir:  {}", config);

  return config;
}

void App::setupIcon()
{
  Common::fs::path p = fmt::format(
    "{}/share/transmitron/transmitron.ico",
    getInstallPrefix().string()
  );
  p.make_preferred();
  mLogger->debug("Loading icon from {}", p.string());
  mFrame->SetIcons(wxIconBundle(p.string()));
}

Common::fs::path App::getExecutablePath()
{
  const auto pathfinder = []()
  {
#if defined WIN32
    std::array<wchar_t, MAX_PATH> moduleFileName {};
    const auto r = GetModuleFileNameW(nullptr, moduleFileName.data(), MAX_PATH);
    if (r == 0)
    {
      throw std::runtime_error("Could not determine installation prefix");
    }

    return std::wstring(moduleFileName.data(), moduleFileName.size());
#else
    return Common::fs::read_symlink("/proc/self/exe");
#endif
  };

  static const Common::fs::path p = pathfinder();
  mLogger->info("Executable path: {}", p.string());
  return p;
}

Common::fs::path App::getInstallPrefix()
{
  const Common::fs::path executable = getExecutablePath();
  const auto executableDir = executable.parent_path();
  auto prefix = executableDir.parent_path();
  return prefix;
}

void App::openProfile(wxDataViewItem item)
{
  const auto options = mProfilesModel->getBrokerOptions(item);

  auto *client = new Tabs::Client(
    mNote,
    options,
    mProfilesModel->getClientOptions(item),
    mProfilesModel->getMessagesModel(item),
    mProfilesModel->getTopicsSubscribed(item),
    mProfilesModel->getTopicsPublished(item),
    mLayoutsModel,
    mProfilesModel->getName(item),
    mDarkMode,
    mOptionsHeight
  );

  client->Bind(Events::RECORDING_SAVE, &App::onRecordingSave, this);

  const size_t selected = (size_t)mNote->GetSelection();
  const auto target = selected == 0
    ? mCount - 1
    : selected;

  mLogger->info("Target: {}", target);

  if (selected != 0)
  {
    // Remove homepage.
    mNote->RemovePage(selected);
  }
  else
  {
    ++mCount;
  }

  mNote->InsertPage(target, client, "");
  mNote->SetSelection(target);
  mNote->SetPageText(target, mProfilesModel->getName(item));

  client->focus();
}

void App::openRecording(const std::string &filename)
{
  Common::fs::path path(filename);
  const auto filenameStr = path.stem().string();
  const std::string decodedStr = Url::decode(filenameStr);

  wxObjectDataPtr<Models::Subscriptions> subscriptions;
  subscriptions = new Models::Subscriptions();
  const auto subscriptionsLoaded = subscriptions->load(filename);

  wxObjectDataPtr<Models::History> history;
  history = new Models::History(subscriptions);
  const auto historyLoaded = history->load(filename);

  if (!subscriptionsLoaded || !historyLoaded)
  {
    mLogger->warn("Could not load recording: '{}'", filename);
    return;
  }

  auto *client = new Tabs::Client(
    mNote,
    history,
    subscriptions,
    mLayoutsModel,
    decodedStr,
    mDarkMode,
    mOptionsHeight
  );

  const size_t selected = (size_t)mNote->GetSelection();
  mNote->RemovePage(selected);
  mNote->InsertPage(selected, client, "");
  mNote->SetSelection(selected);
  mNote->SetPageText(selected, decodedStr);

  client->focus();
}

int App::calculateOptionHeight()
{
  auto *button = new wxBitmapButton(mFrame, -1, *bin2cPlus18x18());
  const auto bestSize = button->GetBestSize();
  mFrame->RemoveChild(button);
  button->Destroy();
  return bestSize.y;
}
