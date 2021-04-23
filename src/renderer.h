#ifndef RENDERER_H_
#define RENDERER_H_

#include <map>
#include <cstdint>
#include <string>

#include <core/SkPaint.h>
#include <core/SkColor.h>
#include <core/SkPoint.h>

#include "cell.h"
#include "point.h"
#include "port.h"
#include "poly_line_cell.h"
#include "routing_grid.h"

class SkCanvas;

namespace boralago {

class Renderer {
 public:
  Renderer(uint64_t width_px, uint64_t height_px)
      : width_px_(width_px),
        height_px_(height_px),
        lower_left_(Point(0, 0)),
        upper_right_(Point(1000, 1000)) {}

  void Fit(const Cell &cell);

  void FitWidth(const Cell &cell);
  void FitHeight(const Cell &cell);

  void RenderToPNG(
      const Cell &cell,
      const std::string &filename);

  void RenderToPNG(
      const PolyLineCell &poly_line_cell,
      const Cell &cell,
      const RoutingGrid &grid,
      const std::string &filename);

 private:
  void DrawPolyLineCell(const PolyLineCell &poly_line_cell, SkCanvas *canvas);
  void DrawPoint(
      const Point &point, uint64_t width, uint64_t height,
      const SkPaint &up_paint, const SkPaint &down_paint, SkCanvas *canvas);
  void DrawPort(const Port &port, SkCanvas *canvas);
  void DrawRoutingGrid(const RoutingGrid &grid, SkCanvas *canvas);
  void DrawCell(const Cell &cell, const Point &offset, SkCanvas *canvas);

  SkPoint MapToSkPoint(const Point &point);
  SkPoint MapToSkPoint(const Point &point, const Point &offset);
  SkColor MapLayerToSkColor(int64_t layer);
  const SkPaint &GetLayerPaint(int64_t layer);

  std::map<int64_t, SkPaint> paint_by_layer_;  

  uint64_t width_px_;
  uint64_t height_px_;

  Point lower_left_;
  Point upper_right_;
};

}  // namespace boralago

#endif  // RENDERER_H_
