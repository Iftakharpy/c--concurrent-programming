# C++ Notes

## Compile and run a c++ file

```bash
F2C="airport" && g++ -Wall -std=c++23 $(echo "$(pwd)/$F2C.cpp") -o $(echo $F2C) && ./$(echo $F2C)
```

## To use OpenCL on Linux

```bash
# Install OpenCL
sudo apt install opencl-headers ocl-icd-opencl-dev -y
# Compile using `g++` with OpenCL 3.0
F2C="imageProcessing" && g++ -Wall -std=c++23 -lOpenCL -D CL_TARGET_OPENCL_VERSION=300 $(echo "$(pwd)/$F2C.cpp") -o $(echo $F2C) && ./$(echo $F2C)
```
