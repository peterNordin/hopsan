cmake_minimum_required(VERSION 3.0)
project(AutoLibs)

install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/ DESTINATION componentLibraries/autoLibs
  FILES_MATCHING
  PATTERN "*.cpp"
  PATTERN "*.h"
  PATTERN "*.hpp"
  PATTERN "*.cci"
  PATTERN "*.xml"
  PATTERN "*.nb"
  PATTERN "*.py"
  PATTERN "*.bat"
  PATTERN "*.sh"
  PATTERN "*.csv"
  PATTERN "*.txt"
  PATTERN "*.md"
  PATTERN "*.hmf"
  PATTERN "*.svg"
  PATTERN "*.mo"
  PATTERN "LICENSE"
  PATTERN "CMakeLists.txt" EXCLUDE)
