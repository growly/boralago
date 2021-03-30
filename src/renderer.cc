#include "renderer.h"

#include <cstdint>
#include <map>
#include <random>

#include <glog/logging.h>

#include <core/SkData.h>
#include <core/SkColor.h>
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

SkColor Renderer::MapLayerToSkColor(int64_t layer) {
  //return SkColorSetARGB(0xff, layer*50, layer*50, layer*50);
  // Using cstdlib, something like:
  // int random_r = 80 + std::rand()/((RAND_MAX + 80u)/255);

  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> distrib(20, 200);

  return SkColorSetARGB(0xff, distrib(gen), distrib(gen), distrib(gen));
}

void Renderer::DrawStickCell(const StickCell &stick_cell, SkCanvas *canvas) {
  const SkScalar scale = 256.0f;

  std::map<int64_t, SkPaint> paint_by_layer;

  canvas->clear(SK_ColorWHITE);
  //canvas->translate(0.5f * scale, 0.5f * scale);
  canvas->translate(10.0f, -10.0f);
  for (const auto &stick : stick_cell.sticks()) {
    int64_t layer = stick->layer();
    auto paint_it = paint_by_layer.find(layer);
    if (paint_it == paint_by_layer.end()) {
      LOG(INFO) << "Creating new paint";
      SkPaint p;
      p.setStyle(SkPaint::kStroke_Style);
      p.setColor(MapLayerToSkColor(layer));
      p.setAntiAlias(true);
      auto insert_result = paint_by_layer.insert({layer, p});
      if (!insert_result.second) LOG(FATAL) << "Couldn't create SkPaint.";
      paint_it = insert_result.first;
    }

    SkPath path;
    path.moveTo(MapToSkPoint(stick->start()));
    for (const auto &segment : stick->segments()) {
      path.lineTo(MapToSkPoint(segment.end));
    }
    // path.close();
    canvas->drawPath(path, paint_it->second);
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
