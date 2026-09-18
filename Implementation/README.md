# Pizzo implementation

Pizzo combines deterministic route optimization with stochastic decision support in one natID desktop application. The main window loads one of the bundled weighted neighborhood graphs, solves the undirected Chinese Postman Problem, and animates the optimal closed traversal. The Blackjack window uses the same wallet and provides an expected-value comparison of Hit and Stand.

## Route-optimization pipeline

For a connected undirected weighted graph, the solver performs the following steps:

1. compute the total cost of all original edges;
2. identify all vertices of odd degree;
3. run Dijkstra's algorithm from each odd vertex;
4. find an exact minimum-weight perfect matching between the odd vertices;
5. duplicate the matched shortest paths to make every degree even;
6. construct an Euler circuit with a Hierholzer-style traversal;
7. convert the vertex circuit into route steps for visualization.

If the original graph is already Eulerian, no edges are duplicated. The resulting closed walk traverses every original edge at least once while minimizing the total added cost.

The matching routine is exhaustive and exact. It is appropriate for the small odd-vertex sets used by the bundled neighborhoods, but it is not intended for graphs with a large number of odd vertices.

## Blackjack advisor

The Blackjack module supports Hit and Stand, while the dealer draws until reaching at least 17. The advisor uses an infinite-deck model in which each rank has probability `1/13`. It recursively evaluates dealer outcomes and future player decisions, memoizes repeated states, and compares the expected values of Hit and Stand.

## Neighborhood file format

Neighborhoods are stored as plain-text graph descriptions:

```text
# Optional comment
V 4
LABELS A B C D
E 0 1 3
E 1 2 5
E 2 3 2
E 3 0 4
```

- `V <count>` declares the number of vertices.
- `LABELS` optionally assigns one label to each vertex.
- `E <u> <v> <cost>` adds an undirected edge with a positive integer cost.
- Empty lines and lines beginning with `#` are ignored.

The graph must be connected for the Chinese Postman solver to produce a route covering the entire neighborhood.

## Prerequisites

- Windows 10 or Windows 11;
- Visual Studio 2022 with **Desktop development with C++**;
- CMake 3.18 or newer;
- natID SDK 4.2.1;
- 64-bit build configuration.

By default, the SDK is expected at:

```text
%USERPROFILE%\natID.SDK
```

A different SDK location can be supplied with the CMake variable `NATID_SDK_ROOT`.

## Command-line build

Place the repository at:

```text
%USERPROFILE%\natid-pizzo
```

Configure a Visual Studio 2022 x64 build:

```bat
cmake -S "%USERPROFILE%\natid-pizzo\Implementation" -B "%USERPROFILE%\natID.RAMDisk\build\Pizzo" -G "Visual Studio 17 2022" -A x64
```

Build the Debug configuration:

```bat
cmake --build "%USERPROFILE%\natID.RAMDisk\build\Pizzo" --config Debug
```

## Running the application

Make the natID runtime libraries available in the current CMD session:

```bat
set "PATH=%USERPROFILE%\natID.SDK\bin;%USERPROFILE%\natID.SDK\bin\GTK;%PATH%"
```

Run Pizzo with its development resources:

```bat
"%USERPROFILE%\natID.RAMDisk\Out\Pizzo\Debug\Pizzo.exe" "-devResPath=%USERPROFILE%\natid-pizzo\Implementation"
```

To launch it without keeping the CMD window occupied:

```bat
start "" "%USERPROFILE%\natID.RAMDisk\Out\Pizzo\Debug\Pizzo.exe" "-devResPath=%USERPROFILE%\natid-pizzo\Implementation"
```

## Building with CMake GUI

1. Set **Where is the source code** to `C:/Users/<username>/natid-pizzo/Implementation`.
2. Set **Where to build the binaries** to `C:/Users/<username>/natID.RAMDisk/build/Pizzo`.
3. Select **Configure**.
4. Choose `Visual Studio 17 2022` and the `x64` platform.
5. Select **Generate**, followed by **Open Project**.
6. Build the `Pizzo` target in the Debug configuration.

## Source structure

```text
Implementation/
├── CMakeLists.txt
├── Pizzo.cmake
├── res/
│   ├── DevRes.xml
│   ├── main.xml
│   ├── appIcon/
│   ├── assets/
│   ├── cards/
│   ├── neighborhoods/
│   └── tr/
└── src/
    ├── core/
    ├── blackjack/
    ├── ui/
    ├── Application.h
    ├── MainWindow.h
    ├── BlackjackWindow.h
    ├── GameState.h
    └── main.cpp
```

The `core` directory contains graph loading and Chinese Postman algorithms. The `blackjack` directory contains the game state and expected-value advisor. The `ui` directory contains the natID canvases used by the two application windows.
