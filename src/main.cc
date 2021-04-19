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
  // poly->set_overhang_start(100);
  // poly->set_overhang_end(100);
  poly->AddSegment({0, 100}, 25);
  poly->AddSegment({100, 100}, 25);
  poly->AddSegment({100, 200}, 50);
  poly->AddSegment({100, 300}, 35);

  boralago::PolyLine *some_region = inverter.AddPolyLine();
  some_region->set_layer(5);
  some_region->set_net("VDD");
  some_region->set_start({100, 100});
  some_region->AddSegment({100, 200});

  boralago::InflatorRules rules;
  boralago::PolyLineInflator inflator(rules);

  // Ownership: Something owns the cells, and cells don't own each other. A
  // cell library?
  boralago::Cell cell = inflator.Inflate(inverter);

  // Tile the cell.
  boralago::Cell top;
  top.AddInstance(boralago::Instance{&cell, boralago::Point(0, 0)});
  top.AddInstance(boralago::Instance{&cell, boralago::Point(100, 100)});

  // Create a routing grid.
  boralago::RoutingLayerInfo layer_1;
  layer_1.layer = 1;
  layer_1.area = boralago::Rectangle(boralago::Point(0, 0), 1000, 1000);
  layer_1.wire_width = 50;
  layer_1.offset = 50;
  layer_1.pitch = 100;
  layer_1.direction = boralago::RoutingTrackDirection::kTrackVertical;

  boralago::RoutingLayerInfo layer_2;
  layer_2.layer = 2;
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
  boralago::Port a(boralago::Point(75, 75), 50, 50, 1, "VDD");
  boralago::Port b(boralago::Point(465, 465), 50, 50, 2, "VDD");
  grid.AddRouteBetween(a, b);

  boralago::Renderer renderer(2048, 2048);
  renderer.RenderToPNG(inverter, top, grid, "test.png");

  return EXIT_SUCCESS;
}
