## LifePath The Game
___
### Description
LifePath is an open-source card game. It uses the OpengGL [Tomos Engine](https://github.com/Life-Path-Game/TomosEngine).

More information will be added soon.

### Development
Create a file structure like this:
```
RootDirectory/
├── LifePath/ (This repository)
├── TomosEngine/ (Tomos Engine repository)
├── CMakeLists.txt (refer to the CMakeLists.txt below)
```

### CMakeLists.txt
```cmake
cmake_minimum_required(VERSION 3.28)
project(LifePath)

set(CMAKE_CXX_STANDARD 23)

add_subdirectory(Tomos)
add_subdirectory(LifePath)
```

### Building
## Building

in the root directory of the project, run the following commands:
```bash
mkdir build
cd build
```

##### Debug

```bash
cmake -DCMAKE_BUILD_TYPE=Debug ..
make
```

##### Release

```bash
cmake -DCMAKE_BUILD_TYPE=Release ..
make
```
