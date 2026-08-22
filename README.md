# TFKiss for Linux

This is a modified version of tfkiss-1.2.4 that will build and
run on 64-bit systems. The original tfkiss versions has some issues with
sizes of datatypes (like pointers) that are differend on 64-bit systems,
causing SEGVs at runtime.

Build with CMake:

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
sudo cmake --install build
```

The optional AXIP, XPID, FLEXNET and Linux high-baud features are described
in `INSTALL`.

The original README content follows below.

## Changes

### master

- ADD: run tfkiss in foreground (`-f`)

### v2.0.0

- ADD: Bluetooth support (`-bt <MAC ADDR>`)
- ADD: All parameters from the ini file can be used as commandline parameter. For more infos, use `--help`

### v1.3.0

- ADD: Show APRS UI Frames in Monitor Channel.

----------------------

This is the release of tfkiss-1.1.0, a port of TheFirmware by
by NORD><LINK written for TNC2 hardware. Refer to the copyrght.txt and
alas.* files in the doc directory for more detailed information on
copyright issues.

tfkiss-1.1.0 now supports NetBSD, Linux and Solaris. During the porting 
process, a few bugs were identified and fixed. A build configuration utility
was also added to automate the original configuration and building process.
Refer to the "CHANGES" file for more detailed information. 

The source code for tfkiss-1.1.0 can be found on 
ftp://ftp.ping.net.au/pub/unix/ham/tfkiss-1.1.0.tar.gz
ftp://ftp.wspse.de/packet_radio/misc/tfkiss-1.1.0.tar.gz
ftp://ftp.bfl.at/pub/src/tnt/tfkiss-1.1.0.tar.gz

Please send your bug-reports or comments to wulf@ping.net.au

Enjoy

73 -
cheerio Berndt
(VK5ABN)
