#ifndef NODE_H_
#define NODE_H_

#include <string>
#include <utility>

#include "circuit_element.h"

namespace boralago {

class Node : public CircuitElement {
 public:
  enum NodeType {
    kInstance,
    kResistor,
    kCapacitor,
    kMos,
    kIsrc,
    kVsrc,
    kDiode
  };

  Node(const NodeType &type);

 private:
  NodeType type_;
};

}   // namespace boralago

#endif  // NODE_H_
