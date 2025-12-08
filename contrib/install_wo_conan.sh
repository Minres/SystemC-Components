#!/bin/bash
##

# Absolute path to this script, e.g. /home/user/bin/foo.sh
SCRIPT=`readlink -f "$0"`
# Absolute path this script is in, thus /home/user/bin
SCRIPTDIR=`dirname "$SCRIPT"`
SCRIPTNAME=`basename "$SCRIPT"`

function print_help {
    echo "Usage: $SCRIPTNAME [-h] [-c] <install dir>"
    echo "Build SCC installation from tar files"
    echo "Optional cli arguments:"
    echo "  -h              print help"
    echo "  -c              clean build and install directory before building"
}

CLEAN=0

while [ $# -gt 0 ]; do
    unset OPTIND
    unset OPTARG
    while getopts hc  options; do
        case $options in
            c) CLEAN=1 ;;
            h) print_help; exit 0 ;;
        esac
    done
    shift $((OPTIND-1))
    if [ ! -z "$1" ]; then
        INSTALL_ROOT="$1"
        shift
    fi
done

if [ -z "${INSTALL_ROOT}" ]; then
    echo "Missing install dir argument"
    exit 1
fi

export CXX_STD=20
export CC=$(type -p gcc)
export CXX=$(type -p g++)
[ -z "${BUILD_TYPE}" ] && BUILD_TYPE=RelWithDebInfo
export SC_VERSION=2.3.4
export SYSTEMC_HOME=${INSTALL_ROOT}/systemc
export SYSTEMCAMS_HOME=${INSTALL_ROOT}/systemc
export SCC_INSTALL=${INSTALL_ROOT}/scc
OS=$(uname)
if [ $OS == "Linux" ]; then
    DISTRO=$(lsb_release -i -s)
else
    DISTRO="Darwin"
fi
if [ "$DISTRO" == "Ubuntu" ]; then
    BOOST_LIBDIR=
    YAML_LIBDIR=
else
    BOOST_LIBDIR=--libdir=${SCC_INSTALL}/lib64
    YAML_LIBDIR=-DLIB_SUFFIX=64
fi
# we need to keep CMAKE_POLICY_VERSION_MINIMUM=3.5 unless yaml-cpp, SystemC & SystemC-AMS have fixed their build system
CMAKE_COMMON_SETTINGS="-DCMAKE_BUILD_TYPE=${BUILD_TYPE} -DBUILD_SHARED_LIBS=OFF -DCMAKE_CXX_STANDARD=${CXX_STD} -DCMAKE_POLICY_VERSION_MINIMUM=3.5"
BOOST_SETTINGS="link=static cxxflags='-std=c++${CXX_STD}'"
set -eup -o pipefail
############################################################################################
#
############################################################################################
if [ ${CLEAN} -eq 1 ]; then
    echo Removing fmt fmt_*.tar.gz spdlog spdlog_*.tar.gz yaml-cpp yaml-cpp_*.tar.gz scc.tar.gz
    rm -rf build ${INSTALL_ROOT} fmt fmt_*.tar.gz spdlog spdlog_*.tar.gz yaml-cpp yaml-cpp_*.tar.gz scc.tar.gz
