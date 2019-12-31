# C++ CAN Parser

This project provides a C++ library that reads and parses [CAN](https://en.wikipedia.org/wiki/CAN_bus) databases which can then be analyzed for other purposes.

Currently the library only understand a subset of the DBC file format. 

## Compiling the project

The project uses CMake to be compiled. It can also be used to easily include the library in your own project.

C++ CAN Parser currently is compiled as a static library.

## can-parse

`can-parse` is a utility program that allows you to parse the content of a CAN database which is then output to the standard output. You can use it to see if the file content is valid or explore the content of the database.

## Supported standards:
* DBC (in progress)
