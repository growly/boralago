#ifndef POLYGON_H_
#define POLYGON_H_

#include <glog/logging.h>
#include <vector>

#include "point.h"

namespace boralago {

class Polygon {
 public:
  Polygon() = default;
  Polygon(const std::vector<Point> &vertices) {
    for (const auto &vertex : vertices) {
      vertices_.push_back(vertex);
    }
  }

  std::vector<Point> &vertices() { return vertices_; }

 private:
  std::vector<Point> vertices_;
};

}  // namespace boralago

#endif  // POLYGON_H_
