#include <vector>
#include <memory>

#include "stick_cell.h"

namespace boralago {

void StickCell::AddStick(const PolyLine &stick) {
  sticks_.emplace_back(new PolyLine(stick));
}

PolyLine *StickCell::AddStick() {
  PolyLine *stick = new PolyLine();
  sticks_.emplace_back(stick);
  return stick;
}

}  // namespace boralago
