# tt-fetch
neofetch but for Tenstorrent hardware

## Building
```sh
git submodule update --init --recursive

# modify C and C++ compiler to your likings
cmake -B build -G Ninja -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++

ninja -j$(nproc) -C build

# now you have the executable at build/tt-fetch
```
