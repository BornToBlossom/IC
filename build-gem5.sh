#!/bin/bash
#
# Build script for legacy gem5 with Python 2.7
# Automatically detects the correct paths
#

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

# Check if environment is set up
if [ ! -d "gem5-venv" ] || [ ! -d ".pyenv-local" ]; then
    echo "ERROR: Build environment not found!"
    echo "Please run ./setup-build-env.sh first to set up the environment."
    exit 1
fi

# Activate the Python 2.7 virtual environment
source gem5-venv/bin/activate

# Set paths for Python 2.7
export PATH="$SCRIPT_DIR/.pyenv-local/versions/2.7.18/bin:$PATH"
export PYTHON_CONFIG="$SCRIPT_DIR/.pyenv-local/versions/2.7.18/bin/python2-config"

# Add Python headers and libraries to the compiler/linker search paths
export CPPFLAGS="-I$SCRIPT_DIR/.pyenv-local/versions/2.7.18/include/python2.7"
export LDFLAGS="-L$SCRIPT_DIR/.pyenv-local/versions/2.7.18/lib -Wl,-rpath,$SCRIPT_DIR/.pyenv-local/versions/2.7.18/lib"

echo "=== Build Environment ==="
echo "Working directory: $SCRIPT_DIR"
echo "Python: $(which python)"
echo "Python version: $(python --version 2>&1)"
echo "SCons: $(which scons)"
echo "SCons version: $(scons --version | head -1)"
echo "========================"
echo ""

# Run SCons with the specified target
scons build/ARM/gem5.opt -j24 "$@"

BUILD_EXIT_CODE=$?

if [ $BUILD_EXIT_CODE -eq 0 ]; then
    echo ""
    echo "=========================================="
    echo "BUILD SUCCESSFUL!"
    echo "=========================================="
    echo "Binary location: $SCRIPT_DIR/build/ARM/gem5.opt"
    echo "Binary size: $(ls -lh build/ARM/gem5.opt 2>/dev/null | awk '{print $5}')"
    echo ""
else
    echo ""
    echo "=========================================="
    echo "BUILD FAILED!"
    echo "=========================================="
    echo "Exit code: $BUILD_EXIT_CODE"
    echo ""
fi

exit $BUILD_EXIT_CODE
