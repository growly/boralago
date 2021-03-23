#include "stick_inflator.h"

#include <glog/logging.h>

#include "point.h"
#include "poly_line.h"
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
    LOG(INFO) << "stick!";
  }
  LOG(INFO) << "INFLATE HERE.";
}

}  // namespace boralago
