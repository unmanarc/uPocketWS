# Building uPocketWS from source

This document covers compiling `uPocketWS` yourself: prerequisites, the CMake
invocations, optional TLS support, per-distribution toolchain notes and how to
shrink the resulting binary.

If you only want a ready-made package, see [`INSTALL.md`](INSTALL.md) — COPR
covers Fedora, RHEL, CentOS, Rocky and AlmaLinux. Back to [`README.md`](README.md).

---

## Prerequisites

- CMake >= 3.12
- A C++17 compiler (GCC >= 7, Clang >= 6, MSVC 2019+)
- [libMantids30](https://github.com/unmanarc/libMantids30) >= 3.0.0 **with its
  `pkg-config` files**
- Boost (`regex`, `thread`, `json` — `json` is pulled in transitively by the
  libMantids30 HTTP headers)
- OpenSSL + zlib — only when building with TLS support

On Debian and Ubuntu the toolchain and libraries are installed with:

```bash
sudo apt update
sudo apt -y install build-essential cmake pkg-config git \
                    libboost-regex-dev libboost-thread-dev libboost-json-dev \
                    zlib1g-dev libssl-dev
```

`libMantids30` is not packaged in Debian/Ubuntu either — build and install it
from <https://github.com/unmanarc/libMantids30> (same `cmake`/`make install`
flow) before configuring `uPocketWS`, or point `-DMANTIDS30_PREFIX=` at the
prefix it was installed into.

> **Overall prerequisites**, identical for every platform:
> libMantids30 >= 3.0.0, cmake >= 3.12 and a C++17 compatible compiler
> (e.g. GCC >= 7).

---

## Build

```bash
git clone https://github.com/unmanarc/uPocketWS
cd uPocketWS

cmake -DCMAKE_BUILD_TYPE=MinSizeRel \
      -DCMAKE_INSTALL_PREFIX:PATH=/opt/osslibs \
      -DEXTRAPREFIX:PATH=/opt/osslibs \
      -B../uPocketWS-build .

cmake --build ../uPocketWS-build -j"$(nproc)"
cmake --install ../uPocketWS-build
```

---

## Building with TLS

```bash
cmake -DCMAKE_BUILD_TYPE=MinSizeRel -DWITH_SSL_SUPPORT=ON -B../uPocketWS-build .
```

Add `-DSSLRHEL7=ON` when linking against the `openssl11` package on RHEL 7.

If `libMantids30` is installed in a custom prefix, either export
`PKG_CONFIG_PATH=/opt/osslibs/share/pkgconfig:$PKG_CONFIG_PATH` or pass
`-DMANTIDS30_PREFIX=/opt/osslibs`.

---

## Distribution notes

| Target | Toolchain | Notes |
| --- | --- | --- |
| RHEL 10 / Rocky 10 / AlmaLinux 10 | gcc 14, cmake 3.31, OpenSSL 3, `dnf5` | Nothing special: `-DWITH_SSL_SUPPORT=ON` links `openssl-devel` 3.x. `libMantids30-devel` comes from the unmanarc COPR. |
| RHEL 9 / Rocky 9 / AlmaLinux 9 | gcc 11, cmake 3.20, OpenSSL 3, `dnf4` | Same as above; enable `crb` if a dependency is missing. |
| RHEL 8 | gcc 8, OpenSSL 1.1 | gcc 8 needs `-lstdc++fs`; `CMakeLists.txt` adds it automatically for GCC < 9. The base `cmake` package may be older than the required 3.12 — install `cmake3` from AppStream and build with `cmake3` instead. |
| RHEL 7 | gcc 8+ (devtoolset), `cmake3` and `openssl11` from EPEL | Pass `-DSSLRHEL7=ON` so `libssl11` is picked up instead of the system OpenSSL 1.0.2. |
| Ubuntu 26.04 LTS / Debian 13 | gcc 15 / 14, cmake 4.2, Boost 1.90, OpenSSL 3.5 | Verified on Ubuntu 26.04.1 LTS: `-DWITH_SSL_SUPPORT=ON` configures and builds with gcc 15.2 / CMake 4.2.3 / OpenSSL 3.5.5, zero warnings, no extra flags. |
| Ubuntu 20.04 – 24.04 LTS / Debian 11 – 12 | gcc 9 – 13, cmake 3.16 – 3.28, OpenSSL 1.1 – 3 | No extra flags: GCC >= 9 ships `std::filesystem` inside libstdc++ and needs no `-lstdc++fs`. |

Verified on Ubuntu 20.04 LTS through **26.04 LTS** and Debian 11 through 13.

---

## Shrinking the binary

```bash
strip -x -X -s /opt/osslibs/bin/uPocketWS
upx -9 --ultra-brute /opt/osslibs/bin/uPocketWS
```

Expected result: **~200 KB** without TLS support (still fits on a 5¼" floppy —
barely).

---

## Windows

The source also carries the Windows code paths (`_WIN32` guards in `src/main.cpp`
plus a `WIN32` link configuration in `CMakeLists.txt`), but Windows is not part of
the regular test matrix.

---

## Next steps

- [`INSTALL.md`](INSTALL.md) — command line reference, systemd service, security notes.
- [`README.md`](README.md) — feature overview and quick start.
