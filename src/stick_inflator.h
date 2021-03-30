#ifndef STICK_INFLATOR_H_
#define STICK_INFLATOR_H_

#include <cstdint>
#include <vector>
#include <memory>
#include <unordered_map>

#include "point.h"
#include "polygon.h"
#include "poly_line.h"
#include "stick_cell.h"
#include "inflator_rules.pb.h"

namespace boralago {

class StickInflator {
 public:
  StickInflator(const InflatorRules &rules);

  // Return a laid-out version of the stick diagram.
  void Inflate(const StickCell &stick_cell);

  void InflatePolyLine(const PolyLine &line, Polygon *polygon);

 private:
  std::unordered_map<uint64_t, InflatorRules> rules_by_layer_;
};

}  // namespace boralago

#endif  // STICK_INFLATOR_H_
