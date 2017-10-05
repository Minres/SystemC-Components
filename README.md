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
