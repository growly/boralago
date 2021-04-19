#include <algorithm>
#include <utility>
#include <cmath>
#include <map>
#include <memory>
#include <deque>
#include <queue>
#include <limits>
#include <map>
#include <vector>
#include <utility>

#include <absl/strings/str_join.h>
#include <glog/logging.h>

#include "routing_grid.h"
#include "rectangle.h"

namespace boralago {

uint64_t RoutingVertex::L1DistanceTo(const Point &point) {
  // The L-1 norm, or Manhattan distance.
  int64_t dx = point.x() - centre_.x();
  int64_t dy = point.y() - centre_.y();
  return std::abs(dx) + std::abs(dy);
}

void RoutingGrid::DescribeLayer(
    const RoutingLayerInfo &info) {
  const Layer &layer = info.layer;
  auto layer_info_it = layer_infos_.find(layer);
  if (layer_info_it != layer_infos_.end()) {
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
  if (lhs_info->direction == RoutingTrackDirection::kTrackHorizontal &&
      rhs_info->direction == RoutingTrackDirection::kTrackVertical) {
    return std::make_pair(lhs_info, rhs_info);
  } else if (lhs_info->direction == RoutingTrackDirection::kTrackVertical &&
             rhs_info->direction == RoutingTrackDirection::kTrackHorizontal) {
    return std::make_pair(rhs_info, lhs_info);
  } else {
    LOG(FATAL) << "Exactly one of each layer must be horizontal and one must be"
               << "vertical: " << lhs << ", " << rhs;
  }
  return std::pair<RoutingLayerInfo*, RoutingLayerInfo*>(nullptr, nullptr);
}

RoutingVertex *RoutingGrid::FindNearestAvailableVertex(
    const Point &point, const Layer &layer) {
  // A key function of this class is to determine an appropriate starting point
  // on the routing grid for routing to/from an arbitrary point.
  //
  // If constrained to one or two layers on a fixed grid, we can determine the
  // nearest vertices quickly by shortlisting those vertices whose positions
  // would correspond to the given point by construction (since we also
  // construct the grid).
  //
  // The more general solution, of finding the nearest vertex across any number
  // of layers, requires us to sort all available vertices by their proximity
  // to the position. This can be quite expensive. Also, there remains the
  // question of whether the vertex we find can be routed to.
  //
  // The first cut of this algorithm is to just find the closest of all the
  // available vertices on the given layer.

  auto it = available_vertices_by_layer_.find(layer);
  if (it == available_vertices_by_layer_.end()) {
    LOG(FATAL) << "Could not find a list of available vertices on layer: "
               << layer;
  }

  if (it->second.empty())
    return nullptr;

  std::vector<std::pair<uint64_t, RoutingVertex*>> costed_vertices;
  for (RoutingVertex *vertex : it->second) {
    uint64_t vertex_cost = vertex->L1DistanceTo(point);
    costed_vertices.emplace_back(vertex_cost, vertex);
  }

  // Should sort automatically based on operator< for first and second entries
  // in pairs.
  std::sort(costed_vertices.begin(), costed_vertices.end());

  return costed_vertices.front().second;
}

std::vector<RoutingVertex*> &RoutingGrid::GetAvailableVertices(
    const Layer &layer) {
  auto it = available_vertices_by_layer_.find(layer);
  if (it == available_vertices_by_layer_.end()) {
    auto insert_it = available_vertices_by_layer_.insert({layer, {}});
    LOG_IF(FATAL, !insert_it.second)
        << "Couldn't create entry for layer " << layer
        << " in available vertices map.";
    it = insert_it.first;
  }
  // Gotta dereference the iterator to get to the goods!
  return it->second;
}

namespace {

// C++ modulo is more 'remainder' than 'modulo' because of how negative numbers
// are handled:
//    mod(-3, 5) = 2
//    rem(-3, 5) = -3 (since -3 / 5 = 0)
// So we have to do this:
int64_t modulo(int64_t a, int64_t b) {
  int64_t remainder = a % b;
  return remainder < 0? remainder + b : remainder;
}


}   // namespace

// TODO(aryap): Do we need to protect against "connect"ing the same layers
// multiple times?
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
  int64_t x_start = x_min + (x_pitch - modulo(x_min - x_offset, x_pitch));
  int64_t x_max = overlap.upper_right().x();
  
  int64_t y_offset = vertical_info->offset;
  int64_t y_pitch = vertical_info->pitch;
  int64_t y_min = overlap.lower_left().y();
  int64_t y_start = y_min + (y_pitch - modulo(y_min - y_offset, y_pitch));
  int64_t y_max = overlap.upper_right().y();

  std::vector<std::vector<RoutingVertex*>> vertex_by_ordinal;
  std::vector<RoutingVertex*> &first_layer_vertices =
      GetAvailableVertices(first);
  std::vector<RoutingVertex*> &second_layer_vertices =
      GetAvailableVertices(second);

  size_t num_edges = 0;
  size_t num_vertices = 0;

  for (int64_t x = x_start; x < x_max; x += x_pitch) {
    vertex_by_ordinal.push_back({});
    std::vector<RoutingVertex*> &y_vertices = vertex_by_ordinal.back();
    for (int64_t y = y_start; y < y_max; y += y_pitch) {
      RoutingVertex *vertex = new RoutingVertex(Point(x, y));
      vertices_.push_back(vertex);  // The class owns all of these.
      ++num_vertices;
      vertex->AddConnectedLayer(first);
      first_layer_vertices.push_back(vertex);
      vertex->AddConnectedLayer(second);
      second_layer_vertices.push_back(vertex);
      LOG(INFO) << "Vertex created: " << vertex->centre() << " on layers: "
                << absl::StrJoin(vertex->connected_layers(), ", ");
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
        ++num_edges;
        edges_.push_back(edge);  // The class owns all of these.
        current->AddEdge(edge);
        other->AddEdge(edge);
      }

      // Enumerate all the other vertices in this row. Again, start at i + 1 to
      // avoid duplicating edges.
      for (size_t q = i + 1; q < vertex_by_ordinal.size(); ++q) {
        RoutingVertex *other = vertex_by_ordinal[q][j];
        RoutingEdge *edge = new RoutingEdge{current, other};
        ++num_edges;
        edges_.push_back(edge);  // The class owns all of these.
        current->AddEdge(edge);
        other->AddEdge(edge);
      }
    }
  }

