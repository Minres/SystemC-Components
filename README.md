SystemC-Components (SCC)
========================

SCC is supposed to be a light weight productivity library for SystemC and TLM 2.0 based modeling tasks using C++11.

Here is a short list of features.

* Extended logging and log configuration implementation

  This is built on top of the SystemC report implementation and allows use of iostream based logging as well as instance based log level connfiguration
  
* Config file reader and configuration handler

  The configurer allows to read a JSON file and apply the values to cci_param as well as to sc_attributes. This can be used for instance-based logging as well as instance-based trace configuration
  
* Automatic tracer

  The tracer(s) allow to automatically discover signals and sc_variables (see below) and register them with the trace file. If the configurer is being used the tracing can be controlled on an per-instance base 
  
* Various optimized trace file implementations   
  * compressed VCD
  * FST (used by [GTKWave](http://gtkwave.sourceforge.net/))

* Tracing TLM2 Sockets

  TLM2.0 compliant sockets which can be configured to trace transactions passing thru them using the SCV transaction recording facilities. The project is set-up to be used with Eclipse CDT and its build system

* Stripped down version of SCV

  To reduce the dependency SCC comes with a stripped down version of Accelleras SystemC Verification (SCV) library. This library does not support introspection and randomization anymore, its primary purpose is to enable transaction recording. Those traces can be visualized using [SCViewer](https://minres.github.io/SCViewer/).

* Extended and optimized transaction recording database(s)

  Aside of the SCV text file format SCC comes with other file format writer. scv_tr_sqlite is a SQLite based database back-end for the SystemC Verification library (SCV) transaction recording infrastructure while scv_tr_compressed is a text base database back-end with compression to reduce the file size. These format are also supporte by the [SCViewer](https://minres.github.io/SCViewer/).

* sysc::sc_variable

  A plain C/C++ variable wrapper to access a storage location via the SystemC object tree. It allows also to register value change observers to react on changes.
  
* sysc::sc_register

  A resource wrapper to access a storage location via a TLM 2.0 socket. It allows to register read and write callback to implement register functionality upon reading/writing the register.
  
* sysc::tlm_target

  A component distributing TLM2.0 accesses to target resources e.g. sysc::sc_register
  
* sysc::router

  A simple component to route TLM2.0 accesses of a set of masters to a set of targets based on generic payload addresses
  
* various TLM2.0 AT and pin-level adapters for common bus protocols like
  * APB
  * AHB
  * AXI/ACE
  * OBI

The full documentation can be found at the [Github pages](https://minres.github.io/SystemC-Components/)

Build instructions
==================

The repo is cmake based and uses conan. Make sure that you have at least cmake 3.16 and conan installed.

The suggested build steps are:

- create a build directory and enter into it
- execute cmake with applicable options 
- execute build
- install build
- run tests

For example:

```

    mkdir build
    cd build
    cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=<some install path> ..
    make 
    make install
    make test
    ./examples/ace-axi/ace_axi_example
    ./examples/axi-axi/axi_axi_example

```
