# How to Build

### On Linux
Install `crypto++` & `boost` libraries using your distribution package manager,
and then use the `make` file to compile the client:
```bash
make default
```


### On Windows using VisualStudio
1. Open VS and go to `File->New->Project From Existing Code...`
2. Set the `client` folder as the `Project file location`, and add all sub-folders (`crypto`, `protocol`, `session`).
3. Link `boost` & `cryptopp` libraries, and set `Runtime Library` to `Multi-threaded Debug (/MTd)`
4. In `Properties->C/C++->Language` set `C++ Language Standard` to `ISO C++20`
5. In `Properties->Linker->System` set `SubSystem` to `Console`
6. In `Properties->C/C++->Output Files` set `Object File Name` to `$(IntDir)/%(RelativeDir)`
7. Build solution.