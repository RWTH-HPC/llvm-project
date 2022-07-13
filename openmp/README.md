# Target-dev specific build instruction

## Prerequisits

- gcc/9 or higher required as the nvcc host compiler
- clang/11
- cuda/11.4 or similar versions
- cmake>=3.13
- hwloc/2.5.0

## Build
Compile with

```bash
# create build folders
mkdir -p BUILD
mkdir -p INSTALL

# set tmp path to installation directory
INSTALL_DIR=$(pwd)/INSTALL

# create Makefiles
cd BUILD
cmake \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=${INSTALL_DIR} \
    -DLIBOMPTARGET_NVPTX_COMPUTE_CAPABILITIES="60,70" \
    -DLIBOMP_USE_NUMA_DEVICE_AFFINITY=1 \
    -DLIBOMP_HWLOC_INSTALL_DIR=<path/to/hwloc/install> ..

# build and install
make -j12
make install
```
