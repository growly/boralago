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
#include "stick_cell.h"

class SkCanvas;

namespace boralago {

class Renderer {
 public:
  Renderer(uint64_t width_px, uint64_t height_px)
      : width_px_(width_px),
        height_px_(height_px) {
  }

  void RenderToPNG(
      const StickCell &stick_cell,
      const Cell &cell,
      const std::string &filename);

 private:
  void DrawStickCell(const StickCell &stick_cell, SkCanvas *canvas);

  void DrawCell(const Cell &cell, SkCanvas *canvas);

  SkPoint MapToSkPoint(const Point &point);
  SkColor MapLayerToSkColor(int64_t layer);
  const SkPaint &GetLayerPaint(int64_t layer);

  std::map<int64_t, SkPaint> paint_by_layer_;  

  uint64_t width_px_;
  uint64_t height_px_;
};

}  // namespace boralago

#endif  // RENDERER_H_
