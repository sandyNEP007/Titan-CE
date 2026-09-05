# Titan-CE ♟️

Titan-CE is a chess engine written entirely in modern C++ from scratch.

This project is being developed step by step to understand the internal architecture of professional chess engines instead of relying on existing engines or libraries.

---

# Current Version

**v0.6**

---

# Features

## Search Engine

* ✅ Negamax Search
* ✅ Alpha-Beta Pruning
* ✅ Root Move Analysis
* ✅ Best Move Selection

## Position Management

* ✅ Zobrist Hashing
* ✅ Transposition Table
* ✅ TT Store
* ✅ TT Hit

## Board Management

* ✅ 8×8 Board Representation
* ✅ Move Generation
* ✅ Make Move
* ✅ Undo Move
* ✅ Castling Support
* ✅ Pawn Promotion

## Evaluation

* ✅ Material Evaluation

---

# Development Roadmap

* [x] Board Representation
* [x] Legal Move Generation
* [x] Negamax Search
* [x] Alpha-Beta Pruning
* [x] Zobrist Hashing
* [x] Transposition Table
* [x] Piece-Square Tables
* [x] Move Ordering
* [x] Iterative Deepening
* [x] Quiescence Search
* [x] Killer Move Heuristic
* [x] History Heuristic
* [x] Null Move Pruning
* [x] UCI Protocol
* [x] Graphical User Interface
* [x] Futility prunning 
* [x] Implemented check extension
* [x] Null move reduction 
* [x] Late move reduction
* [ ] NNUE Evaluation [undergoing...]

---

# Project Structure

```
Titan-CE
│
├── include/
├── src/
├── README.md
├── .gitignore
└── author.txt
```

---

# Build

Compile using:

```bash
g++ src/main.cpp src/board.cpp src/move.cpp src/game.cpp -Iinclude -o Titan-CE.exe
```

Run:

```bash
./Titan-CE.exe
```

---

# Goal

The objective of Titan-CE is to build a complete chess engine from first principles while understanding every algorithm involved in modern engine development.

Every feature is implemented manually for educational purposes.
Later it will have GUI to play and learn chess with AI assistant with real time move analyser.
