#ifndef POLYGON_H_
#define POLYGON_H_

#include <glog/logging.h>
#include <ostream>
#include <vector>

#include "point.h"
#include "shape.h"

namespace boralago {

class Polygon : public Shape {
 public:
  Polygon() = default;
  Polygon(const std::vector<Point> &vertices) {
    for (const auto &vertex : vertices) {
      vertices_.push_back(vertex);
    }
  }

  void AddVertex(const Point &point) {
    vertices_.push_back(point);
  }

  const std::pair<Point, Point> GetBoundingBox() const override;

  const std::vector<Point> &vertices() const { return vertices_; }

 private:
  std::vector<Point> vertices_;
};

std::ostream &operator<<(std::ostream &os, const Polygon &polygon);

}  // namespace boralago

#endif  // POLYGON_H_
