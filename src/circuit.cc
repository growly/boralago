#include "circuit.h"

#include <set>

namespace boralago {

void Circuit::AddElement(CircuitElement *element) {
  elements_.emplace(element);
}

}   // namespace boralago