  LOG(INFO) << "Connected layer " << first << " and " << second << "; "
            << "generated " << num_vertices << " vertices and "
            << num_edges << " edges.";
}

bool RoutingGrid::AddRouteBetween(
    const Port &begin, const Port &end) {
  RoutingVertex *begin_vertex = FindNearestAvailableVertex(
      begin.centre(), begin.layer());
  if (!begin_vertex) {
    LOG(ERROR) << "Could not find available vertex for begin port.";
    return false;
  }
  RoutingVertex *end_vertex = FindNearestAvailableVertex(
      end.centre(), end.layer());
  if (!end_vertex) {
    LOG(ERROR) << "Could not find available vertex for end port.";
    return false;
  }

  std::unique_ptr<RoutingPath> shortest_path(
      ShortestPath(begin_vertex, end_vertex));

  if (!shortest_path) {
    LOG(WARNING) << "No path found.";
    return false;
  }

  paths_.push_back(shortest_path.release());
  return true;
}

RoutingPath *RoutingGrid::ShortestPath(
    RoutingVertex *begin, RoutingVertex *end) {
  // Give everything its index for the duration of this algorithm.
  for (size_t i = 0; i < vertices_.size(); ++i) {
    vertices_[i]->set_contextual_index(i);
  }

  std::vector<double> cost(vertices_.size());

  // Recording the edge to take back to the start that makes the shortest path,
  // as well as the vertex it leads to. If RoutingEdge* is nullptr then this is
  // invalid.
  std::vector<std::pair<size_t, RoutingEdge*>> prev(vertices_.size());

  // All vertices sorted according to their cost.
  std::vector<RoutingVertex*> queue;

  size_t begin_index = begin->contextual_index();
  size_t end_index = end->contextual_index();

  cost[begin_index] = 0;

  for (size_t i = 0; i < vertices_.size(); ++i) {
    RoutingVertex *vertex = vertices_[i];
    LOG_IF(FATAL, i != vertex->contextual_index())
      << "Vertex " << i << " no longer matches its index "
      << vertex->contextual_index();
    prev[i].second = nullptr;
    if (i == begin_index)
      continue;
    cost[i] = std::numeric_limits<double>::max();
  }

  queue.push_back(begin);

  while (!queue.empty()) {
    // Have to resort the queue so that new cost changes take effect. (The
    // queue is already mostly sorted so an insertion sort will be fast.)
    std::sort(queue.begin(), queue.end(),
              [&](RoutingVertex *a, RoutingVertex *b) {
      // We want the lowest value at the back of the array.
      return cost[a->contextual_index()] > cost[b->contextual_index()];
    });

    RoutingVertex *current = queue.back();
    queue.pop_back();
    size_t current_index = current->contextual_index();

    if (current == end) {
      break;
    }

    //seen.add(current);

    for (RoutingEdge *edge : current->edges()) {
      // Searching outward, always check the 2nd ("far") vertex of an edge.
      RoutingVertex *next = edge->second();
      size_t next_index = next->contextual_index();
      double next_cost = cost[current_index] + edge->cost() + next->cost();

      if (next_cost < cost[next_index]) {
        cost[next_index] = next_cost;
        prev[next_index] = std::make_pair(current_index, edge);

        // Since we now have a faster way to get to this edge, we should visit it.
        queue.push_back(next);
      }
    }
  }

  std::deque<RoutingEdge*> shortest_edges;

  size_t last_index = prev[end_index].first;
  RoutingEdge *last_edge = prev[end_index].second;

  while (last_edge != nullptr) {
    LOG_IF(FATAL, last_edge->first() != vertices_[last_index])
        << "last_edge does not land back at source vertex";

    shortest_edges.push_front(last_edge);

    if (last_index == begin_index) {
      // We found our way back.
      break;
    }

    auto &last_entry = prev[last_index];
    last_index = last_entry.first;
    last_edge = last_entry.second;
  }

  if (shortest_edges.empty()) {
    return nullptr;
  } else if (shortest_edges.front()->first() != begin) {
    LOG(FATAL) << "Did not find beginning vertex.";
    return nullptr;
  }

  RoutingPath *path = new RoutingPath(shortest_edges);
  return path;
}

} // namespace boralago
