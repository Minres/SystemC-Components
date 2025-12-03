# Running Tests

This repository has a GoogleTest-based suite (see `tests/`). The tests target the CMake option `BUILD_TESTING=ON` and require the same dependencies as the main build plus GoogleTest (fetched automatically via `FetchContent`).

## Prerequisites

- CMake ≥ 3.20
- A C++ compiler (GCC/Clang) with C++17 or later
- Ninja build system
- Python 3 with `conan` installed. A virtual environment is recommended:

```bash
python3 -m venv .venv
source .venv/bin/activate
pip install conan
```

If Conan cannot supply SystemC/CCI on your host, use the Docker flow below or point CMake at a local SystemC install via `SYSTEMC_HOME`.

## Configure and Build

```bash
cmake --preset Debug -B build -DBUILD_TESTING=ON -DCMAKE_CXX_STANDARD=20
cmake --build build -j
```

## Run Tests

```bash
cd build

# unit tests
ctest --output-on-failure

# transaction recording example binary
./examples/transaction_recording/transaction_recording
```

## Run Tests in Docker (Linux toolchain)

If your host setup cannot fetch SystemC via Conan, use the provided Dockerfile:

```bash
# from repo root
docker build -f Dockerfile.test -t scc-tests .
docker run --rm -v "$PWD":/work -w /work scc-tests /bin/bash -lc "\
  cmake --preset Debug -B build -DBUILD_TESTING=ON -DCMAKE_CXX_STANDARD=20 && \
  cmake --build build -j && \
  cd build && ctest --output-on-failure && \
  ./examples/transaction_recording/transaction_recording"
```

## CI

### Unit Tests
Tests run in `.github/workflows/ci-unit-tests.yml`, which configures with `-DBUILD_TESTING=ON`, builds, and runs `ctest` with C++20.

### C++ Standards Compliance
The repository is tested for compliance with multiple C++ standards in `.github/workflows/ci-compliance.yml`. This workflow:
- Tests against C++11, C++14, C++17, and C++20
- Builds the project with each standard
- Runs the `transaction_recording` example to verify basic functionality

To test a specific C++ standard locally:

```bash
cmake --preset Debug -B build -DCMAKE_CXX_STANDARD=<11|14|17|20>
cmake --build build -j
./build/examples/transaction_recording/transaction_recording
```
