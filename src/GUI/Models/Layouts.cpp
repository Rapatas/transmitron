#include "Layouts.hpp"

#include <algorithm>
#include <cstring>
#include <fstream>
#include <iterator>
#include <stdexcept>

#include <fmt/format.h>
#include <nlohmann/json.hpp>
#include <system_error>

#include "Common/Log.hpp"
#include "Common/Url.hpp"

using namespace Rapatas::Transmitron;
using namespace GUI::Models;
using namespace Common;

Layouts::Layouts() :
  mLogger(Common::Log::create("Models::Layouts")) //
{
  injectDefaultLayouts();
}

// Public {

bool Layouts::load(const std::string &configDir) {
  if (configDir.empty()) {
    mLogger->error("No directory provided");
    return false;
  }

  mLayoutsDir = configDir + "/layouts";

  const bool exists = fs::exists(mLayoutsDir);
  const bool isDir = fs::is_directory(mLayoutsDir);

  std::error_code ec;
  if (exists && !isDir && !fs::remove(mLayoutsDir, ec)) {
    mLogger->warn("Could not remove file {}: {}", mLayoutsDir, ec.message());
    return false;
  }

  if (!exists && !fs::create_directory(mLayoutsDir)) {
    mLogger->warn("Could not create profiles directory: {}", mLayoutsDir);
    return false;
  }

  for (const auto &entry : fs::directory_iterator(mLayoutsDir)) {
    const auto item = loadLayoutFile(entry.path());
    if (!item.IsOk()) { continue; }

    const auto parent = wxDataViewItem(nullptr);
    ItemAdded(parent, item);
  }

  return true;
}

bool Layouts::remove(wxDataViewItem item) {
  const auto id = toId(item);
  const auto &node = mLayouts.at(id);

  std::error_code ec;
  fs::remove_all(node->path, ec);
  if (ec) {
    mLogger->error("Could not delete '{}': {}", node->name, ec.message());
    return false;
  }

  mLayouts.erase(id);

  const auto parent = wxDataViewItem(nullptr);
  ItemDeleted(parent, item);

  return true;
}

wxDataViewItem Layouts::create(const Perspective &perspective) {
  const std::string name = getUniqueName();
  const std::string encoded = Url::encode(name);

  const std::string path = fmt::format("{}/{}.json", mLayoutsDir, encoded);

  const auto id = mAvailableId++;
  auto layout = std::make_unique<Node>();
  layout->name = name;
  layout->perspective = perspective;
  layout->path = path;
  layout->saved = false;
  mLayouts.insert({id, std::move(layout)});

  save(id);

  const auto parent = wxDataViewItem(nullptr);
  const auto item = toItem(id);
  ItemAdded(parent, item);

  return item;
}

// Getters {

wxDataViewItem Layouts::getDefault() { return toItem(1); }

bool Layouts::isDeletable(wxDataViewItem item) {
  const auto id = toId(item);
  return id >= 4;
}

const Layouts::Perspective &Layouts::getPerspective(wxDataViewItem item) const {
  const auto id = toId(item);
  const auto &node = mLayouts.at(id);
  return node->perspective;
}

const std::string &Layouts::getName(wxDataViewItem item) const {
  const auto id = toId(item);
  const auto &node = mLayouts.at(id);
  return node->name;
}

wxArrayString Layouts::getLabelArray() const {
  wxArrayString result;
  for (const auto &layout : mLayouts) { result.push_back(layout.second->name); }
  return result;
}

std::vector<std::string> Layouts::getLabelVector() const {
  std::vector<std::string> result;
  result.reserve(mLayouts.size());
  for (const auto &layout : mLayouts) { result.push_back(layout.second->name); }
  return result;
}

