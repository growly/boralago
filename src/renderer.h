#ifndef RENDERER_H_
#define RENDERER_H_

#include <string>

#include "cell.h"

class SkCanvas;

namespace boralago {

class Renderer {
 public:
  class Renderer(const Cell &cell)
      : cell_(cell) {}

  void WritePNG(const std::string &filename);

 private:
  void DrawCell(SkCanvas *canvas);

  const Cell &cell_;
};

}  // namespace boralago

#endif  // RENDERER_H_
