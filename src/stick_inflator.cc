#include "stick_inflator.h"

#include "inflator_rules.pb.h"

namespace boralago {

StickInflator::StickInflator(const InflatorRules &rules) {
  for (const auto &rules : rules.layer_rules()) {
    LOG(INFO) << "LOL";
  }
}

}  // namespace boralago
