#ifndef ROUTING_GRID_H_
#define ROUTING_GRID_H_

#include "layer.h"
#include "point.h"
#include "port.h"
#include "rectangle.h"

#include <map>
#include <deque>
#include <vector>

namespace boralago {

class RoutingEdge;

// Need a way to invalidate all edges sharing a routing track after one chunk
// is used. This also needs to be able to tell us if we can use part of the
// track for small routes.
class RoutingTrack {
 
 
 private:
  Layer layer_;
  int64_t x_or_y_;
};

class RoutingVertex {
 public:
  RoutingVertex(const Point &centre) : centre_(centre) {}

  void AddEdge(RoutingEdge *edge) { edges_.push_back(edge); }
  const std::vector<RoutingEdge*> &edges() { return edges_; }

  uint64_t L1DistanceTo(const Point &point);

  void AddConnectedLayer(const Layer &layer) {
    connected_layers_.push_back(layer);
  }
  const std::vector<Layer> &connected_layers() { return connected_layers_; }

  void set_contextual_index(size_t index) { contextual_index_ = index; }
  size_t contextual_index() const { return contextual_index_; }

  const std::vector<RoutingEdge*> edges() const { return edges_; }

  // This is the const of connecting through this vertex (i.e. a via).
  double cost() const { return 1.0; }

 private:
  // This is a minor optimisation to avoid having to key things by pointer.
  // This index should be unique within the RoutingGrid that owns this
  // RoutingVertex for the duration of whatever process requires it.
  size_t contextual_index_;

  Point centre_;
  std::vector<Layer> connected_layers_;
  std::vector<RoutingEdge*> edges_;
};

class RoutingEdge {
 public:
  RoutingEdge(RoutingVertex *first, RoutingVertex *second)
    : first_(first), second_(second), cost_(1.0) {}

  void set_cost(double cost) { cost_ = cost; }
  double cost() const { return cost_; }

  RoutingVertex *first() { return first_; }
  RoutingVertex *second() { return second_; }

 private:
  RoutingVertex *first_;
  RoutingVertex *second_;

  // Need some function of the distance between the two vertices (like of
  // length, sheet resistance). This also needs to be computed only once...
  double cost_;
};

class RoutingPath {
 public:
  RoutingPath(const std::deque<RoutingEdge*> edges)
      : edges_(edges.begin(), edges.end()) {}

  RoutingVertex *begin() const {
    return Empty() ? nullptr : edges_.front()->first();
  }
  RoutingVertex *end() const {
    return Empty() ? nullptr : edges_.back()->second();
  }

  bool Empty() const { return edges_.empty(); }

 private:
  // The list of edges. Edge i connected vertices_[j] and vertices_[j+1].
  std::vector<RoutingEdge*> edges_;
};

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

class RoutingGrid {
 public:
  // Stage the description of a layer that's usable for routing.
  void DescribeLayer(const RoutingLayerInfo &info);

  // Connecting two layers generates the graph that describes all the paths one
  // can take between them; concretely, it creates a vertex every time a
  // horizontal and vertical routing line cross. (The two described layers must
  // be orthogonal in routing direction.)
  void ConnectLayers(
      const Layer &first, const Layer &second, const LayerConnectionInfo &info);

  bool AddRouteBetween(
      const Port &begin, const Port &end);

 private:
  RoutingLayerInfo *FindRoutingInfoOrDie(const Layer &layer);

  std::pair<RoutingLayerInfo*, RoutingLayerInfo*> PickHorizontalAndVertical(
    const Layer &lhs, const Layer &rhs);

  std::vector<RoutingVertex*> &GetAvailableVertices(const Layer &layer);

  RoutingVertex *FindNearestAvailableVertex(
      const Point &point, const Layer &layer);

  // Returns nullptr if no path found. If a RoutingPath is found, the caller
  // now owns the object.
  RoutingPath *ShortestPath(
      RoutingVertex *begin, RoutingVertex *end);

  // The list of all owned edges.
  std::vector<RoutingEdge*> edges_;

  // The list of all owned vertices.
  std::vector<RoutingVertex*> vertices_;

  // The list of all available vertices per layer.
  std::map<Layer, std::vector<RoutingVertex*>> available_vertices_by_layer_;

  std::map<Layer, RoutingLayerInfo> layer_infos_;
};

}  // namespace boralago

#endif  // ROUTING_GRID_H_
