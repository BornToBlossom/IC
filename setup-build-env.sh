#!/bin/bash
#
# Automatic Build Environment Setup for Legacy gem5
# This script sets up Python 2.7 and SCons in a local sandbox
#

set -e  # Exit on error

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

echo "=========================================="
echo "gem5 Build Environment Setup"
echo "=========================================="
echo "Working directory: $SCRIPT_DIR"
echo ""

# Check if already set up
if [ -d ".pyenv-local" ] && [ -d "gem5-venv" ]; then
    echo "Build environment already exists!"
    echo ""
    read -p "Do you want to reinstall? This will delete existing setup. (y/N): " -n 1 -r
    echo
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        echo "Setup cancelled. Use ./build-gem5.sh to build."
        exit 0
    fi
    echo "Removing old environment..."
    rm -rf .pyenv-local gem5-venv
fi

echo "Step 1/5: Cloning pyenv..."
if [ ! -d ".pyenv-local" ]; then
    git clone https://github.com/pyenv/pyenv.git .pyenv-local
    echo "✓ pyenv cloned"
else
    echo "✓ pyenv already exists"
fi

echo ""
echo "Step 2/5: Installing Python 2.7.18..."
echo "This may take 5-10 minutes as it compiles from source..."
export PYENV_ROOT="$SCRIPT_DIR/.pyenv-local"
export PATH="$PYENV_ROOT/bin:$PATH"

if [ ! -d ".pyenv-local/versions/2.7.18" ]; then
    .pyenv-local/bin/pyenv install 2.7.18
    echo "✓ Python 2.7.18 installed"
else
    echo "✓ Python 2.7.18 already installed"
fi

echo ""
echo "Step 3/5: Installing pip for Python 2.7..."
if ! .pyenv-local/versions/2.7.18/bin/python -m pip --version &>/dev/null; then
    curl https://bootstrap.pypa.io/pip/2.7/get-pip.py -o /tmp/get-pip-py27.py
    .pyenv-local/versions/2.7.18/bin/python /tmp/get-pip-py27.py
    echo "✓ pip installed"
else
    echo "✓ pip already installed"
fi

echo ""
echo "Step 4/5: Creating virtual environment..."
.pyenv-local/versions/2.7.18/bin/pip install virtualenv
.pyenv-local/versions/2.7.18/bin/virtualenv gem5-venv
echo "✓ Virtual environment created"

echo ""
echo "Step 5/5: Installing SCons 2.5.1..."
source gem5-venv/bin/activate
pip install "scons==2.5.1"
deactivate
echo "✓ SCons 2.5.1 installed"

echo ""
echo "=========================================="
echo "Setup Complete!"
echo "=========================================="
echo ""
echo "Build environment is ready in: $SCRIPT_DIR"
echo ""
echo "To build gem5, run:"
echo "  ./build-gem5.sh"
echo ""
echo "The build script will automatically use the correct Python 2.7 environment."
echo ""
