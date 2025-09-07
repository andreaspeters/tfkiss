# TFKiss for Linux

This is a modified version of tfkiss-1.2.4 that will build and
run on 64-bit systems. The original tfkiss versions has some issues with
sizes of datatypes (like pointers) that are differend on 64-bit systems,
causing SEGVs at runtime.

For building information, please check the file README.hb9xar

The original README content follows below.

## Changes

### v1.3.0

- ADD: Show APRS UI Frames in Monitor Channel.

----------------------

This is the release of tfkiss-1.1.0, a port of TheFirmware by
by NORD><LINK written for TNC2 hardware. Refer to the copyrght.txt and
alas.* files in the doc directory for more detailed information on
copyright issues.

tfkiss-1.1.0 now supports NetBSD, Linux and Solaris. During the porting 
process, a few bugs were identified and fixed. In addition, the autoconf
utility was added automating the configuration and building process.
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
