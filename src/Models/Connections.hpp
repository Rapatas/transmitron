#ifndef CONNECTIONS_MODEL_H
#define CONNECTIONS_MODEL_H

#include "Connection.hpp"
#include <filesystem>
#include <wx/dataview.h>

class Connections :
  public wxDataViewModel
{
public:

  explicit Connections();
  virtual ~Connections();

  enum class Column : unsigned
  {
    Name,
    URL,
    Max
  };

  void updateConnection(wxDataViewItem &item, const Connection &data);
  wxDataViewItem createConnection(const Connection &data);
  Connection getConnection(wxDataViewItem &item) const;

private:

  struct ConnectionInfo
  {
    Connection connection;
    bool saved = false;
  };

  std::vector<ConnectionInfo*> mConnections;

  // wxDataViewModel interface.
  virtual unsigned GetColumnCount() const override;
  virtual wxString GetColumnType(unsigned int col) const override;
  virtual void GetValue(
    wxVariant &variant,
    const wxDataViewItem &item,
    unsigned int col
  ) const override;
  virtual bool SetValue(
    const wxVariant &variant,
    const wxDataViewItem &item,
    unsigned int col
  ) override;
  virtual bool IsEnabled(
    const wxDataViewItem &item,
    unsigned int col
  ) const override;
  virtual wxDataViewItem GetParent(
    const wxDataViewItem &item
  ) const override;
  virtual bool IsContainer(
    const wxDataViewItem &item
  ) const override;
  virtual unsigned int GetChildren(
    const wxDataViewItem &parent,
    wxDataViewItemArray &array
  ) const override;

  static std::filesystem::path getConfigPath();
  static std::filesystem::path toFileName(const std::string &name);
};

#endif // CONNECTIONS_MODEL_H