std::string Layouts::getUniqueName() const {
  const constexpr std::string_view NewLayoutName{"New Layout"};
  std::string uniqueName{NewLayoutName};
  unsigned postfix = 0;
  while (std::any_of(
    std::begin(mLayouts),
    std::end(mLayouts),
    [=](const auto &layout) { return layout.second->name == uniqueName; }
  )) {
    ++postfix;
    uniqueName = fmt::format("{} - {}", NewLayoutName, postfix);
  }

  return uniqueName;
}

wxDataViewItem Layouts::findItemByName(const std::string &name) const {
  const auto it = std::find_if(
    std::begin(mLayouts),
    std::end(mLayouts),
    [name](const auto &layout) { return layout.second->name == name; }
  );

  if (it == std::end(mLayouts)) { return wxDataViewItem(nullptr); }

  return toItem(it->first);
}

// Getters }

// wxDataViewModel interface {

unsigned Layouts::GetChildren(
  const wxDataViewItem &parent,
  wxDataViewItemArray &children
) const {
  if (parent.IsOk()) { return 0; }

  for (const auto &node : mLayouts) { children.push_back(toItem(node.first)); }

  std::sort(
    std::begin(children),
    std::end(children),
    [this](wxDataViewItem lhs, wxDataViewItem rhs) {
      const auto lhsid = toId(lhs);
      const auto rhsid = toId(rhs);

      auto lhsv = mLayouts.at(lhsid)->name;
      auto rhsv = mLayouts.at(rhsid)->name;

      std::transform(
        rhsv.begin(),
        rhsv.end(),
        rhsv.begin(),
        [](unsigned char value) { return std::tolower(value); }
      );

      std::transform(
        lhsv.begin(),
        lhsv.end(),
        lhsv.begin(),
        [](unsigned char value) { return std::tolower(value); }
      );

      return lhsv < rhsv;
    }
  );

  return static_cast<unsigned>(children.size());
}

unsigned Layouts::GetColumnCount() const {
  return static_cast<unsigned>(Column::Max);
}

wxString Layouts::GetColumnType(unsigned int /* col */) const {
  return wxDataViewTextRenderer::GetDefaultType();
}

wxDataViewItem Layouts::GetParent(const wxDataViewItem & /* item */
) const {
  return wxDataViewItem(nullptr);
}

bool Layouts::IsEnabled(
  const wxDataViewItem & /* item */,
  unsigned int /* col */
) const {
  return true;
}

bool Layouts::IsContainer(const wxDataViewItem &item) const {
  return !item.IsOk();
}

void Layouts::GetValue(
  wxVariant &value,
  const wxDataViewItem &item,
  unsigned int col
) const {
  const auto id = toId(item);
  const auto &node = mLayouts.at(id);

  switch (static_cast<Column>(col)) {
    case Column::Name: {
      const auto &name = node->name;
      const auto wxs = wxString::FromUTF8(name.data(), name.length());
      value = wxs;
    } break;
    default: {
    }
  }
}

bool Layouts::SetValue(
  const wxVariant &value,
  const wxDataViewItem &item,
  unsigned int col
) {
  const auto id = toId(item);
  if (static_cast<Column>(col) != Column::Name) { return false; }
  if (value.GetType() != "string") { return false; }
  if (value.GetString().empty()) {
    mLogger->warn("Empty name provided");
    return false;
  }

  const auto wxs = value.GetString();
  const auto utf8 = wxs.ToUTF8();
  const std::string newName(utf8.data(), utf8.length());

  auto &node = mLayouts.at(id);

  if (node->name == newName) { return true; }

  for (const auto &layout : mLayouts) {
    if (layout.second->name == newName) { return false; }
  }

  mLogger->info("Renaming '{}' to '{}'", node->name, newName);

  const std::string encoded = Url::encode(newName);

  const std::string newPath = fmt::format("{}/{}.json", mLayoutsDir, encoded);

  std::error_code ec;
  fs::rename(node->path, newPath, ec);
  if (ec) {
    mLogger->error(
      "Could not rename '{}' to '{}': {}",
      node->path.string(),
      newPath,
      ec.message()
    );
    return false;
  }

  node->name = newName;
  node->path = newPath;

  ItemChanged(item);

  return true;
}

