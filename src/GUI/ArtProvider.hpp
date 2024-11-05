#pragma once

#include <map>

#include <spdlog/logger.h>
#include <wx/bitmap.h>

#include "Common/Filesystem.hpp"

namespace Rapatas::Transmitron::GUI {

enum class Icon : uint8_t {
  Add,
  Archive,
  Cancel,
  Clear,
  Connect,
  Connected,
  Connecting,
  Copy,
  Delete,
  Disconnected,
  Edit,
  File,
  FileFull,
  Folder,
  History,
  Home,
  Mute,
  NewColor,
  NewDir,
  NewFile,
  NewProfile,
  Profile,
  Publish,
  Save,
  SaveAs,
  Search,
  Settings,
  Solo,
  Subscribe,
  Subscriptions,
  Unmute,
  Unsubscribe,
};

class ArtProvider
{
public:

  ArtProvider();

  void initialize(const Common::fs::path &base, wxSize size, bool dark);

  [[nodiscard]] const wxBitmap &bitmap(Icon icon) const;

private:

  std::shared_ptr<spdlog::logger> mLogger;
  std::map<Icon, wxBitmap> mIcons;
  wxBitmap mPlaceholder;
};

} // namespace Rapatas::Transmitron::GUI
