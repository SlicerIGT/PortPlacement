set(proj NLopt)

# Set dependency list
set(${proj}_DEPENDS
  ""
  )

# Include dependent projects if any
ExternalProject_Include_Dependencies(${proj} PROJECT_VAR proj)

if(${SUPERBUILD_TOPLEVEL_PROJECT}_USE_SYSTEM_${proj})
  message(FATAL_ERROR "Enabling ${SUPERBUILD_TOPLEVEL_PROJECT}_USE_SYSTEM_${proj} is not supported !")
endif()

# Sanity checks
if(DEFINED NLopt_DIR AND NOT EXISTS ${NLopt_DIR})
  message(FATAL_ERROR "NLopt_DIR variable is defined but corresponds to nonexistent directory")
endif()

if(NOT DEFINED ${proj}_DIR AND NOT ${SUPERBUILD_TOPLEVEL_PROJECT}_USE_SYSTEM_${proj})

  ExternalProject_SetIfNotDefined(
    ${SUPERBUILD_TOPLEVEL_PROJECT}_${proj}_GIT_REPOSITORY
    "${EP_GIT_PROTOCOL}://github.com/stevengj/nlopt.git"
    QUIET
    )

  ExternalProject_SetIfNotDefined(
    ${SUPERBUILD_TOPLEVEL_PROJECT}_${proj}_GIT_TAG
    "014208e6bd03531aab8c45cf1902bec8b720df99" # master 2017-08-05
    QUIET
    )

  set(EP_SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj})
  set(EP_BINARY_DIR ${CMAKE_BINARY_DIR}/${proj}-build)

  ExternalProject_Add(${proj}
    ${${proj}_EP_ARGS}
    GIT_REPOSITORY "${${SUPERBUILD_TOPLEVEL_PROJECT}_${proj}_GIT_REPOSITORY}"
    GIT_TAG "${${SUPERBUILD_TOPLEVEL_PROJECT}_${proj}_GIT_TAG}"
    SOURCE_DIR ${EP_SOURCE_DIR}
    BINARY_DIR ${EP_BINARY_DIR}
    INSTALL_DIR ${proj}-install
    CMAKE_CACHE_ARGS
      # Compiler settings
      -DCMAKE_C_COMPILER:FILEPATH=${CMAKE_C_COMPILER}
      -DCMAKE_C_FLAGS:STRING=${ep_common_c_flags}
      -DCMAKE_CXX_COMPILER:FILEPATH=${CMAKE_CXX_COMPILER}
      -DCMAKE_CXX_FLAGS:STRING=${ep_common_cxx_flags}
      -DCMAKE_CXX_STANDARD:STRING=${CMAKE_CXX_STANDARD}
      -DCMAKE_CXX_STANDARD_REQUIRED:BOOL=${CMAKE_CXX_STANDARD_REQUIRED}
      -DCMAKE_CXX_EXTENSIONS:BOOL=${CMAKE_CXX_EXTENSIONS}
      # Output directories
      -DCMAKE_RUNTIME_OUTPUT_DIRECTORY:PATH=${CMAKE_BINARY_DIR}/${Slicer_THIRDPARTY_BIN_DIR}
      -DCMAKE_LIBRARY_OUTPUT_DIRECTORY:PATH=${CMAKE_BINARY_DIR}/${Slicer_THIRDPARTY_LIB_DIR}
      -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY:PATH=${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}
      # Install directories
      -DNLopt_INSTALL_RUNTIME_DIR:STRING=${Slicer_INSTALL_THIRDPARTY_LIB_DIR}
      -DNLopt_INSTALL_LIBRARY_DIR:STRING=${Slicer_INSTALL_THIRDPARTY_LIB_DIR}
      -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>

      -DBUILD_TESTING:BOOL=OFF
      -DBUILD_SHARED_LIBS:BOOL=OFF

      # TODO: check if needed
      #-DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
      #-DCMAKE_MACOSX_RPATH:BOOL=0

    DEPENDS
      ${${proj}_DEPENDS}
    )
  set(${proj}_DIR ${EP_BINARY_DIR})

  #TODO: check if needed
  #set(${proj}_DIR ${CMAKE_BINARY_DIR}/${proj}-install/lib/cmake/nlopt/)

else()
  ExternalProject_Add_Empty(${proj} DEPENDS ${${proj}_DEPENDS})
endif()

mark_as_superbuild(${proj}_DIR:PATH)
