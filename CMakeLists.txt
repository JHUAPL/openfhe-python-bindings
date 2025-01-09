#
# (c) 2021-2023 The Johns Hopkins University Applied Physics
# Laboratory LLC (JHU/APL).
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# 1. Redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above
# copyright notice, this list of conditions and the following
# disclaimer in the documentation and/or other materials provided
# with the distribution.
#
# 3. Neither the name of the copyright holder nor the names of its
# contributors may be used to endorse or promote products derived
# from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
# FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
# COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
# INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
# STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
# OF THE POSSIBILITY OF SUCH DAMAGE.
#

cmake_minimum_required (VERSION 3.21.1)

# set the project name
project(OpenFHEPythonBindings VERSION 1.0)
  
set(CMAKE_CXX_STANDARD 17)

find_package(OpenFHE)

set (Python_FIND_VIRTUALENV FIRST)


find_package(fmt REQUIRED)
find_package (Python3 COMPONENTS Interpreter Development)
find_package (Boost COMPONENTS numpy3 REQUIRED)

set( CMAKE_CXX_FLAGS "${OpenFHE_CXX_FLAGS} -DBOOST_NO_AUTO_PTR -Wno-error=deprecated-declarations -Wno-error=unused-variable -Wno-error" )
set( CMAKE_EXE_LINKER_FLAGS ${OpenFHE_EXE_LINKER_FLAGS} )

message(STATUS "OpenFHE directory: " ${OpenFHE_INCLUDE})

include_directories(include/)
include_directories( ${OPENMP_INCLUDES} )
include_directories( ${OpenFHE_INCLUDE} )
include_directories( ${OpenFHE_INCLUDE}/third-party/include )
include_directories( ${OpenFHE_INCLUDE}/core )
include_directories( ${OpenFHE_INCLUDE}/pke )
include_directories( ${OpenFHE_INCLUDE}/binfhe )
include_directories(/openFHE/fmt/include/)

include_directories(${Python3_INCLUDE_DIRS})
include_directories(${Python3_ROOT_DIR}/include/python3.10)
include_directories(${Python3_ROOT_DIR}/include)
include_directories(${Boost_INCLUDE_DIRS})

link_directories( ${Python3_ROOT_DIR}/lib )
link_directories( ${OpenFHE_LIBDIR} )
link_directories( ${OPENMP_LIBRARIES} )

link_libraries( ${OpenFHE_SHARED_LIBRARIES} )

file (GLOB_RECURSE PYTHON_SRC_FILES CONFIGURE_DEPENDS src/*.cpp)

add_library (pythonobj OBJECT ${PYTHON_SRC_FILES})
set_property(TARGET pythonobj PROPERTY POSITION_INDEPENDENT_CODE 1)

add_library (pyOpenFHE SHARED $<TARGET_OBJECTS:pythonobj>)
set_property(TARGET pyOpenFHE PROPERTY VERSION 1)
set_property(TARGET pyOpenFHE PROPERTY SOVERSION 1)
set_property(TARGET pyOpenFHE PROPERTY RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib/)
set_property(TARGET pyOpenFHE PROPERTY PREFIX "")
install(TARGETS pyOpenFHE DESTINATION lib)

target_link_directories(pyOpenFHE PUBLIC ${pyOpenFHE_DIR}/lib)

target_link_libraries (pyOpenFHE PUBLIC ${Boost_LIBRARIES} ${THIRDPARTYLIBS} ${OpenMP_CXX_FLAGS} fmt::fmt)
