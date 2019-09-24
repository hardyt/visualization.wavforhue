cmake_minimum_required(VERSION 3.5)
project(visualization.wavforhue)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR})

find_package(Kodi REQUIRED)
find_package(CURL REQUIRED)
find_package(glm REQUIRED)

if(NOT WIN32 AND (APP_RENDER_SYSTEM STREQUAL "gl" OR NOT APP_RENDER_SYSTEM))
  find_package(OpenGl REQUIRED)
  set(DEPLIBS ${OPENGL_LIBRARIES})
  set(includes ${OPENGL_INCLUDE_DIR})
  add_definitions(${OPENGL_DEFINITIONS})
else()
  find_package(OpenGLES REQUIRED)
  set(DEPLIBS ${OPENGLES_LIBRARIES})
  set(includes ${OPENGLES_INCLUDE_DIR})
  add_definitions(${OPENGLES_DEFINITIONS})
endif()

include_directories(${includes}
                    ${CURL_INCLUDE_DIR}
                    ${GLM_INCLUDE_DIR}
                    ${KODI_INCLUDE_DIR}/..) # Hack way with "/..", need bigger Kodi cmake rework to match right include ways (becomes done in future)

set(WAVFORHUE_SOURCES src/Main.cpp
                      src/FFT.cpp
                      src/jsoncpp.cpp
                      src/WavforHue.cpp
                      src/WavforHue_Thread.cpp)

set(WAVFORHUE_HEADERS src/Main.h
                      src/FFT.h
                      src/WavforHue.h
                      src/WavforHue_Thread.h)

list(APPEND DEPLIBS ${CURL_LIBRARIES})

build_addon(visualization.wavforhue WAVFORHUE DEPLIBS)

include(CPack)
