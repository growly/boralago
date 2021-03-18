#ifndef SHAPE_H_
#define SHAPE_H_

#include "point.h"

namespace boralago {

class Shape {
 public:
  Shape() = default;

  Shape(const int64_t layer, const std::string &net)
      : layer_(layer),
        net_(net) {}

  void set_layer(const int64_t layer) { layer_ = layer; }
  int64_t layer() { return layer_; }

  void set_net(const std::string &net) { net_ = net; }
  const std::string &net() { return net_; }

 private:
  int64_t layer_;
  std::string net_;
};

}   // namespace boralago

#endif  // SHAPE_H_
