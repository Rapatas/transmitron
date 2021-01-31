#ifndef TRANSMITRON_MODELS_SNIPPETS_HPP
#define TRANSMITRON_MODELS_SNIPPETS_HPP

#include <wx/dataview.h>

namespace Transmitron::Models
{

class Snippets :
  public wxDataViewModel
{
public:

  enum class Column : unsigned
  {
    Name,
    Max
  };

  explicit Snippets();
  virtual ~Snippets();

private:

  std::vector<std::string*> mSnippets;

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

};

}

#endif // TRANSMITRON_MODELS_SNIPPETS_HPP
