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

#include "c_make_header.h"

#include "routing_grid.h"
#include "renderer.h"
#include "rectangle.h"
#include "instance.h"
#include "poly_line_cell.h"
#include "poly_line_inflator.h"
#include "inflator_rules.pb.h"

DEFINE_string(example_flag, "default", "for later");

int main(int argc, char **argv) {
  google::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);

  LOG(INFO) << "Boralago " << boralago_VERSION_MAJOR << "." << boralago_VERSION_MINOR
            << std::endl;

  boralago::PolyLineCell inverter;
  boralago::PolyLine *poly = inverter.AddPolyLine();
  poly->set_layer(3);
  poly->set_net("I");
  poly->set_start({0, 0});
  poly->set_overhang_start(100);
  poly->set_overhang_end(100);
  poly->AddSegment({0, 100}, 25);
  poly->AddSegment({100, 100}, 25);
  poly->AddSegment({100, 200}, 50);
  poly->AddSegment({100, 300}, 35);

  boralago::PolyLine *some_region = inverter.AddPolyLine();
  some_region->set_layer(5);
  some_region->set_net("VDD");
  some_region->set_start({100, 100});
  some_region->AddSegment({100, 200});

  boralago::PolyLine *other_region = inverter.AddPolyLine();
  other_region->set_layer(4);
  other_region->set_start({0, 150});
  other_region->AddSegment({0, 250}, 10);

  boralago::InflatorRules rules;
  boralago::PolyLineInflator inflator(rules);

  // Ownership: Something owns the cells, and cells don't own each other. A
  // cell library?
  boralago::Cell cell = inflator.Inflate(inverter);

  // Tile the cell.
  boralago::Cell top;

  // auto box = cell.GetBoundingBox();
  // int64_t dx = box.second.x() - box.first.x();
  // int64_t buffer_x = 10;
  // int64_t dy = box.second.y() - box.first.y();
  // int64_t buffer_y = 10;
  // for (int i = 0; i < 9; ++i) {
  //   for (int j = 0; j < 6; ++j) {
  //     top.AddInstance(boralago::Instance{
  //         &cell, boralago::Point(i*(dx + buffer_x), j*(dy + buffer_y))});
  //   }
  // }

  // Create a routing grid.
  boralago::RoutingLayerInfo layer_1;
  layer_1.layer = 4;
  layer_1.area = boralago::Rectangle(boralago::Point(0, 0), 1000, 1000);
  layer_1.wire_width = 50;
  layer_1.offset = 50;
  layer_1.pitch = 100;
  layer_1.direction = boralago::RoutingTrackDirection::kTrackVertical;

  boralago::RoutingLayerInfo layer_2;
  layer_2.layer = 5;
  layer_2.area = boralago::Rectangle(boralago::Point(0, 0), 1000, 1000);
  layer_2.wire_width = 50;
  layer_2.offset = 50;
  layer_2.pitch = 100;
  layer_2.direction = boralago::RoutingTrackDirection::kTrackHorizontal;

  boralago::RoutingGrid grid;
  grid.DescribeLayer(layer_1);
  grid.DescribeLayer(layer_2);

  boralago::LayerConnectionInfo layer_1_2;
  layer_1_2.cost = 1.0;
  grid.ConnectLayers(layer_1.layer, layer_2.layer, layer_1_2);


  // Make up ports for test.
  boralago::Port a(boralago::Point(150, 150), 50, 50, 4, "VDD");
  boralago::Port b(boralago::Point(465, 465), 50, 50, 5, "VDD");
  grid.AddRouteBetween(a, b);
  grid.AddRouteBetween(a, b);
  grid.AddRouteBetween(a, b);

  std::unique_ptr<boralago::PolyLineCell> grid_lines(
      grid.CreatePolyLineCell());
  boralago::Cell grid_cell = inflator.Inflate(*grid_lines);
  top.AddInstance(boralago::Instance{&grid_cell, boralago::Point(0, 0)});

  boralago::Renderer renderer(2048, 2048);
  renderer.RenderToPNG(top, "top.png");
  renderer.RenderToPNG(inverter, top, grid, "mess.png");

  LOG(INFO) << "done";

  return EXIT_SUCCESS;
}
