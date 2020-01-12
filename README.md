# C++ CAN Parser

This project provides a C++ library that reads and parses [CAN](https://en.wikipedia.org/wiki/CAN_bus) databases which can then be analyzed for other purposes.

Currently the library only understand a subset of the DBC file format. 

## Compiling the project

The project uses CMake to be compiled. It can also be used to easily include the library in your own project.

C++ CAN Parser currently is compiled as a static library.

## can-parse

`can-parse` is a utility program that allows you to parse the content of a CAN database which is then output to the standard output. 

Different uses of `can-parse` are possible, mainly 4 operations are included in `can-parse`:
* Print a summary of the whole database
* Print a detailed view of a single entry of the database (very basic implementation for now)
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
## Supported standards:
* DBC (in progress)
