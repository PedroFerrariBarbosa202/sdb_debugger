# SDB: Minimal x64 Debugger

<p align="center">
Project based on the book *Write a Native X64 Debugger from Scratch*, by Sy Brand.
</p>

---
## Releases
All releases are compiled for linux. If you don't want to build the project, please consider reading the execution instructions in one of the available releases.

## Dependencies
- vcpkg

| Library      | Description| Install command (linux) |
|--------------|------------|-------------------------|
| libedit      | cli editing and recording | sudo apt install libedit-dev |
| catch2       | unit testing    | sudo apt install catch2 |
| fmt          | modern string formatting  | sudo apt install libfmt-dev |

## Building
```
git clone https://github.com/PedroFerrariBarbosa202/sdb_debugger
cd sdb_debugger/build
cmake --build .

OBS: a pre-compiled build for linux is available under sdb_debugger/build/application
```

## Running
```
# TARGET_DIRECTORY -> absolute or based on current directory

cd sdb_debugger/build/application
./sdb TARGET_DIRECTORY
```
## Commands
```
# This prints all the available commands in the debugger

sdb> help
```
