#include "poly_line_inflator.h"

#include <glog/logging.h>
#include <cmath>
#include <memory>

#include "cell.h"
#include "point.h"
#include "line.h"
#include "poly_line.h"
#include "polygon.h"
#include "poly_line_cell.h"
#include "inflator_rules.pb.h"

namespace boralago {

PolyLineInflator::PolyLineInflator(const InflatorRules &rules) {
  for (const auto &rules : rules.layer_rules()) {
    LOG(INFO) << "LOL";
  }
}

Cell PolyLineInflator::Inflate(const PolyLineCell &poly_line_cell) {
  Cell cell;
  for (const auto &poly_line : poly_line_cell.poly_lines()) {
    LOG_IF(FATAL, !poly_line) << "poly_line is nullptr?!";

    Polygon polygon;
    InflatePolyLine(*poly_line, &polygon);
    auto bb = polygon.GetBoundingBox();
    LOG(INFO) << polygon << " bounded by ll= " << bb.first << " ur= " << bb.second;
    cell.AddPolygon(polygon);
  }
  return cell;
}

// So, you could do this in one pass by inflating every central poly_line into its
// bounding lines, but that would create two problems when joining one segment
// to its immediate neighbour:
//                         1) deciding which two lines to intersect;
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
// The more naive (and simple) way seems to be to walk down the segments in one
// direction and then back in the other. Treating them as vectors we can either
// keep track of the direction we're going in or reverse the start/end
// positions to reverse the vector itself. In either case, we take care to
// generate the shifted line in the same position relative to all vectors.
//
// Because we determine the orientation by finding the vector's angle to the
// horizon, we created a shifted copy of the vector in a consistent direction
// relative to the vector's own bearing.
void PolyLineInflator::InflatePolyLine(const PolyLine &polyline, Polygon *polygon) {
  LOG_IF(FATAL, polyline.segments().empty()) << "Inflating empty PolyLine";

  std::vector<Line> line_stack;
  std::unique_ptr<Line> last_shifted_line;

  Point start = polyline.start();
  // Turn the segments into Lines, so we can deal with them.
  for (const auto &segment : polyline.segments()) {
    line_stack.emplace_back(start, segment.end);
    Line &line = line_stack.back();

    // AnchorPosition growth_anchor;

    // LineOrientation orientation = LineOrientation::kOther;
    // if (segment.end.x() == start.x()) {
    //   orientation = LineOrientation::kVertical;
    //   growth_anchor = AnchorPosition::kCenterVertical;
    // } else if (segment.end.y() == start.y()) {
    //   orientation = LineOrientation::kHorizontal;
    //   growth_anchor = AnchorPosition::kCenterHorizontal;
    // }

    // growth_anchor = segment.growth_anchor == AnchorPosition::kCenterAutomatic ?
    //     growth_anchor : segment.growth_anchor;

    double width = segment.width == 0 ? 100 : static_cast<double>(segment.width);

    std::unique_ptr<Line> shifted_line(GenerateShiftedLine(line, width));
    LOG(INFO) << "Shifted " << line << " to " << *shifted_line;

    // Modify the last line we created to intersect with this new one.
    if (last_shifted_line != nullptr) {
      Point intersection;
      if (!Line::Intersect(*last_shifted_line, *shifted_line, &intersection)) {
        LOG(FATAL) << "The last line we created and the newly shifted line "
                   << "never intersect: " << *last_shifted_line << " and "
                   << *shifted_line;
      }
      polygon->AddVertex(intersection);
    } else {
      // Set the starting point.
      polygon->AddVertex(shifted_line->start());
    }
    last_shifted_line = std::move(shifted_line);
    start = segment.end;
  }
  polygon->AddVertex(last_shifted_line->end());

  last_shifted_line = nullptr;
  // TODO(aryap): lmao if you use size_t here it underflows and never breaks
  // the loop.
  for (int i = line_stack.size() - 1; i >= 0; --i) {
    Line &line = line_stack.at(i);
    line.Reverse();

    const LineSegment &segment = polyline.segments().at(i);
    double width = segment.width == 0 ? 100 : static_cast<double>(segment.width);

    std::unique_ptr<Line> shifted_line(GenerateShiftedLine(line, width));
    LOG(INFO) << "Shifted " << line << " to " << *shifted_line;

    if (last_shifted_line != nullptr) {
      Point intersection;
      if (!Line::Intersect(*last_shifted_line, *shifted_line, &intersection)) {
        LOG(FATAL) << "The last line we created and the newly shifted line "
                   << "never intersect: " << *last_shifted_line << " and "
                   << *shifted_line;
      }
      polygon->AddVertex(intersection);
    } else {
      // Set the starting point.
      polygon->AddVertex(shifted_line->start());
    }

    last_shifted_line = std::move(shifted_line);
  }
  // We flipped all the lines on the way back, so the last point is the 'end'
  // position of the first line in the list.
  polygon->AddVertex(last_shifted_line->end());
}

Line *PolyLineInflator::GenerateShiftedLine(
    const Line &source, double width,
    double extension_source, double extension_end) {
  // TODO(aryap): Integer division can lead to precision loss here,
  // so we make sure we recover it.
  double half_width = static_cast<double>(width) / 2.0;

  double theta = source.AngleToHorizon();

  int64_t shift_x = static_cast<int64_t>(std::sin(theta) * half_width);
  int64_t shift_y = static_cast<int64_t>(std::cos(theta) * half_width);

  Line *shifted_line = new Line(source);
  shifted_line->Shift(-shift_x, shift_y);

  if (extension_source > 0.0) {
    int64_t extension_x =
        static_cast<int64_t>(std::cos(theta) * extension_source);
    int64_t extension_y =
        static_cast<int64_t>(std::sin(theta) * extension_source);
    shifted_line->ShiftStart(extension_x, extension_y);
  }

  if (extension_end > 0.0) {
    int64_t extension_x =
        static_cast<int64_t>(std::cos(theta) * extension_end);
    int64_t extension_y =
        static_cast<int64_t>(std::sin(theta) * extension_end);
    shifted_line->ShiftEnd(extension_x, extension_y);
  }

  return shifted_line;
}

}  // namespace boralago