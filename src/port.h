#ifndef PORT_H_
#define PORT_H_

#include "layer.h"
#include "point.h"
#include "shape.h"

namespace boralago {

class Cell;

class Port : public Shape {
 public:
  // TODO(aryap): Wait, is a port just a rect with some other stuff? So this is
  // a rect:
  Port(const Point &centre, uint64_t width, uint64_t height,
       const Layer &layer, const std::string &net)
    : Shape(layer, net) {
    lower_left_ = Point(centre.x() - width / 2,
                        centre.y() - height / 2);
    upper_right_ = lower_left_ + Point(width, height);
  }

  Port(const Point &lower_left, const Point &upper_right,
       const Layer &layer, const std::string &net)
    : lower_left_(lower_left),
      upper_right_(upper_right),
      Shape(layer,  net) {}

  const std::pair<Point, Point> GetBoundingBox() const override {
    return std::make_pair(lower_left_, upper_right_);
  }

 private:
  Point lower_left_;
  Point upper_right_;
};

}  // namespace boralago

#endif  // PORT_H_
