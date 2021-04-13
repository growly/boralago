#include <utility>
#include <map>

#include <glog/logging.h>

#include "routing_grid.h"
#include "rectangle.h"

namespace boralago {

void RoutingGrid::DescribeLayer(
    const Layer &layer, const RoutingLayerInfo &info) {
  auto layer_info_it = layer_infos_.find(layer);
  if (layer_info_it == layer_infos_.end()) {
    LOG(FATAL) << "Duplicate layer: " << layer;
  }
  layer_infos_.insert({layer, info});
}

RoutingLayerInfo *RoutingGrid::FindRoutingInfoOrDie(const Layer &layer) {
  auto lhs_info_it = layer_infos_.find(layer);
  if (lhs_info_it == layer_infos_.end()) {
    LOG(FATAL) << "Could not find info for layer: " << layer;
  }
  return &lhs_info_it->second;
}

// Return the (horizontal, vertical) routing infos.
std::pair<RoutingLayerInfo*, RoutingLayerInfo*>
    RoutingGrid::PickHorizontalAndVertical(
    const Layer &lhs, const Layer &rhs) {
  RoutingLayerInfo *lhs_info = FindRoutingInfoOrDie(lhs);
  RoutingLayerInfo *rhs_info = FindRoutingInfoOrDie(rhs);
  if (lhs_info->direction == RoutingDirection::kHorizontal &&
      rhs_info->direction == RoutingDirection::kVertical) {
    return std::make_pair(lhs_info, rhs_info);
  } else if (lhs_info->direction == RoutingDirection::kVertical &&
             rhs_info->direction == RoutingDirection::kHorizontal) {
    return std::make_pair(rhs_info, lhs_info);
  } else {
    LOG(FATAL) << "Exactly one of each layer must be horizontal and one must be"
               << "vertical: " << lhs << ", " << rhs;
  }
  return std::pair<RoutingLayerInfo*, RoutingLayerInfo*>(nullptr, nullptr);
}

void RoutingGrid::ConnectLayers(
    const Layer &first, const Layer &second, const LayerConnectionInfo &info) {
  // One layer has to be horizontal, and one has to be vertical.
  auto split_directions = PickHorizontalAndVertical(first, second);
  RoutingLayerInfo *horizontal_info = split_directions.first;
  RoutingLayerInfo *vertical_info = split_directions.second;

  // Determine the area over which the grid is valid.
  Rectangle overlap = horizontal_info->area.OverlapWith(vertical_info->area);
  LOG(INFO) << "Drawing grid between layers " << horizontal_info->layer << ", "
            << vertical_info->layer << " over " << overlap;
  
  //                      x_min v   v x_start
  //           |      |      |  +   |      |
  //           |      |      |  +   |      |
  //           |      |      |  +   |      |
  //           |      |      |  +   |      |
  //  origin   |      |      |  +   |      |
  //  O -----> | ---> | ---> | -+-> | ---> |
  //    offset   pitch          ^ start of grid boundary
  //
  int64_t x_offset = horizontal_info->offset;
  int64_t x_pitch = horizontal_info->pitch;
  int64_t x_min = overlap.lower_left().x();
  int64_t x_start = x_min + (x_pitch - ((x_min - x_offset) % x_pitch));
  int64_t x_max = overlap.upper_right().x();

  int64_t y_offset = vertical_info->offset;
  int64_t y_pitch = vertical_info->pitch;
  int64_t y_min = overlap.lower_left().y();
  int64_t y_start = y_min + (y_pitch - ((y_min - y_offset) % y_pitch));
  int64_t y_max = overlap.upper_right().y();

  std::vector<std::vector<RoutingVertex*>> vertex_by_ordinal;

  for (int64_t x = x_start; x < x_max; x += x_pitch) {
    vertex_by_ordinal.push_back({});
    std::vector<RoutingVertex*> &y_vertices = vertex_by_ordinal.back();
    for (int64_t y = y_start; y < y_max; y += y_pitch) {
      RoutingVertex *vertex = new RoutingVertex(Point(x, y));
      vertices_.push_back(vertex);  // The class owns all of these.
      vertex->AddConnectedLayer(first);
      vertex->AddConnectedLayer(second);
      y_vertices.push_back(vertex);
    }
  }

  for (size_t i = 0; i < vertex_by_ordinal.size(); ++i) {
    // The vertices in our column, for a given x (given by i).
    std::vector<RoutingVertex*> &y_vertices = vertex_by_ordinal[i];
    for (size_t j = 0; j < y_vertices.size(); ++j) {
      // Ever vertex gets an edge to every other vertex in its row and column.
      RoutingVertex *current = y_vertices[j];

      // Enumerate all the other vertices in this column. Start at j + 1 to
      // avoid duplicating edges. (Vertices below should already have created
      // an edge to this one.)
      for (size_t p = j + 1; p < vertex_by_ordinal.size(); ++p) {
        // RoutingVertex *other = vertex_by_oridinal[i][p];
        RoutingVertex *other = y_vertices[p];
        RoutingEdge *edge = new RoutingEdge(current, other);
        edges_.push_back(edge);  // The class owns all of these.
        current->AddEdge(edge);
        other->AddEdge(edge);
      }

      // Enumerate all the other vertices in this row. Again, start at i + 1 to
      // avoid duplicating edges.
      for (size_t q = i + 1; q < vertex_by_ordinal.size(); ++q) {
        RoutingVertex *other = vertex_by_ordinal[q][j];
        RoutingEdge *edge = new RoutingEdge{current, other};
        edges_.push_back(edge);  // The class owns all of these.
        current->AddEdge(edge);
        other->AddEdge(edge);
      }
    }
  }


}

} // namespace boralago
