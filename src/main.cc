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

#include "polygon.h"
#include "path.h"
#include "point.h"
#include "cell.h"

DEFINE_string(example_flag, "default", "for later");

int main(int argc, char **argv) {
  google::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);

  LOG(INFO) << "Boralago " << boralago_VERSION_MAJOR << "." << boralago_VERSION_MINOR
            << std::endl;


  return EXIT_SUCCESS;
}
