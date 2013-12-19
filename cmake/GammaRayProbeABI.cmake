# Determine probe ABI identifier
#
# This contains all properties that define ABI compatibility of a probe with a target
# It includes:
# - Qt version (major/minor)
# - Compiler (Windows only)
# - Release/Debug (Windows only) TODO
# - processor architecture

if(Qt5Core_FOUND)
  set(GAMMARAY_PROBE_ABI "qt${Qt5Core_VERSION_MAJOR}.${Qt5Core_VERSION_MINOR}")
else()
 set(GAMMARAY_PROBE_ABI "qt${QT_VERSION_MAJOR}.${QT_VERSION_MINOR}")
endif()

if(WIN32)
  set(GAMMARAY_PROBE_ABI "${GAMMARAY_PROBE_ABI}-${CMAKE_CXX_COMPILER_ID}")
endif()

set(GAMMARAY_PROBE_ABI "${GAMMARAY_PROBE_ABI}-${CMAKE_SYSTEM_PROCESSOR}")

message(STATUS "Building probe for ABI: ${GAMMARAY_PROBE_ABI}")