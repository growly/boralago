#ifndef PHYSICAL_PROPERTIES_DATABASE_H_
#define PHYSICAL_PROPERTIES_DATABASE_H_

#include <ostream>

#include "layer.h"

namespace boralago {

enum RoutingTrackDirection {
  kTrackHorizontal,
  kTrackVertical
};

struct RoutingLayerInfo {
  Layer layer;
  Rectangle area;
  int64_t wire_width;
  int64_t offset;
  RoutingTrackDirection direction;
  int64_t pitch;
};

struct LayerConnectionInfo {
  // Need some measure of cost for connecting between these two layers. Maybe
  // a function that describes the cost based on something (like length,
  // sheet resistance).
  double cost;
};


// Manages information about physical layout constraints.
class PhysicalPropertiesDatabase {
 public:
  PhysicalPropertiesDatabase() {}

 private:
  std::map<Layer, RoutingLayerInfo> layer_infos_;

  // Stores the connection info between the ith (first index) and jth (second
  // index) layers. The "lesser" layer (std::less) should always be used to
  // index first, so that half of the matrix can be avoided.
  std::map<Layer, std::map<Layer, LayerConnectionInfo>> connection_infos_;
};

std::ostream &operator<<(std::ostream &os, const PhysicalPropertiesDatabase &rectangle);

}  // namespace boralago

#endif  // PHYSICAL_PROPERTIES_DATABASE_H_
