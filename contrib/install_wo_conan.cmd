@REM call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\Tools\VsDevCmd.bat"
@REM call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\Common7\Tools\VsDevCmd.bat"

set CURR_LOC=%cd%
set INSTALL_DIR=%CURR_LOC%\..\SCC-Install
:parse
IF "%1"=="" GOTO endparse
set INSTALL_DIR=%1
:endparse

set SC_VERSION=2.3.4
set CXX_STD=14
set SCC_HOME=%INSTALL_DIR%\scc
set SYSTEMC_HOME=%INSTALL_DIR%\systemc

If "%VisualStudioVersion%" equ "17.0" goto seventeen
If "%VisualStudioVersion%" equ "16.0" goto sixteen
If "%VisualStudioVersion%" equ "15.0" goto fifteen
echo No valid VisualStudioVersion found, did you load envvars?
EXIT /B 1
:fifteen:
set GENERATOR=Visual Studio 15 2017
set TOOLSET=msvc-14.1
goto start_build
:sixteen:
set GENERATOR=Visual Studio 16 2019
set TOOLSET=msvc-14.2
goto start_build
:seventeen:
set GENERATOR=Visual Studio 17 2022
set TOOLSET=msvc-14.3
:start_build:

set CMAKE_OPTS=-G "%GENERATOR%" -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_GENERATOR_PLATFORM=x64 -DCMAKE_CXX_STANDARD=%CXX_STD%
set BOOST_LIB_EXCLUDE=--without-contract --without-fiber --without-graph --without-graph_parallel --without-iostreams --without-json --without-locale --without-log --without-math --without-mpi --without-nowide --without-python --without-random --without-stacktrace --without-test --without-timer --without-wave
@REM ############################################################################################
@REM build_boost
@REM ############################################################################################
if NOT EXIST "boost_1_85_0" tar xjvf boost_1_85_0.tar.bz2
pushd boost_1_85_0
@call .\bootstrap.bat
b2 -j8 toolset=%TOOLSET% address-model=64 architecture=x86 link=static threading=multi runtime-link=shared --build-type=minimal --build-dir=..\build\boost --stagedir=..\build\boost --prefix=%SCC_HOME% %BOOST_LIB_EXCLUDE% install
popd
@REM ############################################################################################
@REM build_fmt
@REM ############################################################################################
if NOT EXIST "fmt" tar xzvf fmt_8.0.1.tar.gz
cmake -S fmt -B build\fmt %CMAKE_OPTS% -DCMAKE_INSTALL_PREFIX=%SCC_HOME% -DBUILD_SHARED_LIBS=ON
cmake --build build\fmt -j 10 --target install
@REM ############################################################################################
@REM build_spdlog
@REM ############################################################################################
if NOT EXIST "spdlog" tar xzvf spdlog_1.9.2.tar.gz
cmake -S spdlog -B build\spdlog %CMAKE_OPTS% -DCMAKE_INSTALL_PREFIX=%SCC_HOME% -DSPDLOG_FMT_EXTERNAL=ON
cmake --build build\spdlog -j 10 --config Release --target install
@REM ############################################################################################
@REM build_yamlcpp
@REM ############################################################################################
if NOT EXIST "yaml-cpp" tar xzvf yaml-cpp_0.6.3.tar.gz
cmake -S yaml-cpp -B build\yaml-cpp %CMAKE_OPTS% -DCMAKE_INSTALL_PREFIX=%SCC_HOME% -DYAML_CPP_BUILD_TESTS=OFF -DYAML_CPP_BUILD_TOOLS=OFF
cmake --build build\yaml-cpp -j 10 --config Release --target install
@REM ############################################################################################
@REM build_zlib
@REM ############################################################################################
if NOT EXIST "zlib-1.3.1" tar xzvf zlib-1.3.1.tar.gz
cmake -S zlib-1.3.1 -B build\zlib %CMAKE_OPTS% -DCMAKE_INSTALL_PREFIX=%SCC_HOME%
cmake --build build\zlib -j 10 --config Release --target install
@REM ############################################################################################
@REM build_systemc
@REM ############################################################################################
if NOT EXIST "systemc" tar xzvf systemc_%SC_VERSION%.tar.gz
cmake -S systemc -B build\systemc %CMAKE_OPTS% -DENABLE_PHASE_CALLBACKS_TRACING=OFF -DCMAKE_INSTALL_PREFIX=%SYSTEMC_HOME%
cmake --build build\systemc -j 10 --config Release --target install
@REM ############################################################################################
@REM build_systemc_ams
@REM ############################################################################################
if NOT EXIST "systemc-ams-%SC_VERSION%" tar xzvf systemc-ams-%SC_VERSION%.tar.gz
cmake -S systemc-ams-%SC_VERSION% -B build/systemcams %CMAKE_OPTS% -DCMAKE_INSTALL_PREFIX=%SYSTEMC_HOME% 
cmake --build build/systemcams -j 10 --config Release --target install
@REM ############################################################################################
@REM build_scc
@REM ############################################################################################
if NOT EXIST "scc" tar xzvf scc.tar.gz
cmake -S scc -B build\scc -Wno-dev %CMAKE_OPTS% -DCMAKE_INSTALL_PREFIX=%SCC_HOME% -DENABLE_CONAN=OFF -DBoost_NO_SYSTEM_PATHS=TRUE -DBOOST_ROOT=%SCC_HOME% -DBOOST_INCLUDEDIR=%SCC_HOME%\include\boost-1_80 -DBoost_NO_WARN_NEW_VERSIONS=ON -DBoost_USE_STATIC_LIBS=ON -DBoost_USE_MULTITHREADED=ON -DBoost_USE_STATIC_RUNTIME=OFF
cmake --build build\scc -j 10 --config Release --target install
