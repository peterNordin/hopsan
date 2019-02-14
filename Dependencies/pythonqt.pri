# Add debug if debugmode (only default on for windows)
dbg_ext =
win32:CONFIG(debug, debug|release):dbg_ext = _d
unix:CONFIG(debug, debug|release):dbg_ext =

# Set hompath and libname
pythonqt_home = $${PWD}/pythonqt
pythonqt_lib = $${pythonqt_home}/lib
libname = PythonQt

defineTest(have_local_pythonqt) {
  exists($${pythonqt_lib}) {
    return(true)
  }
  return(false)
}

defineTest(have_system_pythonqt) {
  packagesExist(PythonQt-Qt5-Python3) {
    return(true)
  }
  return(false)
}

defineTest(have_pythonqt) {
  have_local_pythonqt() {
    return(true)
  }
  have_system_pythonqt() {
    return(true)
  }
  return(false)
}

have_local_pythonqt() {
  include($${pythonqt_home}/build/PythonQt.prf)
  # Note! The RPATH is absolute and only meant for dev builds in the IDE, on release runtime paths should be stripped
  QMAKE_RPATHDIR *= $${pythonqt_lib}
  message(Found local PythonQt)
} else:have_system_pythonqt() {
  CONFIG *= link_pkgconfig
  PKGCONFIG *= PythonQt-Qt5-Python3
  PKGCONFIG *= python3
  message(Found system pkg-config PythonQt)
}
