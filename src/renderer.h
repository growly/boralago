#ifndef RENDERER_H_
#define RENDERER_H_

#include <cstdint>
#include <string>

#include <core/SkColor.h>
#include <core/SkPoint.h>

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
      const std::string &filename);

 private:
  void DrawStickCell(const StickCell &stick_cell, SkCanvas *canvas);

  SkPoint MapToSkPoint(const Point &point);
  SkColor MapLayerToSkColor(int64_t layer);

  uint64_t width_px_;
  uint64_t height_px_;
};

}  // namespace boralago

#endif  // RENDERER_H_
