SystemC-Components (SCC)
========================

SCC is supposed to be a light weight productivity library for SystemC and TLM 2.0 based modeling tasks using C++11.
* scv4tlm - Tracing TLM2 Sockets

  TLM2.0 compliant sockets which can be configured to trace transactions passing thru them using the SCV transaction recording facilities. The project is set-up to be used with Eclipse CDT and its build system

* sysc/scv_tr_db.h extended transaction recording databases

  scv_tr_sqlite is a SQLite based database back-end for the SystemC Verification library (SCV) transaction recording infrastructure while scv_tr_compressed is a text base database back-end with compression to reduce the file size

* sysc::sc_register

  a resource wrapper to access a storage location via a TLM 2.0 socket. This is realized using
  
* sysc::tlm_target

  a component distributing TLM2.0 accesses to target resources e.g. sysc::sc_register
  
* sysc::router

  a simple component to route TLM2.0 accesses of a set of masters to a set of targets based on generic payload addresses

build instructions
==================

the repo is cmake based. Make sure that you have at least cmake 3.12 installed.

There is a dependency on SystemC. For some capabilities SCV is required, but this
can be selected during build time. 

The suggested build flow is:

- create a build directory and enter into it
- execute cmake with applicable options 
- execute build
- install build
- run tests

For example:

git clone https://github.com/Minres/SystemC-Components.git
cd SystemC-Components/
git checkout adding-examples
git clone https://github.com/Arteris-IP/tlm2-interfaces.git
cd tlm2-interfaces/
git checkout adding-examples

setenv WORKAREA `pwd`
setenv SYSTEMC_HOME /workarea3/SystemC/systemc-2.3.3-gcc-6.3.0-c++14-install
setenv GCC_HOME     /workarea3/gcc-6.3.0-install
setenv SCV_HOME ${SYSTEMC_HOME}
setenv TLM_HOME ${SYSTEMC_HOME}/include
setenv PATH     ${GCC_HOME}/bin:${PATH}
setenv CC       ${GCC_HOME}/bin/gcc
setenv CXX      ${GCC_HOME}/bin/g++
setenv LD_LIBRARY_PATH ${GCC_HOME}/lib64
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=$WORKAREA/install/$LIB_VERSION -DENABLE_SCV=ON ..
make 
make install
make test
