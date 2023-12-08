#!/bin/bash
##

CXX_STD=14
CC=$(type -p cc)
CXX=$(type -p c++)
[ -z "${BUILD_TYPE}" ] && BUILD_TYPE=RelWithDebInfo
[ -z "${INSTALL_ROOT}" ] && export INSTALL_ROOT=$1
export SYSTEMC_HOME=${INSTALL_ROOT}/systemc
export SCC_INSTALL=${INSTALL_ROOT}/scc
############################################################################################
#
############################################################################################
function build_boost {
	export BOOST_LIB_EXCLUDE=contract,fiber,graph,graph_parallel,iostreams,json,locale,log,math,mpi,nowide,python,random,stacktrace,test,timer,wave

	if [ ! -d boost_1_80_0 ]; then
	    [ -f boost_1_80_0.tar.bz2 ] || wget https://boostorg.jfrog.io/artifactory/main/release/1.80.0/source/boost_1_80_0.tar.bz2
	    tar xjf boost_1_80_0.tar.bz2
	fi
	(cd boost_1_80_0; \
	  ./bootstrap.sh --prefix=${SCC_INSTALL} --libdir=${SCC_INSTALL}/lib64 --without-libraries=${BOOST_LIB_EXCLUDE};\
	  ./b2 cxxflags="-std=c++${CXX_STD}" install)
}
############################################################################################
#
############################################################################################
function build_fmt {
	if [ ! -d fmt ]; then
	    if [ ! -f fmt_8.0.1.tar.gz ]; then
		git clone --depth 1 --branch 8.0.1 -c advice.detachedHead=false https://github.com/fmtlib/fmt.git
		tar czf fmt_8.0.1.tar.gz fmt --exclude=.git
	    else
		tar xzf fmt_8.0.1.tar.gz
	    fi
	fi
	cmake -S fmt -B build/fmt -DCMAKE_BUILD_TYPE=${RelWithDebInfo} -DCMAKE_INSTALL_PREFIX=${SCC_INSTALL} -DCMAKE_CXX_STANDARD=${CXX_STD} || exit 1
	cmake --build build/fmt -j 10 --target install || exit 2
}
############################################################################################
#
############################################################################################
function build_spdlog {
	if [ ! -d spdlog ]; then
	    if [ ! -f spdlog_1.9.2.tar.gz ]; then
		git clone --depth 1 --branch v1.9.2 -c advice.detachedHead=false https://github.com/gabime/spdlog.git
		tar czf spdlog_1.9.2.tar.gz spdlog --exclude=.git
	    else
		tar xzf spdlog_1.9.2.tar.gz
	    fi
	fi
	cmake -S spdlog -B build/spdlog -DCMAKE_BUILD_TYPE=${RelWithDebInfo} -DCMAKE_INSTALL_PREFIX=${SCC_INSTALL} -DCMAKE_CXX_STANDARD=${CXX_STD} || exit 1
	cmake --build build/spdlog -j 10 --target install || exit 2
}
############################################################################################
#
############################################################################################
function build_yamlcpp {
	if [ ! -d yaml-cpp ]; then
	    if [ ! -f yaml-cpp_0.6.3.tar.gz ]; then
		git clone --depth 1 --branch yaml-cpp-0.6.3 -c advice.detachedHead=false https://github.com/jbeder/yaml-cpp.git
		tar czf yaml-cpp_0.6.3.tar.gz yaml-cpp --exclude=.git
	    else
		tar xzf yaml-cpp_0.6.3.tar.gz
	    fi
	fi
	cmake -S yaml-cpp -B build/yaml-cpp -DCMAKE_BUILD_TYPE=${RelWithDebInfo} -DCMAKE_INSTALL_PREFIX=${SCC_INSTALL} -DCMAKE_CXX_STANDARD=${CXX_STD} \
		-DYAML_CPP_BUILD_TESTS=OFF -DYAML_CPP_BUILD_TOOLS=OFF -DLIB_SUFFIX=64 || exit 1
	cmake --build build/yaml-cpp -j 10 --target install || exit 2
	rm  -rf ~/.cmake/packages/yaml-cpp
}
############################################################################################
#
############################################################################################
function build_systemc {
	if [ ! -d systemc ]; then
	    if [ ! -f systemc_2.3.4.tar.gz ]; then
		git clone --depth 1 --branch 2.3.4 -c advice.detachedHead=false https://github.com/accellera-official/systemc.git
		tar czf systemc_2.3.4.tar.gz systemc --exclude=.git
	    else
		tar xzf systemc_2.3.4.tar.gz
	    fi
	fi
	cmake -S systemc -B build/systemc -DCMAKE_BUILD_TYPE=${RelWithDebInfo} \
		-DCMAKE_INSTALL_PREFIX=${SYSTEMC_HOME} -DCMAKE_CXX_STANDARD=${CXX_STD} || exit 1
	cmake --build build/systemc -j 10 --target install || exit 2
	rm  -rf ~/.cmake/packages/SystemC*
}
############################################################################################
#
############################################################################################
function build_scc {
	if [ ! -d scc ]; then
	    if [ ! -f scc.tar.gz ]; then
		git clone --recursive --branch develop -c advice.detachedHead=false https://github.com/Minres/SystemC-Components.git scc
		tar czf scc.tar.gz scc --exclude=.git
	    else
		tar xzf scc.tar.gz
	    fi
	fi
	cmake -S scc -B build/scc -Wno-dev \
		-DCMAKE_INSTALL_PREFIX=${SCC_INSTALL} -DCMAKE_CXX_STANDARD=${CXX_STD} -DENABLE_CONAN=OFF \
		-DBoost_NO_SYSTEM_PATHS=TRUE -DBOOST_ROOT=${SCC_INSTALL} -DBoost_NO_WARN_NEW_VERSIONS=ON || exit 1
	cmake --build build/scc -j 10 --target install || exit 2
}

build_boost
build_fmt
build_spdlog
build_yamlcpp
build_systemc
build_scc
