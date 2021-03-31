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

#include "cell.h"
#include "point.h"
#include "poly_line_cell.h"

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

const SkPaint &Renderer::GetLayerPaint(int64_t layer) {
  auto paint_it = paint_by_layer_.find(layer);
  if (paint_it == paint_by_layer_.end()) {
    LOG(INFO) << "Creating new paint";
    SkPaint p;
    p.setStyle(SkPaint::kStroke_Style);
    p.setColor(MapLayerToSkColor(layer));
    p.setAntiAlias(true);
    auto insert_result = paint_by_layer_.insert({layer, p});
    if (!insert_result.second) LOG(FATAL) << "Couldn't create SkPaint.";
    paint_it = insert_result.first;
  }
  return paint_it->second;
}

void Renderer::DrawPolyLineCell(const PolyLineCell &poly_line_cell, SkCanvas *canvas) {
  //const SkScalar scale = 256.0f;
  //canvas->translate(0.5f * scale, 0.5f * scale);
  for (const auto &poly_line : poly_line_cell.poly_lines()) {
    const SkPaint &paint = GetLayerPaint(poly_line->layer()); 
    SkPath path;
    path.moveTo(MapToSkPoint(poly_line->start()));
    for (const auto &segment : poly_line->segments()) {
      path.lineTo(MapToSkPoint(segment.end));
    }
    // path.close();
    canvas->drawPath(path, paint);
  }
}

void Renderer::DrawCell(const Cell &cell, SkCanvas *canvas) {
  for (const auto &polygon : cell.polygons()) {
    const SkPaint &paint = GetLayerPaint(polygon.layer());
    SkPath path;
    path.moveTo(MapToSkPoint(polygon.vertices().front()));
    for (size_t i = 1; i < polygon.vertices().size(); ++i) {
      path.lineTo(MapToSkPoint(polygon.vertices().at(i)));
    }
    path.close();
    canvas->drawPath(path, paint);
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

// HACK HACK HACK
void Renderer::RenderToPNG(
    const PolyLineCell &poly_line_cell,
    const Cell &cell,
    const std::string &filename) {
  // This code from the Skia tutorial!
  sk_sp<SkSurface> raster_surface =
      SkSurface::MakeRasterN32Premul(width_px_, height_px_);
  SkCanvas* raster_canvas = raster_surface->getCanvas();

  raster_canvas->clear(SK_ColorWHITE);
  raster_canvas->translate(200.0f, -200.0f);
  DrawPolyLineCell(poly_line_cell, raster_canvas);
  DrawCell(cell, raster_canvas);

  sk_sp<SkImage> image(raster_surface->makeImageSnapshot());
  if (!image) { return; }
  sk_sp<SkData> png(image->encodeToData());
  if (!png) { return; }
  SkFILEWStream out(filename.c_str());
  out.write(png->data(), png->size());
}

}   // namespace boralago
