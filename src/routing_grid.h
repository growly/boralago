#ifndef ROUTING_GRID_H_
#define ROUTING_GRID_H_

#include "layer.h"
#include "point.h"
#include "port.h"
#include "rectangle.h"

#include <map>
#include <set>
#include <deque>
#include <vector>

namespace boralago {

class RoutingEdge;
class RoutingTrack;

class RoutingVertex {
 public:
  RoutingVertex(const Point &centre) : available_(true), centre_(centre) {}

  void AddEdge(RoutingEdge *edge) { edges_.push_back(edge); }
  const std::vector<RoutingEdge*> &edges() { return edges_; }

  uint64_t L1DistanceTo(const Point &point);

  // This is the cost of connecting through this vertex (i.e. a via).
  double cost() const { return 1.0; }

  void AddConnectedLayer(const Layer &layer) {
    connected_layers_.push_back(layer);
  }
  const std::vector<Layer> &connected_layers() { return connected_layers_; }

  void set_contextual_index(size_t index) { contextual_index_ = index; }
  size_t contextual_index() const { return contextual_index_; }

  const std::vector<RoutingEdge*> edges() const { return edges_; }

  const Point &centre() const { return centre_; }

  void set_available(bool available) { available_ = available; }
  bool available() { return available_; }

  void set_horizontal_track(RoutingTrack *track) { horizontal_track_ = track; }
  RoutingTrack *horizontal_track() const { return horizontal_track_; }
  void set_vertical_track(RoutingTrack *track) { vertical_track_ = track; }
  RoutingTrack *vertical_track() const { return vertical_track_; }

 private:
  bool available_;
  RoutingTrack *horizontal_track_;
  RoutingTrack *vertical_track_;

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
    : available_(true), first_(first), second_(second), cost_(1.0) {}

  void set_cost(double cost) { cost_ = cost; }
  double cost() const { return cost_; }

  RoutingVertex *first() { return first_; }
  RoutingVertex *second() { return second_; }

  void set_available(bool available) { available_ = available; }
  bool available() { return available_; }

  void set_track(RoutingTrack *track) { track_ = track; }
  RoutingTrack *track() const { return track_; }

 private:
  bool available_;
  RoutingTrack *track_;

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

  //PolyLineCell *AsPolyLineCell() const;

  bool Empty() const { return edges_.empty(); }

  const std::vector<RoutingEdge*> edges() const { return edges_; }

 private:
  // The list of edges. Edge i connected vertices_[j] and vertices_[j+1].
  std::vector<RoutingEdge*> edges_;
};

enum RoutingTrackDirection {
  kTrackHorizontal,
  kTrackVertical
};

class RoutingTrackBlockage {
 public:
  RoutingTrackBlockage(int64_t start, int64_t end)
      : start_(start), end_(end) {
    LOG_IF(FATAL, end_ <= start_) << "RoutingTrackBlockage start must be before end.";
  }

  bool Contains(int64_t position);
  bool IsAfter(int64_t position);
  bool IsBefore(int64_t position);

  bool Blocks(int64_t low, int64_t high);

  void set_start(int64_t start) { start_ = start; }
  void set_end(int64_t end) { end_ = end; }

  int64_t start() const { return start_; }
  int64_t end() const { return end_; }

 private:
  int64_t start_;
  int64_t end_;
};

// RoutingTracks keep track of the edges, which are physical spans, that could
// fall on them. When such an edge is used for a route, the RoutingTrack
// determines which other edges must be invalidated.
//
// RoutingTracks do not own anything, but keep track of which vertices and
// edges associated with them. They invalidate those objects when they are used
// up.
class RoutingTrack {
 public:
  RoutingTrack(const Layer &layer,
               const RoutingTrackDirection &direction,
               int64_t offset)
      : layer_(layer), direction_(direction), offset_(offset) {}

  ~RoutingTrack() {
    for (RoutingTrackBlockage *blockage : blockages_) { delete blockage; }
  }

  // Takes ownership of the given pointer.
  void AddEdge(RoutingEdge *edge);
  void AddVertex(RoutingVertex *vertex);

  // 
  void MarkEdgeAsUsed(RoutingEdge *edge);

  void ReportAvailableEdges(std::vector<RoutingEdge*> *edges_out);
  void ReportAvailableVertices(std::vector<RoutingVertex*> *vertices_out);

  std::string Debug() const;

 private:
  bool IsBlocked(const Point &point) const { return IsBlocked(point, point); }
  bool IsBlocked(const Point &low_point, const Point &high_point) const;

  int64_t ProjectOntoTrack(const Point &point) const;

  void MergeBlockage(
      const Point &low_point, const Point &high_point);

  void SortBlockages();

  // The edges and vertices on this track.
  // These objects are NOT OWNED by this one.
  std::set<RoutingEdge*> edges_;
  std::set<RoutingVertex*> vertices_;

  Layer layer_;
  RoutingTrackDirection direction_;

  // The x or y coordinate for this track.
  int64_t offset_;

  // We want to keep a sorted list of blockages, but if we keep them as a std::set
  // we can't mutate the objects (since then no resorting is performed).
  // Instead we keep a vector and make sure to sort it ourselves.
  std::vector<RoutingTrackBlockage*> blockages_;
};

std::ostream &operator<<(std::ostream &os, const RoutingTrack &track);

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
  ~RoutingGrid() {
    for (auto entry : tracks_by_layer_) {
      for (RoutingTrack *track : entry.second) {
        delete track;
      }
    }
    for (RoutingPath *path : paths_) { delete path; }
    for (RoutingEdge *edge : edges_) { delete edge; }
    for (RoutingVertex *vertex : vertices_) { delete vertex; }
  }

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

  const std::vector<RoutingPath*> &paths() const { return paths_; }
  const std::vector<RoutingEdge*> &edges() const { return edges_; }
  const std::vector<RoutingVertex*> &vertices() const { return vertices_; }

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

  // Takes ownership of the given object and accounts for the path's resources
  // as used.
  void InstallPath(RoutingPath *path);

  void AddTrackToLayer(RoutingTrack *track, const Layer &layer);

  // All installed paths (which we also own).
  std::vector<RoutingPath*> paths_;

  std::vector<RoutingEdge*> edges_;
  std::vector<RoutingVertex*> vertices_;

  // All routing tracks (we own these).
  std::map<Layer, std::vector<RoutingTrack*>> tracks_by_layer_;

  // The list of all available vertices per layer.
  std::map<Layer, std::vector<RoutingVertex*>> available_vertices_by_layer_;

  std::map<Layer, RoutingLayerInfo> layer_infos_;

  // All routing tracks (which we own).
  std::map<Layer, std::vector<RoutingTrack*>> tracks_;

};

}  // namespace boralago

#endif  // ROUTING_GRID_H_
