# Building Windows Version
At first you must install [MSYS2](https://www.msys2.org) then open the `MSYS2 UCRT64` and execute the following commands:

##### Update all packages
```bash
pacman -Syu
```

##### Install dependencies
```bash
dependencies=(
  "git"
  "mingw-w64-ucrt-x86_64-boost"  # Optional
  "mingw-w64-ucrt-x86_64-cmake"
  "mingw-w64-ucrt-x86_64-cppwinrt"
  "mingw-w64-ucrt-x86_64-curl-winssl"
  "mingw-w64-ucrt-x86_64-doxygen"  # Optional, for docs... better to install official Doxygen
  "mingw-w64-ucrt-x86_64-graphviz"  # Optional, for docs
  "mingw-w64-ucrt-x86_64-MinHook"
  "mingw-w64-ucrt-x86_64-miniupnpc"
  "mingw-w64-ucrt-x86_64-nlohmann-json"
  "mingw-w64-ucrt-x86_64-nodejs"
  "mingw-w64-ucrt-x86_64-nsis"
  "mingw-w64-ucrt-x86_64-onevpl"
  "mingw-w64-ucrt-x86_64-openssl"
  "mingw-w64-ucrt-x86_64-opus"
  "mingw-w64-ucrt-x86_64-toolchain"
)
pacman -S "${dependencies[@]}"
```

And if you want to make sure the important libs are installed you can use the following command:

```bash
pacman -S \
  git \
  mingw-w64-ucrt-x86_64-cmake \
  mingw-w64-ucrt-x86_64-openssl \
  mingw-w64-ucrt-x86_64-toolchain
```

After installing all packages you need to clone the repository.
you can use following commands:

```bash
git clone https://github.com/MobinYengejehi/Apollo --recurse-submodules
cd sunshine
mkdir buildvs
mkdir build
```

And after that you must use cmake to create projects and solutions to be able to build those solutions. Do the following instructions to create projects:

# Visual Studio (NOT RECOMMENDED [Because the code is more suitable with MINGW compiler!])
Execute the following command:

```bash
cmake -B buildvs -G "Visual Studio 17 2022" -S .   -DOPENSSL_ROOT_DIR=/ucrt64   -DOPENSSL_INCLUDE_DIR=/ucrt64/include   -DOPENSSL_CRYPTO_LIBRARY=/ucrt64/lib/libcrypto.dll.a   -DOPENSSL_SSL_LIBRARY=/ucrt64/lib/libssl.dll.a
```

Now you can go to `buildvs` directory and open `Sunshine.sln` file which is the project file. If you open that file the `Visual Studio` will pop up and you can explore in codes and build any solution you want.

# Ninja (MINGW)
Execute the following commands:

```bash
cmake -B build -G Ninja -S .   -DOPENSSL_ROOT_DIR=/ucrt64   -DOPENSSL_INCLUDE_DIR=/ucrt64/include   -DOPENSSL_CRYPTO_LIBRARY=/ucrt64/lib/libcrypto.dll.a   -DOPENSSL_SSL_LIBRARY=/ucrt64/lib/libssl.dll.a
```

After that go to `build` directory using command:

```bash
cd build
```

And start building project:

```bash
ninja

#or if your are not in `build` directory and you are in Sunshine's directory:
ninja -C build
```

# Note
If you want to get the list of solutions or targets in `Ninja`, you must execute the following command in `build` directory:

```bash
ninja -t targets 
```

After you saw the list, you can choose which solution you want to build. for example if you want to build `sunshine` solution just execute the following command in `build` directory:

```bash
ninja sunshine
```

Also if you couldn't build project using `ninja -C build`, just use the cmake command and build the solutions for `Visual Studio` and then build each solution with ninja, using solution name that you extracted with `Visual Studio`. or also you can use `ninja -t targets` in `build` directory to see each solution's name.

It seems like the `test_sunshine` solution in `Apollo` has technical problems and cannot be built. Just try to pass the `test_sunshine` solution from building.

# Note 2
If you can't clone the project or you see some other network issues you can use [`Electro DNS`](https://electrotm.org/).

```bash
78.157.42.100
78.157.42.101
```