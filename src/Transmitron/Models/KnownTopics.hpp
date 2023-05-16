#ifndef TRANSMITRON_MODELS_KNOWNTOPICS_HPP
#define TRANSMITRON_MODELS_KNOWNTOPICS_HPP

#include "Common/Filesystem.hpp"
#include <set>
#include <vector>
#include <wx/dataview.h>
#include "Transmitron/Models/Subscriptions.hpp"

namespace Transmitron::Models
{

class KnownTopics :
  public wxDataViewVirtualListModel
{
public:

  enum class Column : unsigned
  {
    Topic,
    Max
  };

  explicit KnownTopics();
  ~KnownTopics() override;

  KnownTopics(const KnownTopics &other) = delete;
  KnownTopics(KnownTopics &&other) = delete;
  KnownTopics &operator=(const KnownTopics &other) = delete;
  KnownTopics &operator=(KnownTopics &&other) = delete;

  bool load(Common::fs::path filepath);

  void clear();
  void setFilter(std::string filter);
  void append(std::string topic);
  void append(std::set<std::string> topics);

  const std::string &getTopic(const wxDataViewItem &item) const;
  const std::string &getFilter() const;

  // wxDataViewVirtualListModel interface.
  unsigned GetColumnCount() const override;
  wxString GetColumnType(unsigned int col) const override;
  unsigned GetCount() const override;
  void GetValueByRow(
    wxVariant &variant,
    unsigned int row,
    unsigned int col
  ) const override;
  bool GetAttrByRow(
    unsigned int row,
    unsigned int col,
    wxDataViewItemAttr &attr
  ) const override;
  bool SetValueByRow(
    const wxVariant &variant,
    unsigned int row,
    unsigned int col
  ) override;

private:

  using Topics = std::set<std::string>;

  std::shared_ptr<spdlog::logger> mLogger;
  Common::fs::path mFilepath;
  Topics mTopics;
  std::vector<Topics::const_iterator> mRemap;
  std::string mFilter;

  void remap();
  void save();

};

}

#endif // TRANSMITRON_MODELS_KNOWNTOPICS_HPP
