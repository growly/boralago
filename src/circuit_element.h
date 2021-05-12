#ifndef CIRCUIT_ELEMENT_H_
#define CIRCUIT_ELEMENT_H_

#include <memory>
#include <set>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace boralago {

struct Pin {
  enum PinDirection {
    kNoDirection,  // TODO(aryap): Is this the same as "both"?
    kInput,
    kOutput,
  };

  std::string name;
  std::set<std::string> nets;
  PinDirection direction;
};

// Any circuit element has a some pins, in a defined order (even if that order
// is logical and not physically important), with some properties. Elements
// also have a bunch of key-value pairs.
class CircuitElement {
 public:
  CircuitElement() = default;
  virtual ~CircuitElement() = default;

  void AddPin(const std::string &name, const std::set<std::string> &nets);
  void ConnectsTo(CircuitElement *element);
  const std::set<std::string> &GetNetsForPin(
      const std::string &pin_name) const;

  const std::set<CircuitElement*> &connects() const { return connects_; }

 protected:
  // TODO(aryap): It strikes me that these will have different types, and so it
  // might be useful to have a parameter database that manages the types for
  // you. Using a proto will take care of this since multiple types can be
  // supported per message. This will do for now.
  std::unordered_map<std::string, std::string> parameters_;

  // The list of pins, in order.
  std::vector<std::unique_ptr<Pin>> pins_;

  // References to the pins in pins_ by their name.
  std::unordered_map<std::string, Pin*> pins_by_name_;

  // The circuit elements connecting to this one. Connections are made
  // according to pin names.
  std::set<CircuitElement*> connects_;
};

}   // namespace boralago

#endif  // CIRCUIT_ELEMENT_H_
