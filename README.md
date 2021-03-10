# *B*erkeley *O*pen *R*econfigurable *A*rchitecture *LA*yout *G*enerat*O*r

## Installation

### Prerequisites

[gperftools/tcmalloc](https://github.com/gperftools/gperftools)
  ```
  git clone git@github.com:gperftools/gperftools
  cd gperftools
  ./autogen.sh
  make -j $(nproc) && sudo make install
  ```

[gflags/gflags](https://github.com/gflags/gflags/blob/master/INSTALL.md)
  ```
  git clone git@github.com:gflags/gflags.git
  cd gflags
  mkdir build && cd build
  make -j $(nproc) && sudo make install
  ```

[glog/glog](https://github.com/google/glog)

  ```
  git clone https://github.com/google/glog.git
  cd glog
  cmake . -B build -H "Unix Makefiles"
  cmake --build build
  sudo cmake --build --target install
  ```

[abseil/abseil-cpp](https://abseil.io/docs/cpp/quickstart-cmake)

  ```
  git clone git@github.com:abseil/abseil-cpp.git
  cd abseil-cpp
  mkdir build && cd build
  cmake .. -DABSL_RUN_TESTS=ON -DABSL_USE_GOOGLETEST_HEAD=ON -DCMAKE_CXX_STANDARD=11
  cmake --build . --target all
  sudo make install
  ```

[skia](https://skia.org/user/build#quick)

(You need the [ninja build system](https://ninja-build.org/).)

  ```
  git submodule update --init
  cd skia
  python2 tools/git-sync-deps
  bin/gn gen out/Shared --args='is_official_build=true is_component_build=true'
  bin/gn gen out/Static --args='is_official_build=true
  tools/install_dependencies.sh
  ninja -C out/Shared
  ninja -C out/Static
  ```
