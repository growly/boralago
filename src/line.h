#ifndef LINE_H_
#define LINE_H_

#include "point.h"

namespace boralago {

class Line {
 public:
  Line() = default;
  Line(const Point &start, const Point &end)
      : start_(start), end_(end) {}

  // Returns true if the lines defined by lhs and rhs intersect, and if so,
  // fills `point` with the intersection point. Returns false if the lines do
  // not intersect (are parallel).
  static bool Intersect(const Line &lhs, const Line &rhs, Point *point);

  // Returns 'm' in y = m*x + c;
  double Gradient() const;

  // Returns 'c' in y = m*x + c;
  double Offset() const;

  const Point &start() const { return start_; }
  const Point &end() const { return end_; }

 private:
  Point start_;
  Point end_;
};

}   // namespace boralago

#endif  // LINE_H_
