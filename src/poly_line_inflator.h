#ifndef POLY_LINE_INFLATOR_H_
#define POLY_LINE_INFLATOR_H_

#include <cstdint>
#include <vector>
#include <memory>
#include <unordered_map>

#include "cell.h"
#include "physical_properties_database.h"
#include "line.h"
#include "point.h"
#include "poly_line.h"
#include "poly_line_cell.h"
#include "polygon.h"
#include "via.h"

namespace boralago {

class PolyLineInflator {
 public:
  PolyLineInflator(const PhysicalPropertiesDatabase &physical_db)
      : physical_db_(physical_db) {}

  // Return a laid-out version of the poly_line diagram.
  Cell Inflate(const PolyLineCell &poly_line_cell);

  void InflateVia(const Via &via, Rectangle *rectangle);
  void InflatePolyLine(const PolyLine &line, Polygon *polygon);

 private:
  // Shift the given line consistently (relative to its bearing) by half the
  // 'width' amount. Add 'extension_source' to the start and 'extension_source'
  // to end of the line's length.
  Line *GenerateShiftedLine(
      const Line &source, double width,
      double extension_source, double extension_end);

  Line *GenerateShiftedLine(
      const Line &source, double width) {
    return GenerateShiftedLine(source, width, 0.0, 0.0);
  }

  // Shifts next_source by half of the given width, then add the intersection
  // of the new line with *last_shifted_line to polygon. Returns the newly
  // shifted line. If last_shifted_line is nullptr, the start of next_source is
  // used.
  std::unique_ptr<Line> ShiftAndAppendIntersection(
    const Line &next_source, double width, Line *last_shifted_line,
    Polygon *polygon);

  // Provides some defaults and rules.
  PhysicalPropertiesDatabase physical_db_;
};

}  // namespace boralago

#endif  // POLY_LINE_INFLATOR_H_
