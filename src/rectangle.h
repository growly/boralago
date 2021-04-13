#ifndef RECTANGLE_H_
#define RECTANGLE_H_

#include <ostream>

#include "shape.h"
#include "point.h"

namespace boralago {

// TODO(aryap): Do we need to have separate classes for a "rectangle", the
// object on a layer with connected nets and such, and a generic notion of a
// "rectangular region", which we use to do math? I mean, maybe. But
// laziness...
class Rectangle : public Shape {
 public:
  Rectangle(const Point &lower_left, uint64_t width, uint64_t height)
      : lower_left_(lower_left),
        upper_right_(lower_left + Point(width, height)),
        Shape(0, "") {}

  Rectangle(const Point &lower_left, const Point &upper_right)
      : lower_left_(lower_left),
        upper_right_(upper_right),
        Shape(0, "") {}

  Rectangle(const Point &lower_left, const Point &upper_right,
            const Layer &layer, const std::string &net)
      : lower_left_(lower_left),
        upper_right_(upper_right),
        Shape(layer, net) {}

  bool Overlaps(const Rectangle &other) const;
  const Rectangle OverlapWith(const Rectangle &other) const;

  const std::pair<Point, Point> GetBoundingBox() const override {
    return std::make_pair(lower_left_, upper_right_);
  }

  const Point &lower_left() const { return lower_left_; }
  void set_lower_left(const Point &lower_left) { lower_left_ = lower_left; }

  const Point &upper_right() const { return upper_right_; }
  void set_upper_right(const Point &upper_right) { upper_right_ = upper_right; }

 private:
  Point lower_left_;
  Point upper_right_;
};

std::ostream &operator<<(std::ostream &os, const Rectangle &rectangle);

}  // namespace boralago

#endif  // RECTANGLE_H_
