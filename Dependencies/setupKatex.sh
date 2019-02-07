#!/bin/bash

# Shell script to extract pre buikt files from katex release tag
# Author: Peter Nordin peter.nordin@liu.se

basedir=`pwd`
name=katex
codedir=${basedir}/${name}-code
builddir=${basedir}/${name}-build
installdir=${basedir}/${name}

cp -a ${codedir} ${installdir}

echo "setupKatex.sh done!"
