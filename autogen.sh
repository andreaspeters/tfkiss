#!/bin/bash
#
# cleanup
rm -rf autom4te.cache
rm -f config.guess
rm -f config.sub
rm -f configure
rm -f Makefile
rm -f doc/Makefile
rm -f examples/Makefile
rm -f src/Makefile

# run autoconfig
automake --add-missing --copy
autoconf

echo "*** ingore the automake error about no Makefile.am found"
