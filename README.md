# tfkiss

`tfkiss` is a TNC/KISS emulator derived from TheFirmware. It provides a
serial or Bluetooth RFCOMM KISS transport, local console and socket access,
and optional AXIP routing. This version is maintained for current 64-bit
systems and is built **exclusively with CMake**.

## Requirements

- CMake 3.16 or newer
- A C compiler with C17 support
- BlueZ development headers on Linux when Bluetooth support is enabled
  (enabled by default on Linux)

## Build and test

Configure out of tree, build, and run the test suite:

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

If BlueZ development headers are unavailable, disable Bluetooth explicitly:

```sh
cmake -S . -B build -DTFKISS_ENABLE_BLUETOOTH=OFF
cmake --build build
```

There is no GNU Configure, Autoconf, Automake, or Makefile build path.

### CMake options

| Option | Default | Description |
| --- | --- | --- |
| `TFKISS_ENABLE_BLUETOOTH` | `ON` on Linux | Bluetooth RFCOMM support |
| `TFKISS_ENABLE_HIBAUD` | `OFF` | Linux baud rates above 38400 |
| `TFKISS_ENABLE_XPID` | `OFF` | Changeable PID support |
| `TFKISS_ENABLE_FLEXNET` | `OFF` | FLEXNET frame decoding |

Example with optional serial features:

```sh
cmake -S . -B build \
  -DTFKISS_ENABLE_HIBAUD=ON \
  -DTFKISS_ENABLE_XPID=ON
```

## Install

The default installation prefix is `/usr/local`:

```sh
sudo cmake --install build
```

Choose another prefix while configuring:

```sh
cmake -S . -B build -DCMAKE_INSTALL_PREFIX=/opt/tfkiss
cmake --build build
cmake --install build
```

A package build can be staged without root privileges:

```sh
DESTDIR="$PWD/stage" cmake --install build
```

The executable is installed below `sbin`, documentation below
`share/tfkiss`, and the example AXIP route configuration as
`share/tfkiss/conf/tfkiss.cfg`.

## Configuration and operation

All tfkiss settings are passed on the command line. `tfkiss.ini`, `.par`,
and `.pid` files are neither required nor created.

```sh
./build/src/tfkiss --help
./build/src/tfkiss --version
```

Common invocations:

```sh
# Local console with a serial KISS TNC
./build/src/tfkiss -t -d /dev/ttyUSB0 -b 9600

# Local console through Bluetooth RFCOMM
./build/src/tfkiss -t -bt 00:11:22:33:44:55

# Unix-domain socket service
./build/src/tfkiss -s /run/tfkiss.sock

# AXIP only, with an explicit route configuration
./build/src/tfkiss --kiss-active 0 --axip-active 1 -a /etc/tfkiss.cfg

# Diagnostic output; socket mode stays in the foreground
./build/src/tfkiss --debug -s /run/tfkiss.sock
```

`-t` selects the local console. In that mode, `Ctrl-C` performs an orderly
shutdown and `Ctrl-Z` uses normal shell job control. Console mode and socket
mode (`-s` or `--extsocket`) cannot be combined.

AXIP support is always compiled in and is enabled at runtime with
`--axip-active 1`. An AXIP route configuration supplied by `-a FILE` is then
required; `examples/tfkiss.cfg` is the supplied template.

`--pakratt232` enables AEA PK-232 host mode. It uses 9600 baud unless a
separate `-b BAUD` option is supplied.

For the complete option reference, including KISS framing, directories,
logging, and full-duplex DAMA configuration, use `tfkiss --help`.

## License and provenance

The project is based on the historic tfkiss/TheFirmware sources from
NORD><LINK. License and historical attribution texts are included in the
`doc/` directory.
