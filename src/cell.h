#ifndef CELL_H_
#define CELL_H_

#include <vector>

#include "point.h"
#include "port.h"
#include "polygon.h"
#include "instance.h"

namespace boralago {

class Cell {
 public:
  Cell() = default;

  void AddPolygon(const Polygon &polygon) { polygons_.push_back(polygon); }
  void AddInstance(const Instance &instance) { instances_.push_back(instance); }
  void AddPort(const Port &port) { ports_.push_back(port); }

  const std::vector<Polygon> &polygons() const { return polygons_; }
  const std::vector<Instance> &instances() const { return instances_; }
  const std::vector<Port> &ports() const { return ports_; }

  const std::pair<Point, Point> GetBoundingBox() const;

 private:
  //std::vector<Rectangle> rectangles_;
  std::vector<Polygon> polygons_;
  std::vector<Port> ports_;

  std::vector<Instance> instances_;
};

}  // namespace boralago

#endif  // CELL_H_
