#include "Config.h"

static void
ListAllMaps(const std::string &prefix, const YAML::Node &node,
            std::list<std::pair<std::string, const YAML::Node>> &output) {
  if (prefix.find_first_not_of("abcdefghikjlmnopqrstuvwxyz._0123456789") !=
      std::string::npos) {
    SOLAR_LOG_ERROR(SOLAR_LOG_ROOT())
        << "Config invalid name: " << prefix << " : " << node;
    return;
  }
  output.push_back(std::make_pair(prefix, node));
  if (node.IsMap()) {
    for (auto it = node.begin(); it != node.end(); ++it) {
      ListAllMaps(prefix.empty() ? it->first.Scalar()
                                 : prefix + "." + it->first.Scalar(),
                  it->second, output);
    }
  }
}

namespace solar {
void Config::LoadFromYaml(const YAML::Node &root) {
  std::list<std::pair<std::string, const YAML::Node>> allNodes;
  ListAllMaps("", root, allNodes);
  for (auto &[key, node] : allNodes) {
    if (key.empty()) {
      continue;
    }
    std::transform(key.begin(), key.end(), key.begin(), ::tolower);
    ConfigVarBase::ptr var = LookupBase(key);
    if (var) {
      if (node.IsScalar()) {
        var->fromYaml(node.Scalar());
      } else {
        std::stringstream ss;
        ss << node;
        var->fromYaml(ss.str());
      }
    }
  }
}
ConfigVarBase::ptr Config::LookupBase(const std::string &name) {
  auto it = GetData().find(name);
  return it == GetData().end() ? nullptr : it->second;
}
} // namespace solar