#ifndef TRANSMITRON_MODELS_CONNECTIONS_HPP
#define TRANSMITRON_MODELS_CONNECTIONS_HPP

#include <filesystem>
#include <wx/dataview.h>
#include "Transmitron/Types/Connection.hpp"

namespace Transmitron::Models
{

class Connections :
  public wxDataViewModel
{
public:

  enum class Column : unsigned
  {
    Name,
    URL,
    Max
  };

  explicit Connections();
  virtual ~Connections();

  bool load(const std::string &configDir);

  bool updateConnection(wxDataViewItem &item, const Types::Connection &data);
  wxDataViewItem createConnection(const Types::Connection &data);
  Types::Connection getConnection(wxDataViewItem &item) const;

private:

  struct ConnectionInfo
  {
    Types::Connection connection;
    bool saved = false;
  };

  std::vector<ConnectionInfo*> mConnections;
  std::string mConnectionsDir;

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

  std::string toDir(const std::string &name) const;
};

}

#endif // TRANSMITRON_MODELS_CONNECTIONS_HPP
