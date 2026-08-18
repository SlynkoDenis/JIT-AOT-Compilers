# Huawei 2022 JIT & AOT Compilers Course

### Dependencies

Install dependencies by running `sudo ./scripts/install_deps.sh`

### Build

The project uses C++26, Clang 22, LLD 22, and CMake 4.4.2. Configure and build it with the provided preset:

```sh
git submodule update --init --recursive
/home/huawei/cmake-4.4.2-linux-x86_64/bin/cmake --preset clang-22-debug
/home/huawei/cmake-4.4.2-linux-x86_64/bin/cmake --build --preset clang-22-debug
```

Run the tests with:

```sh
/home/huawei/cmake-4.4.2-linux-x86_64/bin/ctest --preset clang-22-debug
```

After build `build/bin` directory will contain two executables: one with tests and another with a program building an IR graph for factorial computation function.
