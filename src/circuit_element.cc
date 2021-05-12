#include "circuit_element.h"

#include <glog/logging.h>
#include <string>
#include <set>
#include <vector>

namespace boralago {

void CircuitElement::AddPin(const std::string &name,
                            const std::set<std::string> &nets) {
  auto it = pins_by_name_.find(name);
  if (it != pins_by_name_.end()) {
    LOG(FATAL) << "Duplicate pin definition: " << name;
    return;
  }
  Pin *pin = new Pin{name, nets, Pin::kNoDirection};
  pins_.emplace_back(pin);
  pins_by_name_[name] = pin;
}

void CircuitElement::ConnectsTo(CircuitElement *element) {
  connects_.insert(element);
}

const std::set<std::string> &CircuitElement::GetNetsForPin(
    const std::string &pin_name) const {
  auto it = pins_by_name_.find(pin_name);
  if (it == pins_by_name_.end()) {
    LOG(FATAL) << "No such pin found: " << pin_name;
  }
  return it->second->nets;
}

}   // namespace boralago
