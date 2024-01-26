#pragma once

#include "Common/Filesystem.hpp"
#include <wx/bitmap.h>
#include <map>
#include <spdlog/logger.h>

namespace Rapatas::Transmitron::GUI
{

enum class Icon
{
  Add,
  Archive,
  Clear,
  Connect,
  Copy,
  Delete,
  Edit,
  History,
  Profile,
  File,
  FileFull,
  Folder,
  NewProfile,
  Cancel,
  Home,
  Mute,
  Unmute,
  NewColor,
  NewDir,
  NewFile,
  Publish,
  Save,
  SaveAs,
  Search,
  Settings,
  Solo,
  Subscribe,
  Subscriptions,
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
