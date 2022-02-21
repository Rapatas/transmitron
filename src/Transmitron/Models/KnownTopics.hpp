#ifndef TRANSMITRON_MODELS_KNOWNTOPICS_HPP
#define TRANSMITRON_MODELS_KNOWNTOPICS_HPP

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

  void clear();
  void setFilter(std::string filter);
  void append(std::string topic);

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

  std::shared_ptr<spdlog::logger> mLogger;
  std::vector<std::string> mTopics;
  std::vector<size_t> mRemap;
  std::string mFilter;

  void remap();

};

}

#endif // TRANSMITRON_MODELS_KNOWNTOPICS_HPP
