#ifndef POLY_LINE_H_
#define POLY_LINE_H_

#include <cstdint>
#include <glog/logging.h>
#include <vector>

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

  void set_start(const Point &start) { start_ = start; }
  const Point &start() const { return start_; }

  void AddSegment(const Point &point) {
    AddSegment(point, 0);
  }

  void AddSegment(const Point &to, const uint64_t width) {
    int64_t last_x = segments_.empty() ? start_.x() : segments_.back().end.x();
    int64_t last_y = segments_.empty() ? start_.y() : segments_.back().end.y();
    if (to.x() != last_x && to.y() != last_y) {
      LOG(FATAL) << "PolyLine segments must be rectilinear. Make sure the new "
                 << "x == last_x or y == last_y.";
    }
    segments_.push_back(LineSegment{to, width});
  }

 private:
  Point start_;
  std::vector<LineSegment> segments_;
};

}  // namespace boralago

#endif  // POLY_LINE_H_
