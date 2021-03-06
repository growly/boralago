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

const Point Renderer::kNoOffset = Point(0, 0);

SkPoint Renderer::MapToSkPoint(const Point &point) {
  return MapToSkPoint(point, Point(0, 0));
}

SkPoint Renderer::MapToSkPoint(const Point &point, const Point &offset) {
  // 1) Find relative position within the box defined by lower_left_ and
  //    upper_right_.
  // 2) Flip y coord for SkPoint.
  //
  // SkScalars measure pixels, I think.
  Point with_offset = point + offset - lower_left_;
  
  int64_t full_height = upper_right_.y() - lower_left_.y();
  int64_t full_width = upper_right_.x() - lower_left_.x();

  SkScalar y =
      static_cast<SkScalar>(with_offset.y()) * static_cast<SkScalar>(height_px_)
          / static_cast<SkScalar>(full_height);
  y = static_cast<SkScalar>(height_px_) - y;
  SkScalar x = 
      static_cast<SkScalar>(with_offset.x()) * static_cast<SkScalar>(width_px_)
          / static_cast<SkScalar>(full_width);
  SkPoint mapped_point = SkPoint::Make(x, y);
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
  for (const auto &poly_line : poly_line_cell.poly_lines()) {
    DrawPolyLine(*poly_line, kNoOffset, canvas);
  }

  LOG(INFO) << "there are this many vias: " << poly_line_cell.vias().size();
  for (const auto &via : poly_line_cell.vias()) {
    DrawVia(*via, kNoOffset, canvas);
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

void Renderer::DrawPolyLine(
    const PolyLine &poly_line, const Point &offset, SkCanvas *canvas) {
  const SkPaint &paint = GetLayerPaint(poly_line.layer()); 
  SkPath path;
  path.moveTo(MapToSkPoint(poly_line.start(), offset));
  for (const auto &segment : poly_line.segments()) {
    path.lineTo(MapToSkPoint(segment.end, offset));
  }
  canvas->drawPath(path, paint);
}

void Renderer::DrawPolygon(
    const Polygon &polygon, const Point &offset, SkCanvas *canvas) {
  const SkPaint &paint = GetLayerPaint(polygon.layer());
  SkPath path;
  path.moveTo(MapToSkPoint(polygon.vertices().front(), offset));
  for (size_t i = 1; i < polygon.vertices().size(); ++i) {
    path.lineTo(MapToSkPoint(polygon.vertices().at(i), offset));
  }
  path.close();
  canvas->drawPath(path, paint);
}

// This is actually the same as drawing a rectangle.
void Renderer::DrawPort(const Port &port, const Point &offset, SkCanvas *canvas) {
  SkPoint lower_left = MapToSkPoint(port.lower_left(), offset);
  SkPoint upper_right = MapToSkPoint(port.upper_right(), offset);
  SkRect rectangle = SkRect::MakeLTRB(
      lower_left.x(), upper_right.y(), upper_right.x(), lower_left.y());
  const SkPaint &paint = GetLayerPaint(port.layer());
  canvas->drawRect(rectangle, paint);
}

void Renderer::DrawRectangle(
    const Rectangle &rectangle, const Point &offset, SkCanvas *canvas) {
  SkPoint lower_left = MapToSkPoint(rectangle.lower_left(), offset);
  SkPoint upper_right = MapToSkPoint(rectangle.upper_right(), offset);
  SkRect sk_rect = SkRect::MakeLTRB(
      lower_left.x(), upper_right.y(), upper_right.x(), lower_left.y());
  const SkPaint &paint = GetLayerPaint(rectangle.layer());
  canvas->drawRect(sk_rect, paint);
}

void Renderer::DrawVia(
    const Via &via, const Point &offset, SkCanvas *canvas) {
  static const double kViaHeight = 15;
  static const double kViaWidth = 15;

  const SkPaint &bottom_paint = GetLayerPaint(via.bottom_layer());
  const SkPaint &top_paint = GetLayerPaint(via.top_layer());
  DrawPoint(via.centre() + offset,
            kViaHeight,
            kViaWidth,
            bottom_paint,
            top_paint,
            canvas);
}

void Renderer::DrawCell(
    const Cell &cell, const Point &offset, SkCanvas *canvas) {
  for (const auto &polygon : cell.polygons()) {
    DrawPolygon(polygon, offset, canvas);
  }

  for (const auto &rectangle : cell.rectangles()) {
    DrawRectangle(rectangle, offset, canvas);
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
    std::vector<std::unique_ptr<PolyLine>> poly_lines;
    std::vector<std::unique_ptr<Via>> vias;

    path->ToPolyLinesAndVias(grid.physical_db(),
                             &poly_lines,
                             &vias);
    for (const auto &poly_line : poly_lines) {
      DrawPolyLine(*poly_line, kNoOffset, canvas);
    }
    for (const auto &via : vias) {
      DrawVia(*via, kNoOffset, canvas);
    }
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
    const Cell &cell,
    const std::string &filename) {
  // This code from the Skia tutorial!
  sk_sp<SkSurface> raster_surface =
      SkSurface::MakeRasterN32Premul(width_px_, height_px_);
  SkCanvas* raster_canvas = raster_surface->getCanvas();

  raster_canvas->clear(SK_ColorWHITE);
  DrawCell(cell, kNoOffset, raster_canvas);

  sk_sp<SkImage> image(raster_surface->makeImageSnapshot());
  if (!image) { return; }
  sk_sp<SkData> png(image->encodeToData());
  if (!png) { return; }
  SkFILEWStream out(filename.c_str());
  out.write(png->data(), png->size());
}

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
  //raster_canvas->translate(200.0f, -200.0f);
  DrawPolyLineCell(poly_line_cell, raster_canvas);
  DrawCell(cell, kNoOffset, raster_canvas);
  DrawRoutingGrid(grid, raster_canvas);

  sk_sp<SkImage> image(raster_surface->makeImageSnapshot());
  if (!image) { return; }
  sk_sp<SkData> png(image->encodeToData());
  if (!png) { return; }
  SkFILEWStream out(filename.c_str());
  out.write(png->data(), png->size());
}

void Renderer::FitWidth(const Cell &cell) {
  // TODO(aryap): actually apply brain to this
  auto bb = cell.GetBoundingBox();
  lower_left_ = bb.first;
  int64_t dx = bb.second.x() - bb.first.x();
  LOG(INFO) << dx;
  double mx = static_cast<double>(width_px_) / static_cast<double>(dx);
  LOG(INFO) << mx;
  int64_t dy = bb.second.y() - bb.first.y();
  LOG(INFO) << dy;
  int64_t y = bb.first.y() + mx*dy;
  LOG(INFO) << y;
  upper_right_ = Point(bb.second.x(), y);
}

void Renderer::Fit(const Cell &cell) {
  auto bb = cell.GetBoundingBox();
  lower_left_ = bb.first;
  upper_right_ = bb.second;
}

}   // namespace boralago
