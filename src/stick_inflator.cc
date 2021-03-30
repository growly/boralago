#include "stick_inflator.h"

#include <glog/logging.h>
#include <memory>

#include "point.h"
#include "line.h"
#include "poly_line.h"
#include "polygon.h"
#include "stick_cell.h"
#include "inflator_rules.pb.h"

namespace boralago {

StickInflator::StickInflator(const InflatorRules &rules) {
  for (const auto &rules : rules.layer_rules()) {
    LOG(INFO) << "LOL";
  }
}

void StickInflator::Inflate(const StickCell &stick_cell) {
  for (const auto &stick : stick_cell.sticks()) {
    LOG_IF(FATAL, !stick) << "stick is nullptr?!";

    LOG(INFO) << "stick!";
    Polygon polygon;
    InflatePolyLine(*stick, &polygon);
  }
  LOG(INFO) << "INFLATE HERE.";
}

void StickInflator::InflatePolyLine(const PolyLine &polyline, Polygon *polygon) {
  LOG_IF(FATAL, polyline.segments().empty()) << "Inflating empty PolyLine";

  Point start = polyline.start();
  std::unique_ptr<Line> last_centre_line;
  std::vector<std::unique_ptr<Line>> outline_stack;

  for (size_t i = 0; i < polyline.segments().size(); ++i) {
    const PolyLine &segment = polyline.segments().at(i);
    std::unique_ptr<Line> line(new Line(start, segment.end));

    AnchorPosition growth_anchor;

    LineOrientation orientation = LineOrientation::kOther;
    if (next.x() == last.x()) {
      orientation = LineOrientation::kVertical;
      growth_anchor = AnchorPosition::kCenterVertical;
    } else if (next.y() == last.y()) {
      orientation = LineOrientation::kHorizontal;
      growth_anchor = AnchorPosition::kCenterHorizontal;
    }

    growth_anchor = segment.growth_anchor == AnchorPosition::kCenterAutomatic ?
        growth_anchor : segment.growth_anchor;

    uint64_t width = segment.width == 0 ? 100 : segment.width;
    // TODO(aryap): Integer division can lead to precision loss here,
    // so we make sure we recover it.
    uint64_t half_width = width/2;
    uint64_t remaining_width = width - half_width;

    Line low;
    Line high;

    // Determine which way the polyline is being inflated.
    switch (growth_anchor) {
      case AnchorPosition::kCenterVertical:
        low = Line(Point(last.x(), last.y() - half_width),
                   Point(next.x(), next.y() - half_width));
        high = Line(Point(last.x(), last.y() + remaining_width),
                    Point(next.x(), next.y() + remaining_width));
        break;
      case AnchorPosition::kCenterHorizontal:
        low = Line(Point(last.x() - half_width, last.y()),
                   Point(next.x() - half_width, next.y()));
        high = Line(Point(last.x() + remaining_width, last.y()),
                    Point(next.x() + remaining_width, next.y()));
      case AnchorPosition::kCenterAutomatic:
      default:
        LOG(FATAL) << "Unsupported growth_anchor value: " << growth_anchor;
    }

    if (!last_centre_line) {
      start = line->start();
      last_centre_line = std::move(line);
      continue;
    }

    // We have now inflated the stick into a rectangle. We have to find where
    // its corners join the previous segment's corners.
    //
    // There are two problems: 1) deciding which two lines to intersect;
    //                         2) finding their intersection.
    //
    // The inner and outer lines always intersect. 
    //
    // One way to determine which the "inner" and "outer" lines are is to
    // bisect the angle ABC between the two joining segments AB &  BC, creating
    // BD, then find the intersection E of the bisector BD with the line
    // created by joining the distant ends of the joining segments intersection
    // of the corner AC onto that line. The inner and outer lines can then be
    // measured by measuring their projection from E onto the line defined by
    // BD.
    //
    //     (A)
    //     +
    //     |\    (D)
    //     | -  /
    //     |  \/ (E)
    //     |  /\
    //     | /  -
    //     |/    \
    // (B) +------+ (C)
    //    /
    //   /
    //
    // The naive, and less computationally intensive, way to do this is just to
    // figure out which way the corner is turning expliclty:


    Point intersection;
    if (Line::Intersect(low, high, &intersection)) {
      LOG(INFO) << low.start() << ", " << low.end() << "; " << high.start() << ", " << high.end();
      LOG(INFO) << intersection;
    } else {
      LOG(INFO) << "no intersection";
    }

    start = segment.end;
    last_centre_line = std::move(line);
    //polygon->vertices() = {start_low, start_high, end_low, end_high};
  }
}

}  // namespace boralago
