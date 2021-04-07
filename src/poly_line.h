#ifndef POLY_LINE_H_
#define POLY_LINE_H_

#include <cstdint>

#include "line_segment.h"
#include "point.h"
#include "shape.h"

namespace boralago {

class PolyLine : public Shape {
 public:
  PolyLine() = default;

  PolyLine(const std::vector<Point> &points) {
    if (points.empty()) return;

    start_ = points.front();
    for (size_t i = 1; i < points.size(); ++i) {
      segments_.push_back(LineSegment{points[i], 0});
    }
  }

  const std::pair<Point, Point> GetBoundingBox() const override;

  void set_start(const Point &start) { start_ = start; }
  const Point &start() const { return start_; }

  uint64_t overhang_start() const { return overhang_start_; }
  void set_overhang_start(uint64_t overhang) { overhang_start_ = overhang; }

  uint64_t overhang_end() const { return overhang_end_; }
  void set_overhang_end(uint64_t overhang) { overhang_end_ = overhang; }

  void AddSegment(const Point &point) {
    AddSegment(point, 0);
  }

  void AddSegment(const Point &to, const uint64_t width);

  const std::vector<LineSegment> &segments() const { return segments_; }

 private:
  Point start_;

  // How much to extend the line over the start/end segments.
  uint64_t overhang_start_;
  uint64_t overhang_end_;

  std::vector<LineSegment> segments_;
};

}  // namespace boralago

#endif  // POLY_LINE_H_
