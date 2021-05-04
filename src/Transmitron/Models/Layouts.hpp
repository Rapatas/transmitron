#ifndef TRANSMITRON_MODELS_LAYOUTS_HPP
#define TRANSMITRON_MODELS_LAYOUTS_HPP

#include <string>
#include <map>
#include <wx/arrstr.h>

namespace Transmitron::Models
{

class Layouts
{
public:

  wxArrayString getNames() const;

  std::string getUniqueName() const;

  std::optional<std::string> getLayout(const std::string &key) const;

  bool store(const std::string &key, const std::string &layout);

private:

  std::map<std::string, std::string> mLayouts;

};

}

#endif // TRANSMITRON_MODELS_LAYOUTS_HPP
