#ifndef PHYSICAL_PROPERTIES_DATABASE_H_
#define PHYSICAL_PROPERTIES_DATABASE_H_

#include <map>
#include <ostream>

#include "via.h"
#include "layer.h"
#include "rectangle.h"

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

struct ViaInfo {
  // Vias have their own layer.
  Layer layer;
  // Need some measure of cost for connecting between these two layers. Maybe
  // a function that describes the cost based on something (like length,
  // sheet resistance).
  double cost;
  int64_t width;
  int64_t height;

  // TODO(aryap): I'm not sure how this generalises.
  int64_t overhang;
};


// Manages information about physical layout constraints.
class PhysicalPropertiesDatabase {
 public:
  PhysicalPropertiesDatabase()
      : internal_units_per_external_(0.001) {}

  // Internally, all positions and lengths are computed in integer units.
  // Externally to this program, the user probably expects real units, like
  // mircons or nanometres (i.e. not yards or inches or anything stupid like
  // that). When setting up a process, we must define the conversion factor
  // between external and internal units.
  int64_t ToInternalUnits(const int64_t external_value) const {
    return external_value * internal_units_per_external_;
  }
  int64_t ToExternalUnits(const int64_t internal_value) const {
    return internal_value / internal_units_per_external_;
  }


  void AddLayer(const RoutingLayerInfo &info);
  const RoutingLayerInfo &GetLayerInfo(const Layer &layer) const;

  void AddViaInfo(const Layer &lhs, const Layer &rhs, const ViaInfo &info);

  const ViaInfo &GetViaInfo(const Via &via) {
    return GetViaInfo(via.bottom_layer(), via.top_layer());
  }
  const ViaInfo &GetViaInfo(const Layer &lhs, const Layer &rhs);

 private:
  double internal_units_per_external_;

  std::map<Layer, RoutingLayerInfo> layer_infos_;

  // Stores the connection info between the ith (first index) and jth (second
  // index) layers. The "lesser" layer (std::less) should always be used to
  // index first, so that half of the matrix can be avoided.
  std::map<Layer, std::map<Layer, ViaInfo>> via_infos_;
};

std::ostream &operator<<(std::ostream &os,
                         const PhysicalPropertiesDatabase &rectangle);

}  // namespace boralago

#endif  // PHYSICAL_PROPERTIES_DATABASE_H_
