#include "geometry_writer.h"

#include <algorithm>
#include <string>
#include <fstream>
#include <google/protobuf/text_format.h>

#include "geometry.pb.h"
#include "cell.h"

namespace boralago {

bool GeometryWriter::WriteCell(const Cell &top, const std::string &filename) {
  vlsirlol::Geometry geo;
  AddToGeometry(top, &geo);
  std::fstream output(
      filename.c_str(), std::ios::out | std::ios::trunc | std::ios::binary);
  return geo.SerializeToOstream(&output);
}

void GeometryWriter::WriteCellText(const Cell &top, const std::string &filename) {
  vlsirlol::Geometry geo;
  AddToGeometry(top, &geo);
  std::string text_format;
  google::protobuf::TextFormat::PrintToString(geo, &text_format);
  std::fstream output(filename.c_str(), std::ios::out | std::ios::trunc);
  output << text_format;
  output.close();
}

void GeometryWriter::MapToExternalPoint(
    const Point &internal, vlsirlol::Point *external) {
  external->set_x(physical_db_.ToExternalUnits(internal.x()));
  external->set_y(physical_db_.ToExternalUnits(internal.y()));
}

void GeometryWriter::AddToGeometry(const Cell &top, vlsirlol::Geometry *geometry) {
  vlsirlol::Cell *cell_pb = geometry->add_cells();
  cell_pb->mutable_name()->set_domain("boralago");
  cell_pb->mutable_name()->set_name(top.name());

  std::set<Layer> layers;

  std::map<Layer, std::vector<const Rectangle*>> rectangles_by_layer;
  // This is neat, but it's longer than just writing the goddamn for loop:
  // std::for_each(
  //     top.rectangles().begin(), top.rectangles().end(),
  //     [&](auto &shape) {
  //         rectangles_by_layer_[shape.layer()].push_back(&shape);
  //     });
  for (const auto &shape : top.rectangles()) {
    rectangles_by_layer[shape.layer()].push_back(&shape);
    layers.insert(shape.layer());
  }

  std::map<Layer, std::vector<const Polygon*>> polygons_by_layer;
  for (const auto &shape : top.polygons()) {
    polygons_by_layer[shape.layer()].push_back(&shape);
    layers.insert(shape.layer());
  }

  std::map<Layer, std::vector<const Port*>> ports_by_layer;
  for (const auto &shape : top.ports()) {
    ports_by_layer[shape.layer()].push_back(&shape);
    layers.insert(shape.layer());
  }

  // Add the shapes in each layer.
  for (const Layer &layer : layers) {
    vlsirlol::LayeredShapes *layered_shapes = cell_pb->add_shapes();
    layered_shapes->mutable_layer()->set_number(layer);

    auto rect_it = rectangles_by_layer.find(layer);
    if (rect_it != rectangles_by_layer.end()) {
      for (const Rectangle *rectangle : rect_it->second) {
        vlsirlol::Rectangle *rectangle_pb = layered_shapes->add_rectangles();
        RectangleToProto(*rectangle, rectangle_pb);
      }
    }

    auto poly_it = polygons_by_layer.find(layer);
    if (poly_it != polygons_by_layer.end()) {
      for (const Polygon *polygon : poly_it->second) {
        vlsirlol::Polygon *polygon_pb = layered_shapes->add_polygons();
        PolygonToProto(*polygon, polygon_pb);
      }
    }
  }
  
  // Add child instances and the references to them.
  for (const Instance &instance : top.instances()) {
    Cell *cell = instance.template_cell();
    AddToGeometry(*cell, geometry);
    vlsirlol::Instance *instance_pb = cell_pb->add_instances();
    instance_pb->mutable_name()->set_domain("boralago");
    instance_pb->mutable_name()->set_name(cell->name());
    instance_pb->set_rotation_clockwise_degrees(0);
    instance_pb->mutable_lower_left()->set_x(0);
  }
}

void GeometryWriter::RectangleToProto(
    const Rectangle &rectangle, vlsirlol::Rectangle *out) {
  out->set_net(rectangle.net());
  MapToExternalPoint(rectangle.lower_left(), out->mutable_lower_left());
  int64_t width = physical_db_.ToExternalUnits(rectangle.Width());
  out->set_width(width);
  int64_t height = physical_db_.ToExternalUnits(rectangle.Height());
  out->set_height(height);
}

void GeometryWriter::PolygonToProto(
    const Polygon &polygon, vlsirlol::Polygon *out) {
  out->set_net(polygon.net());
  for (const Point &point : polygon.vertices()) {
    vlsirlol::Point *point_pb = out->add_vertices();
    MapToExternalPoint(point, point_pb);
  }
}

}   // namespace boralago
