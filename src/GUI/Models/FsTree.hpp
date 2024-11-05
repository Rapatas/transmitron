#pragma once

#include <list>
#include <memory>
#include <set>

#include <spdlog/spdlog.h>
#include <wx/dataview.h>

#include "GUI/ArtProvider.hpp"

namespace Rapatas::Transmitron::GUI::Models {

class FsTree : public wxDataViewModel
{
public:

  using Id = size_t;
  using Item = wxDataViewItem;

  class Leaf
  {
  public:

    Leaf() = default;
    virtual ~Leaf() = default;
    Leaf(const Leaf &other) = delete;
    Leaf(Leaf &&other) = delete;
    Leaf &operator=(const Leaf &other) = delete;
    Leaf &operator=(Leaf &&other) = delete;
  };

  enum Column : uint8_t {
    Name,
    Max
  };

  explicit FsTree(
    const std::string &name,
    size_t columnCount,
    const ArtProvider &artProvider
  );

  static Item getRootItem();

  Item createFolder(Item parent);
  bool rename(Item item, const std::string &name);
  bool remove(Item item);

  Item moveBefore(Item item, Item sibling);
  Item moveAfter(Item item, Item sibling);
  Item moveInsideFirst(Item item, Item parent);
  Item moveInsideLast(Item item, Item parent);
  Item moveInsideAtIndex(Item item, Item parent, size_t index);

  [[nodiscard]] bool hasChildNamed(Item parent, const std::string &name) const;
  [[nodiscard]] Item getItemFromName(const std::string &name) const;
  [[nodiscard]] std::string getName(Item item) const;

  [[nodiscard]] unsigned GetColumnCount() const override;
  [[nodiscard]] wxString GetColumnType(unsigned int col) const override;
  void GetValue(
    wxVariant &variant,
    const Item &item,
    unsigned int col //
  ) const override;
  bool SetValue(
    const wxVariant &value,
    const Item &item,
    unsigned int col //
  ) override;
  [[nodiscard]] bool IsEnabled(
    const Item &item,
    unsigned int col //
  ) const override;
  [[nodiscard]] Item GetParent(const Item &item) const override;
  [[nodiscard]] bool IsContainer(const Item &item) const override;
  unsigned int GetChildren(
    const Item &parent,
    wxDataViewItemArray &array //
  ) const override;

protected:

  static Id toId(const Item &item);
  static Item toItem(Id id);

  bool load(const std::string &baseDir);
  Item leafReplace(Item item, std::unique_ptr<Leaf> leaf);
  Item leafCreate(
    Item parent,
    std::unique_ptr<Leaf> leaf,
    const std::string &name
  );
  Item leafInsert(
    const std::string &name,
    std::unique_ptr<Leaf> leaf,
    Item parent
  );

  [[nodiscard]] virtual bool isLeaf(const Common::fs::directory_entry &entry
  ) const = 0;
  virtual std::unique_ptr<Leaf> leafLoad(
    Id id,
    const Common::fs::path &path
  ) = 0;
  virtual void leafValue(Id id, wxDataViewIconText &value, unsigned int col)
    const = 0;
  virtual bool leafSave(Id id) = 0;

  [[nodiscard]] std::map<Id, Leaf *> getLeafs() const;
  [[nodiscard]] Leaf *getLeaf(Item item) const;
  [[nodiscard]] std::string getNodePath(Id id) const;
  [[nodiscard]] std::vector<std::string> getNodeSegments(Id id) const;
  [[nodiscard]] std::string createUniqueName(Item parent, std::string_view name)
    const;

private:

  struct Node {
    enum class Type : uint8_t {
      Folder,
      Payload,
    };

    Id parent = 0;
    std::string name;
    std::string encoded;
    Type type = Type::Folder;
    std::list<Id> children;
    std::unique_ptr<Leaf> payload;
    bool saved = false;
  };

  std::shared_ptr<spdlog::logger> mLogger;
  Id mNextAvailableId = 0;
  std::map<Id, Node> mNodes;
  std::string mBaseDir;
  const ArtProvider &mArtProvider;
  std::set<std::string> mIgnoreDirs;
  size_t mColumnCount;

  void clear();
  void loadDirectoryRecursive(const Common::fs::path &path, Id parentId);
  void loadLeaf(const Common::fs::directory_entry &entry, Id parentId);
  bool indexFileRead(const Common::fs::path &path, Id id);
  bool indexFileWrite(Id id);
  bool save(Id id);
  bool saveLeaf(Id id);
  bool saveFolder(Id id);
  bool moveFile(Id nodeId, Id newParentId);
  bool moveCheck(Item item, Item parent, size_t index);
  void moveUnderNewParent(Id nodeId, Id newParentId, size_t index);
  void moveUnderSameParent(Id nodeId, Id newParentId, size_t index);
  Id getNextId();

  [[nodiscard]] bool isRecursive(Item parent, Item item) const;
};

} // namespace Rapatas::Transmitron::GUI::Models
