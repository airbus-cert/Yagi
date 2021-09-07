# yagi
Yet Another Ghidra Integration for IDA

## Overview

`yagi` intend to include the wonderful [Ghidra](https://github.com/NationalSecurityAgency/ghidra) decompiler part to both IDA pro and IDA Freeware.

You can download installer for Windows and Linux version [here](https://github.com/airbus-cert/Yagi/releases), press F7 and enjoy!

The list of architectures that are currently supported :

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

It's really easy to add one if it's supported by Ghidra, so just open an issue, and we will try to do our best!

It allows you to rename retype any global or local symbol. Changes are persistent.

## Build

As `Yagi` is built using git `submodules` to handle `ghidra` dependencies:
```
git clone https://github.cert.corp/CERT/yagi --recursive
```

### For Windows

#### Install Dependencies

As ghidra use `bison` and `flex` to parse  the `sleigh` grammar, we need first to install build dependencies from [here](https://github.com/lexxmark/winflexbison/releases/tag/v2.5.24)

You also need the `IDA` SDK associated with you version of IDA.

#### Cmake

`yagi` build system is based on cmake, you need to install from [here](https://github.com/Kitware/CMake/releases/download/v3.20.4/cmake-3.20.4-windows-x86_64.msi).

You need at least a visual studio compiler with c++ tollchain.

#### Production

To generate a Wix installer, you need to install [WiX](http://wixtoolset.org/releases/v3.11.1/stable) before
Then `cmake` magic happen:

```
git clone https://github.com/airbus-cert/Yagi --recursive
mkdir build_yagi
cd build_yagi
cmake ..\Yagi -DIDA_SDK_SOURCE_DIR=[PATH_TO_IDA_SDK_ROOT_FOLDER]
cmake --build . --target package --config release
```

Then a new `yagi-1.0.0-win64.msi` will be generated and will install all dependencies.

#### Development

To generate a dev environment you need to generate the visual studio solution :

```
git clone https://github.com/airbus-cert/Yagi --recursive
mkdir build_yagi
cd build_yagi
cmake ..\Yagi -DIDA_SDK_SOURCE_DIR=[PATH_TO_IDA_SDK_ROOT_FOLDER] -DBUILD_TESTS=ON
```

`PATH_TO_IDA_SDK_ROOT_FOLDER` represent the root path of the decompressed archive provided by Hex-Ray. 

To launch unit tests, just use `ctest` installed with `cmake`:

```
cd tests
ctest -VV
```

### For Linux

#### Install Dependencies

As ghidra use `bison` and `flex` to parse  the `sleigh` grammar, and `yagi` is built using `cmake` and `c++` :

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

This will produce `yagi-1.0.0-Linux.sh` script:

Then you've just have to launch it :

```
./yagi-1.0.0-Linux.sh --prefix=[PATH_TO_IDA_INSTALL_FOLDER]
y
n
```

Enjoy!

#### Development

To generate a dev environment you need to generate the Makefile :

```
git clone https://github.com/airbus-cert/Yagi --recursive
mkdir build_yagi
cd build_yagi
cmake ..\Yagi -DIDA_SDK_SOURCE_DIR=[PATH_TO_IDA_SDK_ROOT_FOLDER] -DBUILD_TESTS=ON -DCMAKE_BUILD_TYPE=Debug
make
```

To launch unit tests, just use `ctest`, installed with `cmake`:

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

