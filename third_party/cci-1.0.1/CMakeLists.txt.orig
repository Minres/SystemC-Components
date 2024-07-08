###############################################################################
#
# Licensed to Accellera Systems Initiative Inc. (Accellera) under one or
# more contributor license agreements.  See the NOTICE file distributed
# with this work for additional information regarding copyright ownership.
# Accellera licenses this file to you under the Apache License, Version 2.0
# (the "License"); you may not use this file except in compliance with the
# License.  You may obtain a copy of the License at
#
#  http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
# implied.  See the License for the specific language governing
# permissions and limitations under the License.
#
###############################################################################

###############################################################################
#
# src/CMakeLists.txt --
# CMake script to configure the SystemC sources and to generate native
# Makefiles and project workspaces for your compiler environment.
#
# Original Author: Torsten Maehne, Université Pierre et Marie Curie, Paris,
#                  2013-06-11
#
###############################################################################

###############################################################################
#
# MODIFICATION LOG - modifiers, enter your name, affiliation, date and
# changes you are making here.
#
#     Name, Affiliation, Date:
# Description of Modification:
#
###############################################################################




###############################################################################
# Build rules for SystemC library
###############################################################################

add_library (cci
                        cci/core/cci_name_gen.h
                        cci/core/cci_value_iterator.h
                        cci/core/cci_meta.h
                        cci/core/cci_cmnhdr.h
                        cci/core/cci_value.h
                        cci/core/cci_callback.h
                        cci/core/cci_value_converter.cpp
                        cci/core/cci_filtered_range.h
                        cci/core/cci_callback_impl.h
                        cci/core/cci_name_gen.cpp
                        cci/core/cci_version.h
                        cci/core/rapidjson.h
                        cci/core/cci_core_types.h
                        cci/core/systemc.h
                        cci/core/cci_value_converter.h
                        cci/core/cci_value.cpp
                        cci/utils/consuming_broker.cpp
                        cci/utils/broker.h
                        cci/utils/broker.cpp
                        cci/utils/consuming_broker.h
                        cci/cfg/cci_mutable_types.h
                        cci/cfg/cci_broker_callbacks.h
                        cci/cfg/cci_broker_types.h
                        cci/cfg/cci_report_handler.h
                        cci/cfg/cci_broker_handle.h
                        cci/cfg/cci_report_handler.cpp
                        cci/cfg/cci_param_if.cpp
                        cci/cfg/cci_broker_manager.cpp
                        cci/cfg/cci_param_untyped.cpp
                        cci/cfg/cci_param_untyped_handle.cpp
                        cci/cfg/cci_macros_undef.h
                        cci/cfg/cci_param_untyped.h
                        cci/cfg/cci_broker_if.h
                        cci/cfg/cci_param_untyped_handle.h
                        cci/cfg/cci_param_typed_handle.h
                        cci/cfg/cci_broker_manager.h
                        cci/cfg/cci_originator.cpp
                        cci/cfg/cci_param_typed.h
                        cci/cfg/cci_config_macros.h
                        cci/cfg/cci_broker_handle.cpp
                        cci/cfg/cci_originator.h
                        cci/cfg/cci_param_callbacks.h
                        cci/cfg/cci_param_if.h
                        )

# Find SystemC package - this will generate appropriate warnings if
# it's not found. As the user is using SystemC, the cmake files should
# be available.
set (CMAKE_PREFIX_PATH /opt/systemc)
if ( NOT SystemCLanguage_FOUND )
  find_package(SystemCLanguage CONFIG REQUIRED)
ENDIF ()

set (CMAKE_CXX_STANDARD ${SystemC_CXX_STANDARD} CACHE STRING
     "C++ standard to build all targets. Supported values are 98, 11, and 14.")
set (CMAKE_CXX_STANDARD_REQUIRED ${SystemC_CXX_STANDARD_REQUIRED} CACHE BOOL
     "The with CMAKE_CXX_STANDARD selected C++ standard is a requirement.")

find_package(RapidJSON CONFIG REQUIRED)

target_link_libraries(cci SystemC::systemc)


add_library (SystemC::cci ALIAS cci)

target_compile_definitions (
  cci
  PUBLIC
  $<$<BOOL:${WIN32}>:WIN32>
  $<$<AND:$<BOOL:${BUILD_SHARED_LIBS}>,$<OR:$<BOOL:${WIN32}>,$<BOOL:${CYGWIN}>>>:
    SC_WIN_DLL>
  PRIVATE
  SC_BUILD
  SC_INCLUDE_FX
  $<$<AND:$<BOOL:${WIN32}>,$<BOOL:${MSVC}>>:_LIB>
  )

target_compile_options(
  cci
  PUBLIC
  $<$<CXX_COMPILER_ID:MSVC>:/vmg /MP>
  PRIVATE
  $<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>:
    -Wall -Wextra -Wno-unused-parameter -Wno-unused-variable>
  $<$<CXX_COMPILER_ID:MSVC>:/W3 /wd4244 /wd4267 /wd4996>)

target_include_directories(cci
  PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
  PRIVATE
  ${RAPIDJSON_INCLUDE_DIRS})

if (APPLE)
  # It's OK that _sc_main is an undefined symbol.
  set_target_properties (cci PROPERTIES LINK_FLAGS -Wl,-U,_sc_main)
endif (APPLE)

set_target_properties (cci
                       PROPERTIES
                       LINKER_LANGUAGE CXX
                       VERSION ${SystemCCCI_VERSION}
                       SOVERSION ${SystemCCCI_SOVERSION}
                       INSTALL_NAME_DIR ${CMAKE_INSTALL_FULL_LIBDIR})


###############################################################################
# Install rules for CCI library
###############################################################################

install (TARGETS cci EXPORT  SystemCCCITargets
                         RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
                         LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
                         ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
                         COMPONENT lib)

# Install the CCI
install (DIRECTORY ./
         DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
         COMPONENT dev
         FILES_MATCHING PATTERN "*.h")
install (DIRECTORY ./
         DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
         COMPONENT dev
         FILES_MATCHING PATTERN "*.hpp")
install (FILES "cci_configuration"
         DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
         COMPONENT dev)
