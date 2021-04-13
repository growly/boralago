#ifndef ROUTING_GRID_H_
#define ROUTING_GRID_H_

#include "layer.h"
#include "point.h"
#include "rectangle.h"

#include <map>

namespace boralago {

enum RoutingDirection {
  kHorizontal,
  kVertical
};

class RoutingEdge;

class RoutingVertex {
 public:
  RoutingVertex(const Point &centre) : centre_(centre) {}

  void AddEdge(RoutingEdge *edge) { edges_.push_back(edge); }
  const std::vector<RoutingEdge*> &edges() { return edges_; }

  void AddConnectedLayer(const Layer &layer) {
    connected_layers_.push_back(layer);
  }
  const std::vector<Layer> &connected_layers() { return connected_layers_; }

 private:
  Point centre_;
  std::vector<Layer> connected_layers_;
  std::vector<RoutingEdge*> edges_;
};

class RoutingEdge {
 public:
  RoutingEdge(RoutingVertex *first, RoutingVertex *second)
    : first_(first), second_(second) {}

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

struct RoutingLayerInfo {
  Layer layer;
  Rectangle area;
  int64_t wire_width;
  int64_t offset;
  RoutingDirection direction;
  int64_t pitch;
};

class RoutingGrid {
 public:

  struct LayerConnectionInfo {
    // Need some measure of cost for connecting between these two layers. Maybe
    // a function that describes the cost based on something (like length,
    // sheet resistance).
    double cost;
  };

  // Stage the description of a layer that's usable for routing.
  void DescribeLayer(const Layer &layer, const RoutingLayerInfo &info);

  // Connecting two layers generates the graph that describes all the paths one
  // can take between them; concretely, it creates a vertex every time a
  // horizontal and vertical routing line cross. (The two described layers must
  // be orthogonal in routing direction.)
  void ConnectLayers(
      const Layer &first, const Layer &second, const LayerConnectionInfo &info);

 private:
  RoutingLayerInfo *FindRoutingInfoOrDie(const Layer &layer);
  std::pair<RoutingLayerInfo*, RoutingLayerInfo*> PickHorizontalAndVertical(
    const Layer &lhs, const Layer &rhs);
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
