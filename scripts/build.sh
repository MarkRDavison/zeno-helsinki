#!/usr/bin/env bash
# Configure, build, and test the Helsinki project (Linux/Fedora).
#
# Uses Ninja and the default system toolchain (GCC/Clang).
# On first run or after --rebuild, runs CMake configure. By default performs
# an incremental build and runs CTest. Success prints "Build passed" and
# "Test passed". Command output is shown only when a step fails.
#
# Usage:
#   ./scripts/build.sh
#   ./scripts/build.sh --rebuild
#   ./scripts/build.sh --clean-only
#   ./scripts/build.sh --no-test

set -euo pipefail

BUILD_TYPE=Debug
REBUILD=false
CLEAN=false
CLEAN_ONLY=false
CONFIGURE=false
NO_TEST=false

usage() {
    sed -n '2,12p' "$0" | sed 's/^# //'
    echo
    echo "Options:"
    echo "  --build-type Debug|Release   CMake build type (default: Debug)"
    echo "  --rebuild                    Remove build dir, reconfigure, build, test"
    echo "  --clean                      Remove build directory before configure"
    echo "  --clean-only                 Remove build directory and exit"
    echo "  --configure                  Force CMake configure"
    echo "  --no-test                    Skip CTest after build"
    echo "  -h, --help                   Show this help"
}

while [[ $# -gt 0 ]]; do
    case "$1" in
        --build-type)
            BUILD_TYPE="$2"
            shift 2
            ;;
        --rebuild) REBUILD=true; shift ;;
        --clean) CLEAN=true; shift ;;
        --clean-only) CLEAN_ONLY=true; shift ;;
        --configure) CONFIGURE=true; shift ;;
        --no-test) NO_TEST=true; shift ;;
        -h|--help) usage; exit 0 ;;
        *)
            echo "Unknown option: $1" >&2
            usage >&2
            exit 2
            ;;
    esac
done

if [[ "$BUILD_TYPE" != Debug && "$BUILD_TYPE" != Release ]]; then
    echo "Invalid --build-type: $BUILD_TYPE (use Debug or Release)" >&2
    exit 2
fi

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="$REPO_ROOT/build"

remove_build_dir() {
    if [[ -d "$BUILD_DIR" ]]; then
        rm -rf "$BUILD_DIR"
    fi
}

run_step() {
    local label="$1"
    shift
    local log
    log="$(mktemp)"
    if (cd "$REPO_ROOT" && "$@" >"$log" 2>&1); then
        rm -f "$log"
        return 0
    fi
    cat "$log"
    rm -f "$log"
    echo "$label" >&2
    return 1
}

if [[ "$CLEAN_ONLY" == true ]]; then
    remove_build_dir
    exit 0
fi

if [[ "$REBUILD" == true ]]; then
    CLEAN=true
fi

if [[ "$CLEAN" == true ]]; then
    remove_build_dir
fi

needs_configure=false
if [[ "$CONFIGURE" == true || "$REBUILD" == true || ! -f "$BUILD_DIR/build.ninja" ]]; then
    needs_configure=true
fi

if [[ "$needs_configure" == true ]]; then
    run_step "Configure failed" \
        cmake -B build -G Ninja -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
        || exit $?
fi

run_step "Build failed" cmake --build build || exit $?
echo "Build passed"

if [[ "$NO_TEST" != true ]]; then
    run_step "Test failed" ctest --output-on-failure --test-dir build || exit $?
    echo "Test passed"
fi
