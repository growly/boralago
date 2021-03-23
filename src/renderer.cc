#include "renderer.h"

#include <cstdint>

#include <core/SkData.h>
#include <core/SkImage.h>
#include <core/SkStream.h>
#include <core/SkSurface.h>
#include <core/SkPath.h>
#include <core/SkCanvas.h>

#include "point.h"
#include "stick_cell.h"

namespace boralago {

SkPoint Renderer::MapToSkPoint(const Point &point) {
  // TODO(aryap): Probably some sort of scaling situation.
  SkScalar y = static_cast<SkScalar>(height_px_ - point.y());
  return SkPoint::Make(static_cast<SkScalar>(point.x()), y);
}

void Renderer::DrawStickCell(const StickCell &stick_cell, SkCanvas *canvas) {
  const SkScalar scale = 256.0f;

  SkPaint p;
  p.setStyle(SkPaint::kStroke_Style);
  p.setAntiAlias(true);

  canvas->clear(SK_ColorWHITE);
  //canvas->translate(0.5f * scale, 0.5f * scale);
  for (const auto &stick : stick_cell.sticks()) {
    SkPath path;
    path.moveTo(MapToSkPoint(stick->start()));
    for (const auto &segment : stick->segments()) {
      path.lineTo(MapToSkPoint(segment.end));
    }
    // path.close();
    canvas->drawPath(path, p);
  }
}

// void raster(int width, int height,
//             void (*draw)(SkCanvas*),
//             const char* path) {
//   // This code from the Skia tutorial!
//   sk_sp<SkSurface> rasterSurface =
//       SkSurface::MakeRasterN32Premul(width, height);
//   SkCanvas* rasterCanvas = rasterSurface->getCanvas();
//   draw(rasterCanvas);
//   sk_sp<SkImage> img(rasterSurface->makeImageSnapshot());
//   if (!img) { return; }
//   sk_sp<SkData> png(img->encodeToData());
//   if (!png) { return; }
//   SkFILEWStream out(path);
//   (void)out.write(png->data(), png->size());
// }

void Renderer::RenderToPNG(
    const StickCell &stick_cell,
    const std::string &filename) {
  // This code from the Skia tutorial!
  sk_sp<SkSurface> raster_surface =
      SkSurface::MakeRasterN32Premul(width_px_, height_px_);
  SkCanvas* raster_canvas = raster_surface->getCanvas();
  DrawStickCell(stick_cell, raster_canvas);
  sk_sp<SkImage> image(raster_surface->makeImageSnapshot());
  if (!image) { return; }
  sk_sp<SkData> png(image->encodeToData());
  if (!png) { return; }
  SkFILEWStream out(filename.c_str());
  out.write(png->data(), png->size());
}

}   // namespace boralago
