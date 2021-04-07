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

#include "renderer.h"
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

  boralago::Renderer renderer(1024, 1024);
  renderer.RenderToPNG(inverter, top, "test.png");

  return EXIT_SUCCESS;
}
