#!/bin/bash

cp ../src/tfkiss tfkiss/usr/bin/tfkiss
cp -r /usr/local/share/tfkiss tfkiss/usr
appimagetool tfkiss
