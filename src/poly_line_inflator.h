#ifndef POLY_LINE_INFLATOR_H_
#define POLY_LINE_INFLATOR_H_

#include <cstdint>
#include <vector>
#include <memory>
#include <unordered_map>

#include "cell.h"
#include "point.h"
#include "line.h"
#include "polygon.h"
#include "poly_line.h"
#include "poly_line_cell.h"
#include "inflator_rules.pb.h"

namespace boralago {

class PolyLineInflator {
 public:
  PolyLineInflator(const InflatorRules &rules);

  // Return a laid-out version of the poly_line diagram.
  Cell Inflate(const PolyLineCell &poly_line_cell);

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

  std::unordered_map<uint64_t, InflatorRules> rules_by_layer_;
};

}  // namespace boralago

#endif  // POLY_LINE_INFLATOR_H_