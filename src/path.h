#ifndef WIRE_H_
#define WIRE_H_

#include <glog/logging.h>
#include <vector>

#include "point.h"

namespace boralago {

class Wire {
 public:
  Wire(const std::vector<Point> &points) {
    for (const auto &point : points) {
      points_.push_back(point);
    }
  }

 private:
  std::vector<Point> points_;
};

}  // namespace boralago

#endif  // WIRE_H_
