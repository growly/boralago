#include <glog/logging.h>

#include "line.h"
#include "point.h"

namespace boralago {

bool Line::Intersect(const Line &lhs, const Line &rhs, Point *point) {
  // (1) y1 = m1*x1 + c1
  // (2) y2 = m2*x2 + c2
  //
  // Set y's equal and solve for x:
  // xx = (c1 - c2)/(m2 - m1)
  // yy = m1*xx + c1
 
  if (lhs.end().x() == lhs.start().x()) {
    // The left line is vertical.
    if (rhs.end().x() == rhs.start().x()) {
      // Both lines are vertical.
      return false;
    }
    double x = static_cast<double>(lhs.end().x());
    double y = rhs.Gradient() * x + rhs.Offset();
    LOG(INFO) << "lhs is vertical at x = " << x
              << " rhs crosses at y = " << y;
    *point = Point(static_cast<int64_t>(x), static_cast<int64_t>(y));
    return true;
  } else if (rhs.end().x() == rhs.start().x()) {
    // Only the other line is vertical.
    double x = static_cast<double>(rhs.end().x());
    double y = lhs.Gradient() * x + lhs.Offset();
    LOG(INFO) << "rhs is vertical at x = " << x
              << " lhs crosses at y = " << y;
    *point = Point(static_cast<int64_t>(x), static_cast<int64_t>(y));
    return true;
  }

  double m1 = lhs.Gradient();
  double c1 = lhs.Offset();

  LOG(INFO) << lhs.start() << " -> " << lhs.end()
            << ": y1 = " << m1 << "*x1 + " << c1;

  double m2 = rhs.Gradient();
  double c2 = rhs.Offset();

  LOG(INFO) << rhs.start() << " -> " << rhs.end()
            << ": y2 = " << m2 << "*x2 + " << c2;

  if (m1 == m2) {
    // Line are parallel.
    return false;
  }

  double x = (c1 - c2)/(m2 - m1);
  double y = m1*x + c1;
  LOG(INFO) << "intersection: " << x << ", " << y;
  *point = Point(static_cast<int64_t>(x), static_cast<int64_t>(y));
  return true;
}

double Line::Gradient() const {
  return static_cast<double>(end_.y() - start_.y()) /
         static_cast<double>(end_.x() - start_.x());
}

double Line::Offset() const {
  return end_.y() - Gradient()*end_.x();
}

} // namespace boralago