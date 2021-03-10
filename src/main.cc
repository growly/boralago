#include <assert.h>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdlib.h>
#include <string>
#include <unordered_map>

#include <gflags/gflags.h>
#include <glog/logging.h>
#include <absl/strings/str_join.h>

#include <core/SkData.h>
#include <core/SkImage.h>
#include <core/SkStream.h>
#include <core/SkSurface.h>
#include <core/SkPath.h>
#include <core/SkCanvas.h>

#include "c_make_header.h"

#include "polygon.h"
#include "path.h"
#include "point.h"
#include "cell.h"

DEFINE_string(example_flag, "default", "for later");

void draw(SkCanvas* canvas) {
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


void raster(int width, int height,
            void (*draw)(SkCanvas*),
            const char* path) {
    sk_sp<SkSurface> rasterSurface =
            SkSurface::MakeRasterN32Premul(width, height);
    SkCanvas* rasterCanvas = rasterSurface->getCanvas();
    draw(rasterCanvas);
    sk_sp<SkImage> img(rasterSurface->makeImageSnapshot());
    if (!img) { return; }
    sk_sp<SkData> png(img->encodeToData());
    if (!png) { return; }
    SkFILEWStream out(path);
    (void)out.write(png->data(), png->size());
}

int main(int argc, char **argv) {
  google::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);

  LOG(INFO) << "Boralago " << boralago_VERSION_MAJOR << "." << boralago_VERSION_MINOR
            << std::endl;


  raster(1024, 1024, &draw, "test.png");

  return EXIT_SUCCESS;
}