// wxDataViewModel interface }

// Public }

// Private {

void Layouts::injectDefaultLayouts() {
  // clang-format off
  const std::string_view peIDE{
    "layout2|"
    "name=History;caption=History;state=1340;dir=5;layer=0;row=0;pos=0;prop=100000;bestw=200;besth=100;minw=200;minh=100;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=-1;floath=-1|"
    "name=Subscriptions;caption=Subscriptions;state=1532;dir=4;layer=1;row=0;pos=0;prop=100000;bestw=200;besth=100;minw=200;minh=100;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=-1;floath=-1|"
    "name=Messages;caption=Messages;state=1532;dir=4;layer=1;row=0;pos=1;prop=100000;bestw=200;besth=100;minw=200;minh=100;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=-1;floath=-1|"
    "name=Publish;caption=Publish;state=1532;dir=3;layer=2;row=0;pos=0;prop=100000;bestw=200;besth=200;minw=200;minh=200;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=-1;floath=-1|"
    "name=Preview;caption=Preview;state=1532;dir=3;layer=2;row=0;pos=1;prop=100000;bestw=200;besth=200;minw=200;minh=200;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=-1;floath=-1|"
    "dock_size(5,0,0)=200|"
    "dock_size(4,1,0)=200|"
    "dock_size(3,2,0)=214|"
  };
  const std::string_view peDefault{
    "layout2|"
    "name=History;caption=History;state=1340;dir=5;layer=0;row=0;pos=0;prop=100000;bestw=412;besth=100;minw=100;minh=100;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=-1;floath=-1|"
    "name=Subscriptions;caption=Subscriptions;state=1532;dir=1;layer=1;row=0;pos=0;prop=100000;bestw=412;besth=100;minw=100;minh=100;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=-1;floath=-1|"
    "name=Messages;caption=Messages;state=1532;dir=4;layer=2;row=0;pos=0;prop=100000;bestw=200;besth=20;minw=100;minh=100;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=-1;floath=-1|"
    "name=Publish;caption=Publish;state=1532;dir=2;layer=2;row=0;pos=0;prop=100000;bestw=412;besth=20;minw=100;minh=100;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=-1;floath=-1|"
    "name=Preview;caption=Preview;state=1532;dir=2;layer=2;row=0;pos=1;prop=100000;bestw=412;besth=20;minw=100;minh=100;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=-1;floath=-1|"
    "dock_size(5,0,0)=412|"
    "dock_size(1,1,0)=117|"
    "dock_size(4,2,0)=200|"
    "dock_size(2,2,0)=412|"
  };
  const std::string_view peCasual{
    "layout2|"
    "name=History;caption=History;state=1340;dir=5;layer=0;row=0;pos=0;prop=100000;bestw=200;besth=100;minw=200;minh=100;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=-1;floath=-1|"
    "name=Subscriptions;caption=Subscriptions;state=1532;dir=1;layer=0;row=1;pos=0;prop=100000;bestw=200;besth=100;minw=200;minh=100;maxw=-1;maxh=-1;floatx=847;floaty=333;floatw=400;floath=250|"
    "name=Publish;caption=Publish;state=1532;dir=4;layer=2;row=0;pos=0;prop=100000;bestw=200;besth=200;minw=200;minh=200;maxw=-1;maxh=-1;floatx=196;floaty=543;floatw=400;floath=250|"
    "name=Preview;caption=Preview;state=1532;dir=2;layer=3;row=0;pos=0;prop=100000;bestw=200;besth=200;minw=200;minh=200;maxw=-1;maxh=-1;floatx=1286;floaty=536;floatw=400;floath=250|"
    "dock_size(5,0,0)=200|"
    "dock_size(2,3,0)=322|"
    "dock_size(1,0,1)=117|"
    "dock_size(4,2,0)=309|"
  };
  // clang-format on

  {
    const auto id = mAvailableId++;
    auto layout = std::make_unique<Node>();
    layout->name = "Default";
    layout->perspective = Perspective(peDefault);
    layout->path = "";
    layout->saved = true;
    mLayouts.insert({id, std::move(layout)});
    const auto parent = wxDataViewItem(nullptr);
    const auto item = toItem(id);
    ItemAdded(parent, item);
  }

  {
    const auto id = mAvailableId++;
    auto layout = std::make_unique<Node>();
    layout->name = "IDE";
    layout->perspective = Perspective(peIDE);
    layout->path = "";
    layout->saved = true;
    mLayouts.insert({id, std::move(layout)});
    const auto parent = wxDataViewItem(nullptr);
    const auto item = toItem(id);
    ItemAdded(parent, item);
  }

  {
    const auto id = mAvailableId++;
    auto layout = std::make_unique<Node>();
    layout->name = "Casual";
    layout->perspective = Perspective(peCasual);
    layout->path = "";
    layout->saved = true;
    mLayouts.insert({id, std::move(layout)});
    const auto parent = wxDataViewItem(nullptr);
    const auto item = toItem(id);
    ItemAdded(parent, item);
  }
}

