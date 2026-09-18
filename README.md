# natid-pizzo

Course project for **Artificial Intelligence**.

**Author:** Tarik Agić (20108)

Pizzo is a gamified route-planning application built with the natID framework. Its main mode solves the undirected Chinese Postman Problem for weighted neighborhood graphs and animates the resulting minimum-cost closed route. A Blackjack mini-game shares the same in-game wallet and includes an expected-value advisor for the Hit/Stand decision.

## Main features

- weighted undirected neighborhood graphs;
- Chinese Postman route optimization;
- Dijkstra shortest paths between odd-degree vertices;
- exact minimum-weight perfect matching for the supplied graph scale;
- Eulerization and Hierholzer-style Euler circuit construction;
- animated traversal with adjustable speed, rewards, and penalties;
- playable Blackjack with a shared wallet and betting controls;
- expected-value Hit/Stand advisor under an infinite-deck model;
- graphical interface, resources, animation, and sound through natID.

## Repository structure

```text
natid-pizzo/
├── Docs/
│   ├── Images/
│   ├── Pizzo_LatexSourceCode.tex
│   ├── Pizzo_Report.pdf
│   ├── Pizzo_Presentation.pdf
│   └── Pizzo_Presentation.pptx
└── Implementation/
    ├── CMakeLists.txt
    ├── Pizzo.cmake
    ├── README.md
    ├── res/
    └── src/
```

Build instructions, input format, and implementation details are provided in [Implementation/README.md](Implementation/README.md).
