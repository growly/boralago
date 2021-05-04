#ifndef GEOMETRY_WRITER_H_
#define GEOMETRY_WRITER_H_

#include <string>

#include "geometry.pb.h"
#include "point.h"
#include "cell.h"
#include "physical_properties_database.h"


namespace boralago {

class Cell;

// TODO(aryap): Is a port an abstract shape or not?
class GeometryWriter {
 public:
  GeometryWriter(const PhysicalPropertiesDatabase &physical_db)
      : physical_db_(physical_db) {}
  bool WriteCell(const Cell &top, const std::string &filename);
  void WriteCellText(const Cell &top, const std::string &filename);
  void AddToGeometry(const Cell &top, vlsirlol::Geometry *geometry);

  void MapToExternalPoint(
      const Point &internal, vlsirlol::Point *external);
 private:
  void RectangleToProto(
      const Rectangle &rectangle, vlsirlol::Rectangle *out);

  void PolygonToProto(
      const Polygon &rectangle, vlsirlol::Polygon *out);

  const PhysicalPropertiesDatabase &physical_db_;
};

}  // namespace boralago

#endif  // GEOMETRY_WRITER_H_
