#include <utility>

#include <glog/logging.h>

#include "layer.h"
#include "physical_properties_database.h"

namespace boralago {

namespace {

std::pair<const Layer&, const Layer&> OrderFirstAndSecondLayers(
    const Layer &lhs, const Layer &rhs) {
  const Layer &first = lhs <= rhs ? lhs : rhs;
  const Layer &second = rhs >= lhs ? rhs : lhs;
  return std::make_pair(first, second);
}

}   // namespace

void PhysicalPropertiesDatabase::AddLayer(
    const RoutingLayerInfo &info) {
  const Layer &layer = info.layer;
  auto layer_info_it = layer_infos_.find(layer);
  LOG_IF(FATAL, layer_info_it != layer_infos_.end())
      << "Duplicate layer: " << layer;
  layer_infos_.insert({layer, info});
}

const RoutingLayerInfo &PhysicalPropertiesDatabase::GetLayerInfo(
    const Layer &layer) const {
  auto lhs_info_it = layer_infos_.find(layer);
  LOG_IF(FATAL, lhs_info_it == layer_infos_.end())
      << "Could not find info for layer: " << layer;
  return lhs_info_it->second;
}

void PhysicalPropertiesDatabase::AddInterLayerInfo(
    const Layer &lhs,
    const Layer &rhs,
    const InterLayerInfo &info) {
  std::pair<const Layer&, const Layer&> ordered_layers =
      OrderFirstAndSecondLayers(lhs, rhs);
  // Order first and second.
  const Layer &first = ordered_layers.first;
  const Layer &second = ordered_layers.second;
  LOG_IF(FATAL,
      inter_layer_infos_.find(first) != inter_layer_infos_.end() &&
      inter_layer_infos_[first].find(second) != inter_layer_infos_[first].end())
      << "Attempt to specify InterLayerInfo for layers " << first << " and "
      << second << " again.";
  inter_layer_infos_[first][second] = info;
}

const InterLayerInfo &PhysicalPropertiesDatabase::GetInterLayerInfo(
    const Layer &lhs, const Layer &rhs) {
  std::pair<const Layer&, const Layer&> ordered_layers =
      OrderFirstAndSecondLayers(lhs, rhs);
  const Layer &first = ordered_layers.first;
  const Layer &second = ordered_layers.second;

  auto first_it = inter_layer_infos_.find(first);
  LOG_IF(FATAL, first_it == inter_layer_infos_.end())
      << "No known connectiion between layer " << first
      << " and layer " << second;
  std::map<Layer, InterLayerInfo> &inner_map = first_it->second;
  auto second_it = inner_map.find(second);
  LOG_IF(FATAL, second_it == inner_map.end())
      << "No known connectiion between layer " << first
      << " and layer " << second;
  return second_it->second;
}

std::ostream &operator<<(std::ostream &os,
                         const PhysicalPropertiesDatabase &rectangle) {
  os << "Physical properties database";
  return os;
}

}   // namespace boralago
