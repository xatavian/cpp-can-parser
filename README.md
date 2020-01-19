C++ CAN Parser
==============
This project provides a C++ library that reads and parses [CAN](https://en.wikipedia.org/wiki/CAN_bus) databases which can then be analyzed for other purposes.

Currently the library only understand a subset of the DBC file format. 

Table of Contents
=================

- [C++ CAN Parser](#c-can-parser)
- [Table of Contents](#table-of-contents)
- [Compile and include the library](#compile-and-include-the-library)
  - [The public headers](#the-public-headers)
  - [Compilation](#compilation)
- [Parsing a CAN database](#parsing-a-can-database)
- [How to use the database](#how-to-use-the-database)
  - [`CANSignal`](#cansignal)
  - [`CANFrame`](#canframe)
  - [`CANDatabase`](#candatabase)
- [can-parse](#can-parse)
- [Supported standards](#supported-standards)
  
Compile and include the library
===============================

## The public headers

You need to put the public headers in an appropriate place:

```
your_project/
  |_ src/ 
  |_ include/
    |_ cpp-can-parser/ # <-- Put here the public headers
```

## Compilation

cpp-can-parser is not a header-only library. I recommand CMake to include the library into your project:

```cmake
# After including cpp-can-parser into your project's CMakeLists.txt...
target_link_libraries(MyAwesomeProject cpp-can-parser)
```

By default, `cpp-can-parser` is linked as a shared object. If you want to use a static library, you can add `set(CPP_CAN_PARSER_USE_STATIC TRUE)` or set it at "command-line time": `cmake -DCPP_CAN_PARSER_USE_STATIC=TRUE ...`


**If you are not using CMake to manage your whole project:**

cpp-can-parser is not provided with an already existing Makefile. You will still need CMake to manage the compilation of the library and then include it with traditional means:

```bash
> cd cpp-can-parser
> mkdir build && cd build
> cmake .. && cmake --build . --target cpp-can-parser
```

```Makefile
LD_FLAGS=-Lpath/to/the/library -lcpp-can-parser
```

Parsing a CAN database
======================

The main feature of the library is the possibility of parsing a file representing the CAN database. There are several popular file formats and the list of the currently ones is available at the end of this README. 

`CANDatabase::fromFile("path/to/the/data.dbc")` is the function to call in order to parse the database from a file. See the following example for a more "advanced" use:

```c++
#include <iostream>
#include "cpp-can-parser/CANDatabase.h"

int main(int argc, char** argv) {
  try {
    CANDatabase db = CANDatabase::fromFile(argv[1]);

    // ...
  }
  catch(const CANDatabaseException& e) {
    std::cerr << "Error: " << e.what();
    return 1;
  }

  return 0;
}
```

One can see that `CANDatabase::fromFile()` can throw a CANDatabaseException. This happens when the parsing goes wrong (basically when the given file does not exist or when there is a syntax error). `CANDatabaseException::what()` will give you details on the error.

If the data that you are using does not come from a file, it is also possible to use `CANDatabase::fromString("...")` which behaves just like its counterpart.


How to use the database
=======================

The library exposes three main kind of objects: 
* `CANDatabase` : represents the whole database. It gatherse all the informations of the database such as the frames and their signals, but you can also find the filename from which the database was created (if applicable).
* `CANFrame`: represents a single frame: we know its CAN ID, the DLC (Data length code), its period (in ms) and also its name (if any). Of course, a `CANFrame` also gathers the list of signals that can be read in a frame
* `CANSignal`: represents a single signal: a signal is a parameter of a frame characterized by different properties. It is mostly identificable through its start bit, length and [endianness](https://en.wikipedia.org/wiki/Endianness) (those three parameters are enough to extract the "raw data" that the signals represent) but one usually also specify a scale and offset factor, a signedness, and a name. More parameters are available and you are welcome to look into the `CANSignal.h` file to look at all the available properties.

All those classes try to behave the closest possible to STL containers. They notably implement all the required iterators methods so **they can be used in range-based for loops**

## `CANSignal`

Here are the most important properties of a `CANSignal` instance:

* `name()`: gives the name of the signal
* `length()` : gives the length of the signal
* `start_bit()` : gives the start bit of the signal
* `scale()` : gives the scale factor of the signal
* `offset()` : gives the offset factor of the signal
* `signedness()` : gives the signedness of the signal
* `endianness()` : gives the endianness of the signal
* `range()` : gives the range of the signal (which has a `min` and `max` property)
* `comment()` : gives the registered comment (if any)
 
Sometimes the database also includes "enumerations", ie for given signals we associate string literals to values (example: 0 = Nothing, 1 = State 1, 2 = State 2). `choices()` allows to iterate through the signal's enumeration (if any).
 
## `CANFrame`

Here are the most important properties of a `CANFrame` instance:

* `name()`: gives the name of the signal
* `can_id()`: gives the CAN ID of the signal
* `dlc()`: gives the DLC of the signal
* `period()`: gives the period of the signal
* `comment()` : gives the registered comment (if any)
* more properties to behave like a "standard container"

Use `begin()`/`end()` and!/or a ranged-based for loop to iterate through the signals of the frame.

```c++
const CANFrame& frame = ...;

// Print the name of all the frames by increasing order
for(const auto& sig : frame) {
  std::cout << "Signal: " << sig.second.name() << std::endl;
}
```

## `CANDatabase`

Here are the most important properties of a `CANDatabase` instance:

* `filename()` : gives the source file name (if any)
* `operator[std::string]` and `at(std::string)` : returns a reference to the `CANFrame` associated with the given frame name. The deviation from the STL behavior is that they both throw an `std::out_of_range` exception if the key does not exist (no `CANFrame` is created like it would with `std::map` for instance)
* `operator[unsigned long long]` and `at(unsigned long long)`: same but the key is the CAN ID of the `CANFrame`
* more properties to behave like a "standard container"

```c++
CANDatabase db = ...;

// Print the name of all the frames by increasing order
size_t i = 0;
for(const auto& frame : db) {
  std::cout << "Frame " << i++ << ": " << frame.second.name() << std::endl;
}
```

can-parse
=========

`can-parse` is a utility program that allows you to parse the content of a CAN database which is then output to the standard output. 

Different uses of `can-parse` are possible, mainly 4 operations are included in `can-parse`:
* Print a summary of the whole database
* Print a detailed view of a single entry of the database
  * CAN ID, DLC, Period, Comment
  * Signals' description
    * Name
    * Start bit
    * Length
    * Endianness
    * Signedness
* Check the integrity of the whole database (a summary is given) (very basic implementation for now)
* Check the integrity of a single frame (a detailed report is given) (very basic implementation for now)

The Command_Line Interface is very easy to use !

```bash
Usage: ./can-parse [ACTION [ARGUMENTS]] <path/to/file>
When no action is specified, defaults to printframe
Possible actions: 
        printframe [CAN ID]   Print the content of the CAN database
                              if CAN ID is specified, prints the details of the given frame
        checkframe [CAN ID]   Check different properties of the CAN database
                              if CAN ID is specified, print the check details of the given frame
        -h / --help           Print the present help message
```

**Compilation:**
To compile, just use the following instructions:

```bash
> cd cpp-can-parser
> mkdir build && cd build
> cmake .. && cmake --build . --target can-parse 
```

There you got yourself a very nice installation of `can-parse` :) !

Supported standards
===================

* DBC (the relevant subset of the file format)
