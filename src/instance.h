#ifndef INSTANCE_H_
#define INSTANCE_H_

#include "point.h"

namespace boralago {

class Cell;

class Instance {
 public:
  Instance(Cell *template_cell,
           const Point &lower_left)
      : template_cell_(template_cell), lower_left_(lower_left) {}

  const std::pair<Point, Point> GetBoundingBox() const;

  Cell *template_cell() const { return template_cell_; }
  const Point &lower_left() const { return lower_left_; }

 private:
  // This is the template cell.
  Cell *template_cell_;

  Point lower_left_;
};

}  // namespace boralago

#endif  // INSTANCE_H_
