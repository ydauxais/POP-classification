# Extracting Partially Ordered Patterns under Constraints for Classification

This repository contains the materials used for writting the paper titled "Extracting Partially Ordered Patterns under
Constraints for Classification" and submitted to ECML/PKDD19

## RPO Implementation

The implementation of our algorithm is available in the [RPO](/RPO) directory.

### Requirements

- Gecode 6 or later: [https://github.com/Gecode/gecode.git](https://github.com/Gecode/gecode.git).
- Boost::program_options (tested with Boost version 1.65.1 but may work with previous version)
- A compiler for C++14 (tested with gcc version 7.3.0)
- Cmake 3.5 or later (tested with CMake version 3.10.2)

- SWIG 3.0 for the Python interface

### Build

For compiling using Cmake (an a shell), use the following lines.

```
cd RPO
mkdir build
cd build
cmake .. -DGECODE_BIN=<Your Gecode repository>
make
```

The executable `bin/RPO` should be created in the `RPO` directory.

/!\ You will generate STRING errors if you don't use the argument ` -DGECODE_BIN=<Your Gecode repository>`.

#### SWIG

The same lines work for compiling the SWIG interface too.
```
cd RPO/SWIG
mkdir build
cd build
cmake .. -DGECODE_BIN=<Your Gecode repository>
make
```
A python module directory `rpo` should be created in the `RPO/SWIG` directory.

### Usage

You can obtain the usage using the argument `--help`:

```
$ ./bin/RPO --help
Description:
RPO Extract discriminant partially ordered patterns.

Usage:  RPO positive_file negative_file [options]
Positional Options (required):
  -p [ --positive_file ] arg input file containing the positive dataset to mine
                             (string)
                             positional : positive_file
  -n [ --negative_file ] arg input file containing the negative dataset to mine
                             (string)
                             positional : negative_file
  -f [ --fmin ] arg          minimal frequency threshold (number)
                             Number of sequences if >= 1 (support)
                             Percent of positive sequences number else
                             positional fmin
  -g [ --gmin ] arg          minimal growth rate threshold (number)
                             positional gmin


General Options:
  --help                       Display this help message
  -t [ --top_lattice ]         Browse mutliset space from the maximal one to
                               the void set
  -c [ --closed ]              Extract patterns only from closed multisets
  --min_length arg             Minimal number of items contained by an
                               extracted pattern
  --max_length arg             Maximal number of items contained by an
                               extracted pattern
  -n [ --numeric_intervals ]   Extract numeric intervals
  -l [ --output_tid_lists ]    Option to output the tid lists describing the
                               subgroups matched by the patterns
  -r [ --relevant ]            Extract only relevant episodes
```

### Reading the output

An example of outputted pattern is:
```
{A:1, B:1}
A:0 |-> B:0
Support: 3/1
T+: [0, 1, 2]
T-: [0]
```

Where `{A:1, B:1}` represents the pattern multiset.
In this case the multiset is composed by `1` event type `A` and `1` `B`.
The orders are following the multiset.
For example `A:0 |-> B:0` describes that the first (index `0`) instance of `A` occurs strictly before the first instance of `B`.

The order representation are:
- `A <-| B`: `A` occurs strictly after `B` (corresponding interval `[-1, -1]`)
- `A <- B`: `A` occurs after or at same time as `B` (corresponding interval `[-1, 0]`)
- `A -- B`: `A` occurs at same time as `B` (corresponding interval `[0, 0]`)
- `A -> B`: `A` occurs at same time or before `B` (corresponding interval `[0, 1]`)
- `A |-> B`: `A` occurs strictly before `B` (corresponding interval `[1, 1]`)

Then comes the support numbers `Support: 3/1` and, if the `-l` option is used the `SID` of each sequence supporting the pattern `T+: [0, 1, 2]` and `T-: [0]` for the positive and negative sequences respectively.

## Datasets

The two datasets [ASL-BU](/datasets/asl-bu) and [Blocks](/datasets/blocks) that are used for our experiments are available in the [datasets dir](/datasets).

- **ASL-BU**: The events are transcriptions from videos of American Sign Language expressions provided by Boston University.
	It consists of observation interval sequences with labels such as "head mvmt: nod rapid" or "shoulders forward" that belong to one of 7 classes like "yes–no question" or "rhetorical question".

- **Blocks**: The labels describe visual primitives obtained from videos of a human hand stacking colored blocks.
These labels describe which blocks touch and the actions of the hand ("contacts blue red," "attached hand red").
Each sequence represents one of 8 different scenarios from atomic actions (pick-up) to complete scenarios (assemble).

Each dataset is represented by a directory.
In these directories, each label is represented by a separated file.
Each line in these files represents a sequence of the form `<event_type_1> <timestamp_1> <event_type_2> <timestamp_2> ...`.
