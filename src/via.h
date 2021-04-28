#ifndef VIA_H_
#define VIA_H_

#include <ostream>

#include "layer.h"
#include "shape.h"
#include "point.h"

namespace boralago {

// Abstract representation of a via.
class Via : public Shape {
 public:
  Via(const Point &centre, const Layer &bottom, const Layer &top)
      : Shape(0, ""),
        centre_(centre),
        bottom_layer_(bottom),
        top_layer_(top) {}

  const Layer &bottom_layer() const { return bottom_layer_; }
  const Layer &top_layer() const { return top_layer_; }

 private:
  Point centre_;
  Layer bottom_layer_;
  Layer top_layer_;
};

std::ostream &operator<<(std::ostream &os, const Via &rectangle);

}  // namespace boralago

#endif  // VIA_H_
