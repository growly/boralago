#ifndef LINE_SEGMENT_H_
#define LINE_SEGMENT_H_

#include "point.h"

namespace boralago {

enum AnchorPosition {
  kCenterAutomatic,   // Grow wide when vertical, tall when horizontal.
  kBottom,
  kTop,
  kCenterHorizontal,
  kCenterVertical,
  kLeft,
  kRight
};

struct LineSegment {
  Point end;
  uint64_t width; // 0 -> unspecified; use default.
  AnchorPosition growth_anchor;
};

}   // namespace boralago

#endif  // LINE_SEGMENT_H_
