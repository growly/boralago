#ifndef STICK_INFLATOR_H_
#define STICK_INFLATOR_H_

#include <cstdint>
#include <vector>
#include <memory>
#include <unordered_map>

#include "cell.h"
#include "point.h"
#include "line.h"
#include "polygon.h"
#include "poly_line.h"
#include "stick_cell.h"
#include "inflator_rules.pb.h"

namespace boralago {

class StickInflator {
 public:
  StickInflator(const InflatorRules &rules);

  // Return a laid-out version of the stick diagram.
  Cell Inflate(const StickCell &stick_cell);

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

#endif  // STICK_INFLATOR_H_
