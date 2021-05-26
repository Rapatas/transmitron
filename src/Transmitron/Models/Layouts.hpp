#ifndef TRANSMITRON_MODELS_LAYOUTS_HPP
#define TRANSMITRON_MODELS_LAYOUTS_HPP

#include <string>
#include <map>
#include <filesystem>
#include <wx/arrstr.h>
#include <wx/dataview.h>

namespace Transmitron::Models
{

class Layouts :
  public wxDataViewModel
{
public:

  enum class Column : unsigned
  {
    Name,
    Max
  };

  explicit Layouts();

  bool load(const std::string &configDir);
  wxDataViewItem getDefault() const;
  wxDataViewItem getItem(const std::string &name) const;
  std::string getUniqueName() const;
  std::optional<std::string> getLayout(const std::string &name) const;
  std::string getName(wxDataViewItem item) const;
  wxDataViewItem create(const std::string &name, const std::string &perspective);
  bool remove(wxDataViewItem item);
  bool updateName(
    wxDataViewItem item,
    const std::string &name
  );

  unsigned GetColumnCount() const override;
  wxString GetColumnType(unsigned int col) const override;
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
  bool IsEnabled(
    const wxDataViewItem &item,
    unsigned int col
  ) const override;
  wxDataViewItem GetParent(
    const wxDataViewItem &item
  ) const override;
  bool IsContainer(
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
    std::string perspective;
    std::filesystem::path path;
    bool saved = false;
  };

  Node::Id_t mAvailableId = 1;
  std::map<Node::Id_t, std::unique_ptr<Node>> mLayouts;
  std::string mLayoutsDir;

  const std::string DefaultPerspective = "layout2|name=History;caption=History;state=1340;dir=5;layer=0;row=0;pos=0;prop=100000;bestw=200;besth=100;minw=200;minh=100;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=-1;floath=-1|name=Subscriptions;caption=Subscriptions;state=1532;dir=4;layer=1;row=0;pos=0;prop=100000;bestw=200;besth=100;minw=200;minh=100;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=-1;floath=-1|name=Snippets;caption=Snippets;state=1532;dir=4;layer=1;row=0;pos=1;prop=100000;bestw=200;besth=100;minw=200;minh=100;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=-1;floath=-1|name=Publish;caption=Publish;state=1532;dir=3;layer=2;row=0;pos=0;prop=100000;bestw=200;besth=200;minw=200;minh=200;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=-1;floath=-1|name=Preview;caption=Preview;state=1532;dir=3;layer=2;row=0;pos=1;prop=100000;bestw=200;besth=200;minw=200;minh=200;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=-1;floath=-1|dock_size(5,0,0)=200|dock_size(4,1,0)=200|dock_size(3,2,0)=214|";

  bool save(size_t index);

  static Node::Id_t toId(const wxDataViewItem &item);
  static wxDataViewItem toItem(Node::Id_t id);
};

}

#endif // TRANSMITRON_MODELS_LAYOUTS_HPP
