#ifndef POLY_LINE_CELL_H_
#define POLY_LINE_CELL_H_

#include <vector>
#include <memory>

#include "point.h"
#include "poly_line.h"

namespace boralago {

class PolyLineCell {
 public:
  void AddPolyLine(const PolyLine &poly_line);
  PolyLine *AddPolyLine();

  const std::vector<std::unique_ptr<PolyLine>> &poly_lines() const {
    return poly_lines_;
  }

  std::vector<std::unique_ptr<PolyLine>> &poly_lines() { return poly_lines_; }

  const std::pair<Point, Point> GetBoundingBox() const;

 private:
  std::vector<std::unique_ptr<PolyLine>> poly_lines_;
};

}  // namespace boralago

#endif  // POLY_LINE_CELL_H_
