# The Solidity Contract-Oriented Programming Language
You can talk to us on [![solidity at https://gitter.im/ethereum/solidity](https://badges.gitter.im/Join%20Chat.svg)](https://gitter.im/ethereum/solidity?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge). Questions, feedback and suggestions are welcome!

Solidity is a statically typed, contract-oriented, high-level language for implementing smart contracts on the Tron platform.

## Table of Contents

- [Background](#background)
- [Build and Install](#build-and-install)
- [Example](#example)
- [Documentation](#documentation)
- [Development](#development)
- [Maintainers](#maintainers)
- [License](#license)
- [Security](#security)

## Background

Solidity is a statically-typed curly-braces programming language designed for developing smart contracts
that run on the Tron Virtual Machine. Smart contracts are programs that are executed inside a peer-to-peer
network where nobody has special authority over the execution, and thus they allow to implement tokens of value,
ownership, voting and other kinds of logics.

When deploying contracts, you should use the latest released version of
Solidity. This is because breaking changes as well as new features and bug fixes are
introduced regularly. We currently use a 0.x version
number [to indicate this fast pace of change](https://semver.org/#spec-item-4).

## Build and Install

Instructions about how to build and install the Solidity compiler can be
found in the [Solidity documentation](https://solidity.readthedocs.io/en/latest/installing-solidity.html#building-from-source).


## Example
# Build and Install
Here is instructions about how to build and install the Solidity compiler.

## Prerequisites - macOS
For macOS, ensure that you have the latest version of Xcode installed. This contains the Clang C++ compiler, the Xcode IDE and other Apple development tools which are required for building C++ applications on OS X. If you are installing Xcode for the first time, or have just installed a new version then you will need to agree to the license before you can do command-line builds:
```
sudo xcodebuild -license accept
```

Our OS X builds require you to install the Homebrew package manager for installing external dependencies.

## Prerequisites - Windows

Here is the list of components that should be installed in Windows:
```
git
CMake
Visual Studio 2017 Build Tools or Visual Studio 2017

# visual studio 2017 Build Tools should include:
Visual Studio C++ core features
VC++ 2017 v141 toolset (x86,x64)
Windows Universal CRT SDK
Windows 8.1 SDK
C++/CLI support
```

## Clone the Repository
To clone the source code, execute the following command:
```
git clone --recursive https://github.com/tronprotocol/solidity.git
cd solidity
```
## build solc

### External Dependencies

We have a helper script which installs all required external dependencies on macOS, Windows and on numerous Linux distros.
```
./scripts/install_deps.sh
```
Or, on Windows (not official verified yet):
```
scripts\install_deps.bat
```
Or, on Centos (not official verified yet):
```
# install  z3
https://github.com/Z3Prover/z3
# install deps
scripts\install_deps.bat
```

Solidity project uses CMake to configure the build. You might want to install ccache to speed up repeated builds. CMake will pick it up automatically. Building Solidity is quite similar on Linux, macOS and other Unices:
```
mkdir build
cd build
cmake .. && make
```

## build solc-js (Build for javascript)
See the [Solidity documentation](https://solidity.readthedocs.io/en/latest/installing-solidity.html#building-from-source) for build instructions.
```
cd solidity

#Prepare to Build solcjson, assume your project dir is "/root/project/solidity"

A "Hello World" program in Solidity is of even less use than in other languages, but still:
# External Dependencies

on Ubuntu
```
docker run -it -v $(pwd):/root/project -w /root/project trzeci/emscripten:sdk-tag-1.35.4-64bit

```
apt-get update
apt-get install wget gcc libboost-all-dev

# install essetial
apt-get install -y build-essential

# if "install build-essential" failed, please install Emscripten from this url:
https://github.com/juj/emsdk/tree/llvm_root

# check if boost is installed
./scripts/travis-emscripten/install_deps.sh

# begin to compile
./scripts/travis-emscripten/build_emscripten.sh

# release soljson.js
bash tran.sh

```solidity
pragma solidity ^0.6.0;
```

### Windows
```
https://solidity.readthedocs.io/en/latest/installing-solidity.html#building-from-source
# install vs2017
# install cmake
scripts\install_deps.bat
部分文件在中文windows下警告被看作错误，改一下文件编码，删除不用的测试就行了
mkdir build
cd build
cmake -G "Visual Studio 15 2017 Win64" ..
cmake --build . --config RelWithDebInfo
```
```
pragma solidity ^0.5.0;

contract HelloWorld {
  function helloWorld() external pure returns (string memory) {
    return "Hello, World!";
  }
}
```

To get started with Solidity, you can use [Remix](https://remix.ethereum.org/), which is an
browser-based IDE. Here are some example contracts:

1. [Voting](https://solidity.readthedocs.io/en/latest/solidity-by-example.html#voting)
2. [Blind Auction](https://solidity.readthedocs.io/en/latest/solidity-by-example.html#blind-auction)
3. [Safe remote purchase](https://solidity.readthedocs.io/en/latest/solidity-by-example.html#safe-remote-purchase)
4. [Micropayment Channel](https://solidity.readthedocs.io/en/latest/solidity-by-example.html#micropayment-channel)

## Documentation

The Solidity documentation is hosted at [Read the docs](https://solidity.readthedocs.io).

## Development

Solidity is still under development. Contributions are always welcome!
Please follow the
[Developers Guide](https://solidity.readthedocs.io/en/latest/contributing.html)
if you want to help.

You can find our current feature and bug priorities for forthcoming
releases [in the projects section](https://github.com/ethereum/solidity/projects).

## Maintainers
* [@axic](https://github.com/axic)
* [@chriseth](https://github.com/chriseth)

## License
Solidity is licensed under [GNU General Public License v3.0](LICENSE.txt).

Some third-party code has its [own licensing terms](cmake/templates/license.h.in).

## Security

The security policy may be [found here](SECURITY.md).
