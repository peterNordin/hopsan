# -------------------------------------------------
# Global project options
# -------------------------------------------------
include( ../Common.prf )
include( HopsanGeneratorBuild.prf )

TARGET = HopsanGenerator
TEMPLATE = lib
CONFIG += shared
DESTDIR = $${PWD}/../bin
TARGET = $${TARGET}$${DEBUG_EXT}

QT -= gui
QT += core xml

# Keep these QtGui related options by them self so that external scrip may deactivate them when needed
useqtgui=True
contains( useqtgui, True ) {
    DEFINES += USEQTGUI
    QT += gui
    isEqual(QT_MAJOR_VERSION, 5){
        QT += widgets
    }
}



#--------------------------------------------------------
# Set the FMILibrary paths and dll/so/dylib/framework post linking copy command
include($${PWD}/../Dependencies/fmilibrary.pri)

#--------------------------------------------------
# Add the include path to our self, (HopsanGenerator)
INCLUDEPATH *= $${PWD}/include/
#--------------------------------------------------

#--------------------------------------------------
# Add the include path to (HopsanCore)
INCLUDEPATH *= $${PWD}/../HopsanCore/include/
LIBS *= -L$${PWD}/../bin -lHopsanCore$${DEBUG_EXT}
#--------------------------------------------------

#--------------------------------------------------
# Add the include path to (SymHop)
INCLUDEPATH *= $${PWD}/../SymHop/include/
LIBS *= -L$${PWD}/../bin -lSymHop$${DEBUG_EXT}
#--------------------------------------------------

# -------------------------------------------------
# Non platform specific HopsanCompGen options
# -------------------------------------------------
CONFIG(debug, debug|release) {
  QMAKE_CXXFLAGS += -pedantic -Wno-long-long
}
CONFIG(release, debug|release) {

}

# -------------------------------------------------
# Platform specific additional project options
# -------------------------------------------------
win32 {
    #DEFINES += STATICCORE      #Use this if you are compiling the generator into a program directly or building a static lib
    DEFINES += DOCOREDLLEXPORT  #Use this if you are compiling the generator as a DLL or SO
    DEFINES -= UNICODE
}
unix { 
}

# -------------------------------------------------
# Project files
# -------------------------------------------------
SOURCES += \
    src/HopsanGeneratorLib.cc \
    src/GeneratorUtilities.cc \
    src/generators/HopsanGenerator.cc \
    src/generators/HopsanSimulinkGenerator.cc \
    src/generators/HopsanModelicaGenerator.cc \
    src/generators/HopsanFMIGenerator.cc \
    src/generators/HopsanLabViewGenerator.cc

HEADERS += \
    include/win32dll.h \
    include/GeneratorUtilities.h \
    include/generators/HopsanGenerator.h \
    include/generators/HopsanModelicaGenerator.h \
    include/generators/HopsanSimulinkGenerator.h \
    include/generators/HopsanFMIGenerator.h \
    include/generators/HopsanLabViewGenerator.h

RESOURCES += \
    templates.qrc








