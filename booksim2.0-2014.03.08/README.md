# BookSim Network Simulator

BookSim is a cycle-accurate interconnection network simulator developed by the Stanford Concurrent VLSI Architecture Group. This version of BookSim has been extended to support reading trace files for packet injection.

## Table of Contents

- [Introduction](#introduction)
- [Features](#features)
- [Installation](#installation)
- [Usage](#usage)
  - [Configuration File](#configuration-file)
  - [Trace File Format](#trace-file-format)
- [Example](#example)
- [Contributing](#contributing)
- [License](#license)

## Introduction

BookSim is designed to simulate various network topologies and routing algorithms for interconnection networks. This extended version allows the simulator to read packet injection traces from a file, enabling more precise and reproducible simulation scenarios based on pre-recorded traffic patterns.

## Features

- Simulate a variety of network topologies (e.g., mesh, torus, butterfly).
- Support for multiple routing algorithms.
- Extended to read trace files for packet injection.

## Installation

To install and build BookSim, follow these steps:

1. Clone the repository:
   ```sh
   git clone https://github.com/sseyyeda/Booksim2_TraceFileInjection.git
   cd Booksim2_TraceFileInjection
   ```

2. Build the simulator:
   ```sh
   make
   ```

## Usage

### Configuration File

To use trace file injection, modify your configuration file as follows:

1. Set the `sim_type` to `file_inject`:
   ```sh
   sim_type = file_inject
   ```

### Trace File Format

The trace file should be a texr file with the following structure:

```
time,src,dst,byte
```

- `time`: The cycle at which the packet is injected.
- `src`: The source node of the packet.
- `dst`: The destination node of the packet.
- `byte`: The size of the packet in bytes.

Example trace file (`tracefile.txt`):

```
0,1,2,64
5,3,4,128
10,1,3,256
```

### Running the Simulator

When running the simulator, provide the path to your trace file as a command-line argument:

```sh
./booksim config.txt path/to/tracefile.csv
```

## Example

1. Create a configuration file (`config.txt`):
   ```sh
   topology = mesh
   k = 4
   n = 2
   routing_function = dim_order
   sim_type = file_inject
   ```

2. Create a trace file (`tracefile.csv`):
   ```sh
   0,1,2,64
   5,3,4,128
   10,1,3,256
   ```

3. Run the simulator with the configuration file and the trace file:
   ```sh
   ./booksim config.txt tracefile.csv
   ```

## Contributing

Contributions are welcome! Please fork the repository and submit pull requests.

