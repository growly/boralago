#include "renderer.h"

#include <core/SkData.h>
#include <core/SkImage.h>
#include <core/SkStream.h>
#include <core/SkSurface.h>
#include <core/SkPath.h>
#include <core/SkCanvas.h>

namespace boralago {

void Renderer::DrawDesign(SkCanvas* canvas) {
  const SkScalar scale = 256.0f;
  const SkScalar R = 0.45f * scale;
  const SkScalar TAU = 6.2831853f;
  SkPath path;
  path.moveTo(R, 0.0f);
  for (int i = 1; i < 7; ++i) {
    SkScalar theta = 3 * i * TAU / 7;
    path.lineTo(R * cos(theta), R * sin(theta));
  }
  path.close();
  SkPaint p;
  p.setAntiAlias(true);
  canvas->clear(SK_ColorWHITE);
  canvas->translate(0.5f * scale, 0.5f * scale);
  canvas->drawPath(path, p);
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

void Renderer::WritePNG(const std::string &filename) {
  // This code from the Skia tutorial!
  sk_sp<SkSurface> rasterSurface =
      SkSurface::MakeRasterN32Premul(width, height);
  SkCanvas* rasterCanvas = rasterSurface->getCanvas();
  draw(rasterCanvas);
  sk_sp<SkImage> img(rasterSurface->makeImageSnapshot());
  if (!img) { return; }
  sk_sp<SkData> png(img->encodeToData());
  if (!png) { return; }
  SkFILEWStream out(path);
  out.write(png->data(), png->size());
}

}   // namespace boralago