wxDataViewItem Layouts::loadLayoutFile(const Common::fs::path &filepath) {
  mLogger->info("Checking {}", filepath.u8string());

  // Get name.
  std::string decoded;
  try {
    decoded = Url::decode(filepath.stem().u8string());
  } catch (std::runtime_error &error) {
    mLogger->error(
      "Could not decode '{}': {}",
      filepath.u8string(),
      error.what() //
    );
    return wxDataViewItem(nullptr);
  }

  std::ifstream input(filepath);
  if (!input.is_open()) {
    mLogger->warn("Could not open '{}'", filepath.u8string());
    return wxDataViewItem(nullptr);
  }

  std::stringstream buffer;
  buffer << input.rdbuf();
  if (!nlohmann::json::accept(buffer.str())) {
    mLogger->warn("Could not parse '{}'", filepath.u8string());
    return wxDataViewItem(nullptr);
  }

  const auto data = nlohmann::json::parse(buffer.str());
  const auto perspectiveIt = data.find("perspective");
  if (perspectiveIt == std::end(data)
      || perspectiveIt->type() != nlohmann::json::value_t::string //
  ) {
    mLogger->warn("Could not parse '{}'", filepath.u8string());
    return wxDataViewItem(nullptr);
  }

  const auto id = mAvailableId++;
  auto layout = std::make_unique<Node>();
  layout->name = decoded;
  layout->perspective = perspectiveIt->get<std::string>();
  layout->path = filepath;
  layout->saved = false;
  mLayouts.insert({id, std::move(layout)});

  return toItem(id);
}

bool Layouts::save(size_t id) {
  auto &layout = mLayouts.at(id);
  if (layout->saved) { return true; }

  std::ofstream output(layout->path);
  if (!output.is_open()) {
    const auto ec = std::error_code(errno, std::system_category());
    mLogger->error("Could not save '{}':", layout->path.string(), ec.message());
    return false;
  }

  output << nlohmann::json{{"perspective", layout->perspective}};
  layout->saved = true;
  return true;
}

// Static {

Layouts::Node::Id_t Layouts::toId(const wxDataViewItem &item) {
  uintptr_t result = 0;
  const void *id = item.GetID();
  std::memcpy(&result, &id, sizeof(item.GetID()));
  return result;
}

wxDataViewItem Layouts::toItem(Node::Id_t id) {
  void *itemId = nullptr;
  const uintptr_t value = id;
  std::memcpy(&itemId, &value, sizeof(id));
  return wxDataViewItem(itemId);
}

// Static }

// Private }
