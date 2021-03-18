#ifndef TRANSMITRON_TYPES_CONNECTION_HPP
#define TRANSMITRON_TYPES_CONNECTION_HPP

#include <filesystem>
#include <nlohmann/json.hpp>
#include <wx/log.h>
#include "Transmitron/Models/Snippets.hpp"
#include "Transmitron/ValueObjects/BrokerOptions.hpp"

namespace Transmitron::Types
{

class Connection
{
public:

  explicit Connection(
    std::string name,
    ValueObjects::BrokerOptions brokerOptions = ValueObjects::BrokerOptions{},
    bool saved = false,
    std::filesystem::path path = {}
  );
  virtual ~Connection() = default;

  const ValueObjects::BrokerOptions &getBrokerOptions() const;
  bool getSaved() const;
  std::string getName() const;
  std::filesystem::path getPath() const;
  const wxObjectDataPtr<Models::Snippets> getSnippetsModel();

  void setBrokerOptions(ValueObjects::BrokerOptions brokerOptions);
  void setSaved(bool saved);
  void setName(std::string name);

private:

  wxObjectDataPtr<Models::Snippets> mSnippetsModel;
  ValueObjects::BrokerOptions mBrokerOptions;
  bool mSaved;
  std::string mName;
  std::filesystem::path mPath;

};

}

#endif // TRANSMITRON_TYPES_CONNECTION_HPP
