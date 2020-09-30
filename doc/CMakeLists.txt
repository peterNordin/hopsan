option(BUILD_USER_DOCUMENTATION "Build user documentation" OFF])

if (${BUILD_USER_DOCUMENTATION})

  find_package(Doxygen REQUIRED dot)

  set(DOXYGEN_BUILTIN_STL_SUPPORT    YES)
  set(DOXYGEN_DISTRIBUTE_GROUP_DOC   YES)
  set(DOXYGEN_EXTRACT_STATIC         YES)
  set(DOXYGEN_CASE_SENSE_NAMES       YES)
  set(DOXYGEN_SORT_GROUP_NAMES       YES)
  set(DOXYGEN_GENERATE_TODOLIST      NO)
  set(DOXYGEN_SHOW_FILES             NO)
  set(DOXYGEN_WARN_IF_UNDOCUMENTED   NO)
  set(DOXYGEN_WARN_LOGFILE           doxygenUserWarnings.log)
  set(DOXYGEN_FILE_PATTERNS          *.cc *.cpp *.h *.hpp *.dox)
  set(DOXYGEN_EXAMPLE_PATH           ${PROJECT_SOURCE_DIR}/componentLibraries/exampleComponentLib)
  set(DOXYGEN_IMAGE_PATH             ${PROJECT_SOURCE_DIR}/doc/graphics)
  set(DOXYGEN_ALPHABETICAL_INDEX     NO)
  set(DOXYGEN_GENERATE_HTML          YES)
  set(DOXYGEN_HTML_EXTRA_STYLESHEET  hopsan.css)
  set(DOXYGEN_SEARCHENGINE           NO)
  set(DOXYGEN_CALL_GRAPH             YES)
  set(DOXYGEN_CALLER_GRAPH           YES)
  set(DOXYGEN_DOT_IMAGE_FORMAT       svg)
  
  # On MINGW CMake TO_NATIVE_PATH inside FindDoxygen does not work, and we need to fix the path here
  if (MINGW)
    string(REPLACE "\\" "" DOXYGEN_DOT_PATH "${DOXYGEN_DOT_PATH}")
    string(REPLACE "/" "\\" DOXYGEN_DOT_PATH "${DOXYGEN_DOT_PATH}")
	message(STATUS "Using dot path: ${DOXYGEN_DOT_PATH}")
  endif()

  doxygen_add_docs(
    user-documentation
    ALL
    ${PROJECT_SOURCE_DIR}/HopsanCore/include
    ${PROJECT_SOURCE_DIR}/HopsanCore/src
    ${PROJECT_SOURCE_DIR}/doc
    COMMENT "Generate HTML documentation"
  )

  install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/html
          DESTINATION ./doc)
endif()
