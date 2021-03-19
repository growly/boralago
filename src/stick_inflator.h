#ifndef STICK_INFLATOR_H_
#define STICK_INFLATOR_H_

#include <cstdint>
#include <vector>
#include <memory>
#include <unordered_map>

#include "point.h"
#include "poly_line.h"
#include "inflator_rules.pb.h"

namespace boralago {

class StickInflator {
 public:
  StickInflator(const InflatorRules &rules);

 private:
  std::unordered_map<uint64_t, InflatorRules> rules_by_layer_;
};

}  // namespace boralago

#endif  // STICK_INFLATOR_H_
