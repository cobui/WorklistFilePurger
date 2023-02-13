# Orthanc - A Lightweight, RESTful DICOM Store
# Copyright (C) 2012-2016 Sebastien Jodogne, Medical Physics
# Department, University Hospital of Liege, Belgium
# Copyright (C) 2017-2020 Osimis S.A., Belgium
#
# This program is free software: you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation, either version 3 of the
# License, or (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.


cmake_minimum_required(VERSION 2.8)

project(WorklistFilePurger)

if (ORTHANC_PLUGIN_VERSION STREQUAL "mainline")
    set(ORTHANC_FRAMEWORK_VERSION "mainline")
    set(ORTHANC_FRAMEWORK_DEFAULT_SOURCE "hg")
else ()
    set(ORTHANC_FRAMEWORK_VERSION "1.9.0")
    set(ORTHANC_FRAMEWORK_DEFAULT_SOURCE "web")
endif ()



include(${CMAKE_SOURCE_DIR}/Resources/CMake/WorklistRemoverPluginParameters.cmake)
include(${ORTHANC_DIR}/OrthancServer/Plugins/Samples/Common/OrthancPlugins.cmake)
include(${ORTHANC_DIR}/OrthancFramework/Resources/CMake/JsonCppConfiguration.cmake)
include(${ORTHANC_DIR}/OrthancFramework/Resources/CMake/BoostConfiguration.cmake)
include(${ORTHANC_DIR}/OrthancFramework/Resources/CMake/GoogleTestConfiguration.cmake)
include(${ORTHANC_DIR}/OrthancFramework/Resources/CMake/OrthancFrameworkConfiguration.cmake)

SET(CMAKE_CXX_FLAGS "-std=c++17")

add_library(WorklistFilePurger SHARED
    Plugins/Plugin.cpp
    Plugins/Purger.cpp
    ${CMAKE_SOURCE_DIR}/../Common/OrthancPluginCppWrapper.cpp
    ${JSONCPP_SOURCES}
    ${BOOST_SOURCES}
    ${LOGGING_SOURCES}
)

message("Setting the version of the plugin to ${WORKLIST_FILE_PURGER_VERSION}")

add_definitions(
  -DWORKLIST_FILE_PURGER_VERSION="${WORKLIST_FILE_PURGER_VERSION}"
  )

set_target_properties(WorklistFilePurger PROPERTIES 
  VERSION ${WORKLIST_FILE_PURGER_VERSION} 
  SOVERSION ${WORKLIST_FILE_PURGER_VERSION}
  COMPILE_FLAGS -DORTHANC_ENABLE_LOGGING_PLUGIN=1
  )

install(
  TARGETS WorklistFilePurger
  RUNTIME DESTINATION lib    # Destination for Windows
  LIBRARY DESTINATION share/orthanc/plugins    # Destination for Linux
  )


add_executable(UnitTest
    UnitTests/UnitTestMain.cpp
    Plugins/Plugin.cpp
    Plugins/Purger.cpp
    ${CMAKE_SOURCE_DIR}/../Common/OrthancPluginCppWrapper.cpp
    ${JSONCPP_SOURCES}
    ${GOOGLE_TEST_SOURCES}
    ${BOOST_SOURCES}
    ${LOGGING_SOURCES}
  )


target_link_libraries(UnitTest ${GOOGLE_TEST_LIBRARIES})
