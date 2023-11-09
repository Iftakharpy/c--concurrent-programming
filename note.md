# C++ Notes

## Compile and run a c++ file

```bash
F2C="bankAccount" && g++ -Wall -std=c++23 $(echo "$(pwd)/$F2C.cpp") -o $(echo $F2C) && ./$(echo $F2C)
```