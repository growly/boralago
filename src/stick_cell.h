#ifndef STICK_CELL_H_
#define STICK_CELL_H_

#include <vector>
#include <memory>

#include "point.h"
#include "poly_line.h"

namespace boralago {

class StickCell {
 public:
  void AddStick(const PolyLine &stick);
  PolyLine *AddStick();

  const std::vector<std::unique_ptr<PolyLine>> &sticks() const { return sticks_; }

  const std::pair<Point, Point> GetBoundingBox() const;

 private:
  std::vector<std::unique_ptr<PolyLine>> sticks_;
};

}  // namespace boralago

#endif  // STICK_CELL_H_
