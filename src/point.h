#ifndef POINT_H_
#define POINT_H_

#include <cstdint>
#include <vector>

namespace boralago {

class Point {
 public:
  Point(const int64_t x, const int64_t y)
      : x_(x),
        y_(y) {}

  const int64_t &x() { return x_; }
  const int64_t &y() { return y_; }

  void set_x(const int64_t &x) { x_ = x; }
  void set_y(const int64_t &y) { y_ = y; }

 private:
  int64_t x_;
  int64_t y_;
};

}  // namespace boralago

#endif  // POINT_H_
