#!/bin/sh                                                                                                                                                                    
#                                                                                                                                                                            
# @file   makeMacRelease.sh                                                                                                                                                         
# @author Magnus Sethson <magnus.sethson@liu.se>                                                                                                                             
# @date   2015-02-27                                                                                                                                                         
#                                                                                                                                                                            
# @brief Script for building, configuring, compiling and adjusting HopsanGUI for Mac OS X
#                                                                                                                                                                            
#$Id$                                                                                                                   
#  

rm -fr ../trunk/bin/*

# Building Dependencies

cd Dependencies

. unpackPatchAndBuildQwt.sh --force
. unpackAndBuildFMILibrary.sh --force

cd ..


# Building for RELEASE

. ./buildMacApp/build.sh --release

echo RELEASE built

# Building for DEBUG

. ./buildMacApp/build.sh --debug

echo DEBUG built

du -sh ./bin/HopsanGUI.app
du -sh ./bin/HopsanGUI_d.app