fi
############################################################################################
#
############################################################################################
function build_boost {
    export BOOST_LIB_EXCLUDE=contract,fiber,graph,graph_parallel,iostreams,json,locale,log,math,mpi,nowide,python,random,stacktrace,test,timer,wave

    if [ ! -d boost_1_89_0 ]; then
        [ -f boost_1_89_0.tar.bz2 ] || wget https://archives.boost.io/release/1.89.0/source/boost_1_89_0.tar.bz2
        tar xjf boost_1_89_0.tar.bz2
    fi
    (cd boost_1_89_0; \
      ./bootstrap.sh --prefix=${SCC_INSTALL} ${BOOST_LIBDIR} --without-libraries=${BOOST_LIB_EXCLUDE};\
      ./b2 ${BOOST_SETTINGS} install) || exit 2
}
############################################################################################
#
############################################################################################
function build_fmt {
    if [ ! -d fmt ]; then
        if [ ! -f fmt_12.0.0.tar.gz ]; then
            git clone --depth 1 --branch 12.0.0 -c advice.detachedHead=false https://github.com/fmtlib/fmt.git
            tar czf fmt_12.0.0.tar.gz --exclude=.git fmt 
        else
            tar xzf fmt_12.0.0.tar.gz
        fi
    fi
    cmake -S fmt -B build/fmt ${CMAKE_COMMON_SETTINGS} -DCMAKE_INSTALL_PREFIX=${SCC_INSTALL} -DFMT_TEST=OFF || exit 1
    cmake --build build/fmt -j 10 --target install || exit 2
}
############################################################################################
#
############################################################################################
function build_spdlog {
    if [ ! -d spdlog ]; then
        if [ ! -f spdlog_1.16.0.tar.gz ]; then
            git clone --depth 1 --branch v1.16.0 -c advice.detachedHead=false https://github.com/gabime/spdlog.git
            tar czf spdlog_1.16.0.tar.gz --exclude=.git spdlog 
        else
            tar xzf spdlog_1.16.0.tar.gz
        fi
    fi
    cmake -S spdlog -B build/spdlog ${CMAKE_COMMON_SETTINGS} -DCMAKE_INSTALL_PREFIX=${SCC_INSTALL} || exit 1
    cmake --build build/spdlog -j 10 --target install || exit 2
}
############################################################################################
#
############################################################################################
function build_yamlcpp {
    if [ ! -d yaml-cpp ]; then
        if [ ! -f yaml-cpp_0.8.0.tar.gz ]; then
            git clone --depth 1 --branch 0.8.0 -c advice.detachedHead=false https://github.com/jbeder/yaml-cpp.git
            tar czf yaml-cpp_0.8.0.tar.gz --exclude=.git yaml-cpp  
        else
            tar xzf yaml-cpp_0.8.0.tar.gz
        fi
    fi
    cmake -S yaml-cpp -B build/yaml-cpp ${CMAKE_COMMON_SETTINGS} -DCMAKE_INSTALL_PREFIX=${SCC_INSTALL} \
        -DYAML_CPP_BUILD_TESTS=OFF -DYAML_CPP_BUILD_TOOLS=OFF ${YAML_LIBDIR} || exit 1
    cmake --build build/yaml-cpp -j 10 --target install || exit 2
    rm  -rf ~/.cmake/packages/yaml-cpp
}
############################################################################################
#
############################################################################################
function build_lz4 {
    if [ ! -d lz4 ]; then
        if [ ! -f lz4_1.10.0.tar.gz ]; then
            git clone --depth 1 --branch v1.10.0 -c advice.detachedHead=false https://github.com/lz4/lz4.git
            tar czf lz4_1.10.0.tar.gz --exclude=.git lz4 
        else
            tar xzf lz4_1.10.0.tar.gz
        fi
    fi
    make -C lz4 clean all || exit 1
    make -C lz4 install PREFIX=${SCC_INSTALL} LIBDIR=${SCC_INSTALL}/lib64 || exit 2
    export PKG_CONFIG_PATH=${SCC_INSTALL}/lib64/pkgconfig
}
############################################################################################
#
############################################################################################
function build_systemc {
    if [ ! -d systemc ]; then
        if [ ! -f systemc_${SC_VERSION}.tar.gz ]; then
            git clone --depth 1 --branch 2.3.4 -c advice.detachedHead=false https://github.com/accellera-official/systemc.git
            tar czf systemc_${SC_VERSION}.tar.gz systemc --exclude=.git
        else
            tar xzf systemc_${SC_VERSION}.tar.gz
        fi
    fi
    cmake -S systemc -B build/systemc ${CMAKE_COMMON_SETTINGS} -DCMAKE_INSTALL_PREFIX=${SYSTEMC_HOME} -DENABLE_PHASE_CALLBACKS_TRACING=OFF || exit 1
    cmake --build build/systemc -j 10 --target install || exit 2
    rm  -rf ~/.cmake/packages/SystemC*
}
############################################################################################
#
############################################################################################
function build_systemc_ams {
    if [ ! -d systemc-ams-${SC_VERSION} ]; then
        if [ ! -f systemc-ams-${SC_VERSION}.tar.gz ]; then
            echo "systemc-ams-${SC_VERSION}.tar.gz missing!"
            exit 2
        fi
        tar xzf systemc-ams-${SC_VERSION}.tar.gz
    fi
    cmake -S systemc-ams-${SC_VERSION} -B build/systemc-ams ${CMAKE_COMMON_SETTINGS} -DCMAKE_INSTALL_PREFIX=${SYSTEMCAMS_HOME} || exit 1
    cmake --build build/systemc-ams -j 10 --target install || exit 2
    rm  -rf ~/.cmake/packages/SystemC*
}
############################################################################################
#
############################################################################################
function build_scc {
    if [ ! -d scc ]; then
        if [ ! -f scc.tar.gz ]; then
            git clone --recursive --branch develop -c advice.detachedHead=false https://github.com/Minres/SystemC-Components.git scc
            tar czf scc.tar.gz --exclude=.git scc
        else
            tar xzf scc.tar.gz
        fi
    elif [ ! -f scc.tar.gz ]; then
        (cd scc; git pull; git submodule update --recursive)
        tar czf scc.tar.gz --exclude=.git scc 
    fi
    cmake -S scc -B build/scc -Wno-dev ${CMAKE_COMMON_SETTINGS} -DCMAKE_INSTALL_PREFIX=${SCC_INSTALL} \
        -DBoost_NO_SYSTEM_PATHS=TRUE -DBOOST_ROOT=${SCC_INSTALL} -DBoost_NO_WARN_NEW_VERSIONS=ON -DBUILD_SCC_LIB_ONLY=ON || exit 1
    cmake --build build/scc -j 10 --target install || exit 2
}

build_boost
build_fmt
build_spdlog
build_yamlcpp
build_systemc
build_systemc_ams
build_scc
