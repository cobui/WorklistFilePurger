# Copyright (C) 2012-2016 Sebastien Jodogne, Medical Physics
# Department, University Hospital of Liege, Belgium
# Copyright (C) 2017-2021 Osimis S.A., Belgium
#
# This program is free software: you can redistribute it and/or
# modify it under the terms of the GNU Affero General Public License
# as published by the Free Software Foundation, either version 3 of
# the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Affero General Public License for more details.
#
# You should have received a copy of the GNU Affero General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.

SET(WORKLIST_FILE_PURGER_VERSION "0.2" CACHE STRING "Version of the plugin")
SET(ALLOW_DOWNLOADS OFF CACHE BOOL "Allow CMake to download packages")
SET(USE_SYSTEM_JSONCPP ON CACHE BOOL "Use the system version of JsonCpp")
SET(USE_SYSTEM_BOOST ON CACHE BOOL "Use the system version of boost")

SET(ALLOW_DOWNLOADS ON CACHE BOOL "Allow CMake to download packages")
SET(ORTHANC_FRAMEWORK_SOURCE "${ORTHANC_FRAMEWORK_DEFAULT_SOURCE}")
SET(ORTHANC_FRAMEWORK_ARCHIVE "")
SET(ORTHANC_FRAMEWORK_ROOT "")


include(${CMAKE_SOURCE_DIR}/Resources/CMake/LoggingSources.cmake)