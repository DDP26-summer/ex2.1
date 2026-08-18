# K5x Sudoku Accelerator

A Sudoku solver built for the K5 platform, with both a software-only
implementation (`sud_basic`) and a hardware-accelerated version
(`sudx_basic`) backed by a SystemVerilog RTL accelerator.

## Repository layout

```
hw/
└── xlrs/
    └── sudx_basic/        RTL for the Sudoku hardware accelerator
sw/
└── apps/
    ├── sud_basic/         Software-only Sudoku solver
    ├── sud_shared/        Shared library, headers, and sample boards
    └── sudx_basic/        Software driver for the HW accelerator
```

## Getting started

Set up your workspace and pull in the base project:

```sh
tsmc65
git clone https://github.com/DDP26-summer/ex2.1
cp -r ex2.1/hw my_k5_proj
cp -r ex2.1/sw my_k5_proj
```

## Running the simulation

**Terminal 1 — start the simulator**

```sh
set_k5_terminal
launch_k5_sim sudx_basic
```

**Terminal 2 — launch the application**

```sh
set_k5_terminal
launch_k5_app sudx_basic -asl sud_shared
```

To run with the hardware accelerator enabled, add the `XON` flag:

```sh
launch_k5_app sudx_basic -asl sud_shared -ccd1 XON
```
