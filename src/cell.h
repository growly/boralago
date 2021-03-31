#ifndef CELL_H_
#define CELL_H_

#include <vector>

#include "point.h"
#include "polygon.h"

namespace boralago {

class Cell {
 public:
  Cell() = default;

  void AddPolygon(const Polygon &polygon) { polygons_.push_back(polygon); }

  const std::vector<Polygon> &polygons() const { return polygons_; }

 private:
  //std::vector<Rectangle> rectangles_;
  std::vector<Polygon> polygons_;
};

}  // namespace boralago

#endif  // CELL_H_
