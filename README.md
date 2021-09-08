# Yagi

Yet Another Ghidra Integration for IDA

## Overview

`Yagi` intends to include the wonderful [Ghidra](https://github.com/NationalSecurityAgency/ghidra) decompiler part into both IDA pro and IDA Freeware.

![Exemple of Yagi](.img/yagi.gif)

You can download installers for Windows and Linux versions [here](https://github.com/airbus-cert/Yagi/releases), then press F7 and enjoy!

Here is the list of architectures that are currently supported:

|Arch Names|Yagi|
|----------|-----------|
|x86|✔️|
|x86_64|✔️|
|arm|✔️|
|aarch64(armv8)|✔️|
|powerpc|✔️|
|mips|✔️|
|sparc|✔️|
|arm|✔️|
|cp1600|❌|
|cr16|❌|
|avr8|❌|
|dalvik|❌|
|jvm|❌|
|tricore|❌|
|riscv|❌|
|z80|❌|
|System Z|❌|
|xCore|❌|

It's easy to add one if it's supported by Ghidra. Just open an issue, and we will do our best!

It allows you to edit the following items:
* Global Symbol like function prototype, global variable, etc.
* Local stack variables name and type
* Local regitry variables name and type

## Build

As `Yagi` is built using git `submodules` to handle Ghidra dependencies, you will first need to do a *recursive* clone:

```
git clone https://github.cert.corp/CERT/yagi --recursive
```

### For Windows

#### Install Dependencies

As Ghidra uses `bison` and `flex` to parse  the `sleigh` grammar, we need first to install build dependencies from [here](https://github.com/lexxmark/winflexbison/releases/)

You also need the `IDA` SDK associated with you version of IDA.

#### Cmake

`Yagi`'s build system is based on cmake; you can find an MSI package [here](https://github.com/Kitware/CMake/releases/).

You need at least a Visual Studio compiler with C++ toolchain.

#### Production

To generate a Wix installer, you need to install [WiX](https://github.com/wixtoolset/wix3/releases) before.

Then let the `cmake` magic happen:

```
git clone https://github.com/airbus-cert/Yagi --recursive
mkdir build_yagi
cd build_yagi
cmake ..\Yagi -DIDA_SDK_SOURCE_DIR=[PATH_TO_IDA_SDK_ROOT_FOLDER]
cmake --build . --target package --config release
```

Then a new `yagi-1.0.0-win64.msi` will be generated and will install all dependencies.

#### Development

To generate a dev environment you need to generate the Visual Studio solution:

```
git clone https://github.com/airbus-cert/Yagi --recursive
mkdir build_yagi
cd build_yagi
cmake ..\Yagi -DIDA_SDK_SOURCE_DIR=[PATH_TO_IDA_SDK_ROOT_FOLDER] -DBUILD_TESTS=ON
```

`PATH_TO_IDA_SDK_ROOT_FOLDER` represents the root path of the decompressed archive provided by Hex-Rays.

To launch unit tests, just use `ctest` installed with `cmake`:

```
cd tests
ctest -VV
```

### For Linux

#### Install Dependencies

As Ghidra uses `bison` and `flex` to parse  the `sleigh` grammar and `Yagi` is built using `cmake` and `c++`, you will need the following:

```
apt install cmake c++ git flex bison yacc
```

#### Production

To generate an installer script:

```
git clone https://github.com/airbus-cert/Yagi --recursive
mkdir build_yagi
cd build_yagi
cmake ..\Yagi -DIDA_SDK_SOURCE_DIR=[PATH_TO_IDA_SDK_ROOT_FOLDER]
cmake --build . --target package --config release
```

This will produce a `yagi-1.0.0-Linux.sh` script. Then you just have to launch it:

```
./yagi-1.0.0-Linux.sh --prefix=[PATH_TO_IDA_INSTALL_FOLDER]
y
n
```

Enjoy!

#### Development

To generate a dev environment you need to generate the Makefile:

```
git clone https://github.com/airbus-cert/Yagi --recursive
mkdir build_yagi
cd build_yagi
cmake ..\Yagi -DIDA_SDK_SOURCE_DIR=[PATH_TO_IDA_SDK_ROOT_FOLDER] -DBUILD_TESTS=ON -DCMAKE_BUILD_TYPE=Debug
make
```

To launch unit tests, just use `ctest` installed with `cmake`:

```
cd tests
ctest -VV
```

## TODO

* Handle enum types
* Add rules to handle CFG on windows
* Add rules to handle T9 for MIPS
* Add rules to handle end function computation on AARCH64

## Credits and references
