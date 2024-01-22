#pragma once

#include "Common/Filesystem.hpp"
#include <map>
#include <memory>
#include <string>

#include <spdlog/spdlog.h>
#include <wx/arrstr.h>
#include <wx/dataview.h>

namespace Rapatas::Transmitron::GUI::Models
{

class Layouts :
  public wxDataViewModel
{
public:

  static constexpr std::string_view DefaultName = "Default";

  using Perspective = std::string;

  enum class Column : unsigned
  {
    Name,
    Max
  };

  explicit Layouts();

  bool load(const std::string &configDir);
  wxDataViewItem create(const Perspective &perspective);
  bool remove(wxDataViewItem item);

  // Getters
  static wxDataViewItem getDefault();
  [[nodiscard]] wxDataViewItem findItemByName(const std::string &name) const;
  [[nodiscard]] std::string getUniqueName() const;
  [[nodiscard]] const Perspective &getPerspective(wxDataViewItem item) const;
  [[nodiscard]] const std::string &getName(wxDataViewItem item) const;
  [[nodiscard]] wxArrayString getLabelArray() const;
  [[nodiscard]] std::vector<std::string> getLabelVector() const;

  // wxDataViewModel interface.
  [[nodiscard]] unsigned GetColumnCount() const override;
  [[nodiscard]] wxString GetColumnType(unsigned int col) const override;
  void GetValue(
    wxVariant &value,
    const wxDataViewItem &item,
    unsigned int col
  ) const override;
  bool SetValue(
    const wxVariant &value,
    const wxDataViewItem &item,
    unsigned int col
  ) override;
  [[nodiscard]] bool IsEnabled(
    const wxDataViewItem &item,
    unsigned int col
  ) const override;
  [[nodiscard]] wxDataViewItem GetParent(
    const wxDataViewItem &item
  ) const override;
  [[nodiscard]] bool IsContainer(
    const wxDataViewItem &item
  ) const override;
  unsigned int GetChildren(
    const wxDataViewItem &parent,
    wxDataViewItemArray &children
  ) const override;

private:

  struct Node
  {
    using Id_t = size_t;
    std::string name;
    Perspective perspective;
    Common::fs::path path;
    bool saved = false;
  };

  std::shared_ptr<spdlog::logger> mLogger;
  Node::Id_t mAvailableId = 1;
  std::map<Node::Id_t, std::unique_ptr<Node>> mLayouts;
  std::string mLayoutsDir;

  static constexpr std::string_view DefaultPerspective = "layout2|name=History;caption=History;state=1340;dir=5;layer=0;row=0;pos=0;prop=100000;bestw=200;besth=100;minw=200;minh=100;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=-1;floath=-1|name=Subscriptions;caption=Subscriptions;state=1532;dir=4;layer=1;row=0;pos=0;prop=100000;bestw=200;besth=100;minw=200;minh=100;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=-1;floath=-1|name=Messages;caption=Messages;state=1532;dir=4;layer=1;row=0;pos=1;prop=100000;bestw=200;besth=100;minw=200;minh=100;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=-1;floath=-1|name=Publish;caption=Publish;state=1532;dir=3;layer=2;row=0;pos=0;prop=100000;bestw=200;besth=200;minw=200;minh=200;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=-1;floath=-1|name=Preview;caption=Preview;state=1532;dir=3;layer=2;row=0;pos=1;prop=100000;bestw=200;besth=200;minw=200;minh=200;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=-1;floath=-1|dock_size(5,0,0)=200|dock_size(4,1,0)=200|dock_size(3,2,0)=214|";

  wxDataViewItem loadLayoutFile(const Common::fs::path &filepath);
  bool save(size_t id);

  static Node::Id_t toId(const wxDataViewItem &item);
  static wxDataViewItem toItem(Node::Id_t id);
};

} // namespace Rapatas::Transmitron::GUI::Models

