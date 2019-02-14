#!/bin/bash
# $Id$

# Shell script building HopsaGUI dependency PythonQt automatically
# Author: Peter Nordin peter.nordin@liu.se

set -e

pyqtversion=c07f0
pyversion=3
basedir=$(pwd)

if [ $# -lt 1 ]; then
  echo "Error: To few input arguments!"
  echo "Usage: `basename $0` {release, debug} [pyversion] [pyqtversion]"
  echo "       pyversion is optional, (c07f0 default)"
  echo "       pyqtversion is optional, (3 default)"
  exit $E_BADARGS
fi

if [ $# -gt 1 ]; then
  pyversion="$2"
fi

if [ $# -gt 2 ]; then
  pyqtversion="$3"
fi

name=pythonqt

./download-dependencies.py ${name}:${pyqtversion}

codedir=${basedir}/${name}-code
builddir=${basedir}/${name}-build
installdir=${basedir}/${name}
E_BADARGS=65

# Include general settings
source setHopsanBuildPaths.sh

cd ${codedir}

# Remove extensions tests and examples to speedup build
sed "s|extensions tests examples||" -i PythonQt.pro

# Prevent collision of DEBUG_EXT
sed "s|([^_])DEBUG_EXT|\1PYTHONQT_DEBUG_EXT|" -ri build/PythonQt.prf

# Set build mode
if [[ "$1" != "release" ]]; then
  sed "s|#CONFIG += debug_and_release build_all|CONFIG += debug_and_release build_all|" -i build/common.prf
fi

# Set python version
sed "s|unix:PYTHON_VERSION=.*|unix:PYTHON_VERSION=${pyversion}|" -i build/python.prf

# Build in build dir
mkdir -p $builddir
cd $builddir

${HOPSAN_BUILD_QT_QMAKE} ${codedir}/PythonQt.pro -r -spec linux-g++
make -j$(getconf _NPROCESSORS_ONLN) -w

# Install manually since PythonQt code does not have install target configured
mkdir -p ${installdir}/include
cp -a lib ${installdir}
cd ${codedir}/src
find -name "*.h" -exec cp -a --parents {} ${installdir}/include \;
mkdir -p ${installdir}/build
cd ${codedir}/build
cp -a PythonQt.prf ${installdir}/build
cp -a python.prf ${installdir}/build
sed "s|INCLUDEPATH.*|INCLUDEPATH += \$\$PWD/../include|" -i ${installdir}/build/PythonQt.prf
sed "s|\$\$DESTDIR|\$\$PWD|" -i ${installdir}/build/PythonQt.prf

cd $basedir
echo "setupPythonQt.sh done!"
