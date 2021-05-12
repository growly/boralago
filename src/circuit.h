#ifndef CIRCUIT_H_
#define CIRCUIT_H_

#include <set>
#include <memory>
#include <string>
#include <utility>

#include "circuit_element.h"

namespace boralago {

// Circuits are a collection of CircuitElements organised so as to be
// convenient. They should provide means to export/serialise the circuit
// description within recursively, since they may transitively contain other
// Circuits through instances of macros/black boxes in CircuitIntances.
class Circuit {
 public:
   // Take ownership of the element.
   void AddElement(CircuitElement *element);

 private:
   std::set<std::unique_ptr<CircuitElement>> elements_;
};

}   // namespace boralago

#endif  // CIRCUIT_H_
