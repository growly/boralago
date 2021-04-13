#ifndef SHAPE_H_
#define SHAPE_H_

#include <string>
#include <utility>

#include "layer.h"
#include "point.h"

namespace boralago {

class Shape {
 public:
  Shape() = default;
  virtual ~Shape() = default;

  Shape(const Layer &layer, const std::string &net)
      : layer_(layer),
        net_(net) {}

  // Return the lower-left and upper-right points defining the bounding box
  // around this shape.
  virtual const std::pair<Point, Point> GetBoundingBox() const = 0;

  void set_layer(const Layer &layer) { layer_ = layer; }
  const Layer &layer() const { return layer_; }

  void set_net(const std::string &net) { net_ = net; }
  const std::string &net() { return net_; }

 private:
  Layer layer_;
  std::string net_;
};

}   // namespace boralago

#endif  // SHAPE_H_
