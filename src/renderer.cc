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
#include <core/SkRect.h>
#include <core/SkCanvas.h>

#include "cell.h"
#include "point.h"
#include "poly_line_cell.h"

namespace boralago {

SkPoint Renderer::MapToSkPoint(const Point &point) {
  return MapToSkPoint(point, Point(0, 0));
}

SkPoint Renderer::MapToSkPoint(const Point &point, const Point &offset) {
  // TODO(aryap): Probably some sort of scaling situation.
  Point with_offset = point + offset;
  SkScalar y = static_cast<SkScalar>(height_px_ - with_offset.y());
  SkPoint mapped_point = SkPoint::Make(static_cast<SkScalar>(with_offset.x()), y);
  VLOG(10) << "Mapped " << point << " to (" << mapped_point.fX << ", " 
           << mapped_point.fY << ")";
  return mapped_point;
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
    VLOG(10) << "Creating new paint";
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

void Renderer::DrawPoint(
    const Point &point, uint64_t width, uint64_t height,
    const SkPaint &up_paint, const SkPaint &down_paint, SkCanvas *canvas) {
  SkPoint lower_left(
      MapToSkPoint(
          Point(point.x() - width / 2,
                point.y() - height / 2)));
  SkPoint upper_right(
      MapToSkPoint(
          Point(point.x() + width / 2,
                point.y() + height / 2)));
  SkPoint upper_left = SkPoint::Make(lower_left.x(), upper_right.y());
  SkPoint lower_right = SkPoint::Make(upper_right.x(), lower_left.y());
  canvas->drawLine(lower_left, upper_right, up_paint);
  canvas->drawLine(upper_left, lower_right, down_paint);
}

// This is actually the same as drawing a rectangle.
void Renderer::DrawPort(const Port &port, SkCanvas *canvas) {
  SkPoint lower_left = MapToSkPoint(port.lower_left());
  SkPoint upper_right = MapToSkPoint(port.upper_right());
  SkRect rectangle = SkRect::MakeLTRB(
      lower_left.x(), upper_right.y(), upper_right.x(), lower_left.y());
  const SkPaint &paint = GetLayerPaint(port.layer());
  canvas->drawRect(rectangle, paint);
}

void Renderer::DrawCell(
    const Cell &cell, const Point &offset, SkCanvas *canvas) {
  for (const auto &polygon : cell.polygons()) {
    const SkPaint &paint = GetLayerPaint(polygon.layer());
    SkPath path;
    path.moveTo(MapToSkPoint(polygon.vertices().front(), offset));
    for (size_t i = 1; i < polygon.vertices().size(); ++i) {
      path.lineTo(MapToSkPoint(polygon.vertices().at(i), offset));
    }
    path.close();
    canvas->drawPath(path, paint);
  }

  // Draw cell boundary.
  std::pair<Point, Point> bounding_box = cell.GetBoundingBox();
  SkPoint lower_left = MapToSkPoint(bounding_box.first, offset);
  SkPoint upper_right = MapToSkPoint(bounding_box.second, offset);

  SkPaint bounding_paint;
  bounding_paint.setStyle(SkPaint::kStroke_Style);
  bounding_paint.setColor(SkColors::kBlue);
  SkRect bounding_rectangle = SkRect::MakeLTRB(
      lower_left.x(),
      upper_right.y(),
      upper_right.x(),
      lower_left.y());
  LOG(INFO) << "Drawing bounding box for cell: "
            << bounding_box.first << " " << bounding_box.second;
  canvas->drawRect(bounding_rectangle, bounding_paint);

  // Draw child cells.
  for (const auto &instance : cell.instances()) {
    DrawCell(*instance.template_cell(), instance.lower_left(), canvas);
  }
}

void Renderer::DrawRoutingGrid(
    const RoutingGrid &grid, SkCanvas *canvas) {
  static const double kVertexWidth = 25;
  static const double kVertexHeight = 25;

  // Draw vertices.
  LOG(INFO) << "Grid has " << grid.vertices().size() << " vertices.";
  for (RoutingVertex *vertex : grid.vertices()) {
    if (vertex->connected_layers().size() < 2) {
      LOG(WARNING) << "Vertex has " << vertex->connected_layers().size()
                   << " connected layers: " << vertex;
      continue;
    }
    VLOG(10) << "Drawing vertex " << vertex << " at " << vertex->centre();

    const SkPaint &first_paint = GetLayerPaint(
        vertex->connected_layers()[0]);
    const SkPaint &second_paint = GetLayerPaint(
        vertex->connected_layers()[1]);

    // Draw a cross through the centre.
    DrawPoint(vertex->centre(), kVertexWidth, kVertexHeight,
              first_paint, second_paint, canvas);
  }

  // Draw paths.
  LOG(INFO) << "Grid has " << grid.paths().size() << " paths.";
  for (RoutingPath *path : grid.paths()) {
    SkPath sk_path;
    RoutingVertex *first = path->vertices().front();
    sk_path.moveTo(MapToSkPoint(first->centre()));
    for (auto iter = std::next(path->vertices().begin());
         iter != path->vertices().end(); ++iter) {
      sk_path.lineTo(MapToSkPoint((*iter)->centre()));
    }

    // TODO(aryap): Actually you want each edge to have its layer colour...
    SkPaint path_paint;
    path_paint.setStyle(SkPaint::kStroke_Style);
    path_paint.setColor(SkColors::kRed);

    canvas->drawPath(sk_path, path_paint);
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
    const RoutingGrid &grid,
    const std::string &filename) {
  // This code from the Skia tutorial!
  sk_sp<SkSurface> raster_surface =
      SkSurface::MakeRasterN32Premul(width_px_, height_px_);
  SkCanvas* raster_canvas = raster_surface->getCanvas();

  raster_canvas->clear(SK_ColorWHITE);
  raster_canvas->translate(200.0f, -200.0f);
  DrawPolyLineCell(poly_line_cell, raster_canvas);
  DrawCell(cell, Point(0, 0), raster_canvas);
  DrawRoutingGrid(grid, raster_canvas);

  sk_sp<SkImage> image(raster_surface->makeImageSnapshot());
  if (!image) { return; }
  sk_sp<SkData> png(image->encodeToData());
  if (!png) { return; }
  SkFILEWStream out(filename.c_str());
  out.write(png->data(), png->size());
}

}   // namespace boralago
