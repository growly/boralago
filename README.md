# *B*erkeley *O*pen *R*econfigurable *A*rchitecture *LA*yout *G*enerat*O*r

## Installation

### Prerequisites

1. [gperftools/tcmalloc](https://github.com/gperftools/gperftools)
  ```
  git clone git@github.com:gperftools/gperftools
  cd gperftools
  ./autogen.sh
  make -j $(nproc) && sudo make install
  ```
