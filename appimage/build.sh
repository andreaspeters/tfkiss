#!/bin/bash
set -euo pipefail

SCRIPT_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
SOURCE_DIR=$(cd -- "${SCRIPT_DIR}/.." && pwd)
BUILD_DIR="${SOURCE_DIR}/build-appimage"
APPDIR="${SCRIPT_DIR}/tfkiss"

cmake -S "${SOURCE_DIR}" -B "${BUILD_DIR}" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=/usr \
    -DCMAKE_INSTALL_SBINDIR=bin
cmake --build "${BUILD_DIR}"
DESTDIR="${APPDIR}" cmake --install "${BUILD_DIR}"
appimagetool "${APPDIR}"
