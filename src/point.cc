#include <ostream>

#include "point.h"

namespace boralago {

std::ostream &operator<<(std::ostream &os, const Point &point) {
  os << "(" << point.x() << ", " << point.y() << ")";
  return os;
}

}  // namespace boralago
