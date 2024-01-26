# C++ Concurrent Programming

These files are exercises from `Concurrent Programming` course that I was doing at `Jamk University of Applied Sciences`.

## Compile and run a c++ file

```bash
F2C="airport" && g++ -Wall -Wextra -std=c++23 $(echo "$(pwd)/$F2C.cpp") -o $(echo $F2C) && ./$(echo $F2C)
```

## To use OpenCL on Linux

```bash
# Also install `clinfo` to check if OpenCL is installed correctly
sudo apt-get install -y clinfo

# Install OpenCL for Intel https://github.com/intel/compute-runtime/
mkdir neo && cd neo
wget https://github.com/intel/intel-graphics-compiler/releases/download/igc-1.0.14828.8/intel-igc-core_1.0.14828.8_amd64.deb
wget https://github.com/intel/intel-graphics-compiler/releases/download/igc-1.0.14828.8/intel-igc-opencl_1.0.14828.8_amd64.deb
wget https://github.com/intel/compute-runtime/releases/download/23.30.26918.9/intel-level-zero-gpu-dbgsym_1.3.26918.9_amd64.ddeb
wget https://github.com/intel/compute-runtime/releases/download/23.30.26918.9/intel-level-zero-gpu_1.3.26918.9_amd64.deb
wget https://github.com/intel/compute-runtime/releases/download/23.30.26918.9/intel-opencl-icd-dbgsym_23.30.26918.9_amd64.ddeb
wget https://github.com/intel/compute-runtime/releases/download/23.30.26918.9/intel-opencl-icd_23.30.26918.9_amd64.deb
wget https://github.com/intel/compute-runtime/releases/download/23.30.26918.9/libigdgmm12_22.3.0_amd64.deb
sudo dpkg -i *.deb

# Compile using `g++` with OpenCL 3.0
F2C="openclHello" && g++ -Wall -Wextra -std=c++23 -D CL_HPP_TARGET_OPENCL_VERSION=300 -D CL_TARGET_OPENCL_VERSION=300 $(echo "$(pwd)/$F2C.cpp") -o $(echo $F2C) -lOpenCL && ./$(echo $F2C)
```

C++ OpenCL 3.0 API Spec: https://registry.khronos.org/OpenCL/specs/3.0-unified/pdf/OpenCL_API.pdf

C++ OpenCL 3.0 Reference guide: https://www.khronos.org/files/opencl30-reference-guide.pdf

The OpenCL C Programming Language: https://registry.khronos.org/OpenCL/specs/3.0-unified/html/OpenCL_C.html
