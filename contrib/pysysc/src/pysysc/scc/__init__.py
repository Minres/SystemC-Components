#
# Copyright (c) 2019 -2021 MINRES Technologies GmbH
#
# SPDX-License-Identifier: Apache-2.0
#

import os.path
import logging
import cppyy
from cppyy import gbl as cpp
import pysysc

logger = logging.getLogger(__name__)
traces = []
cfgs = []

def load_lib(project_dir):
    """
    Find and load SCC libs in PySysC.
    :param project_dir: Toplevel project directory and root of the search tree.
    """
    logging.debug("Loading SC-Components lib")
    pysysc.add_include_path(os.path.join(project_dir, 'scc/src/common'))
    pysysc.add_library('scc_util.h', 'libscc-util.so', project_dir)
    pysysc.add_include_path(os.path.join(project_dir, 'scc/third_party'))
    pysysc.add_include_path(os.path.join(project_dir, 'scc/third_party/scv-tr/src'))
    pysysc.add_library('scv-tr.h', 'libscv-tr.so', project_dir)
    pysysc.add_include_path(os.path.join(project_dir, 'scc/src/sysc'))
    pysysc.add_library('scc_sysc.h', 'libscc-sysc.so', project_dir)
    pysysc.add_include_path(os.path.join(project_dir, 'scc/src/components'))
    cppyy.include('scc_components.h')
    
    
def setup(log_level = logging.WARNING):
    try:
        if log_level >= logging.FATAL:
            cpp.scc.init_logging(cpp.logging.FATAL, False)
        elif log_level >= logging.ERROR:
            cpp.scc.init_logging(cpp.logging.ERROR, False)
        elif log_level >= logging.WARNING:
            cpp.scc.init_logging(cpp.logging.WARNING, False)
        elif log_level >= logging.INFO:
            cpp.scc.init_logging(cpp.logging.INFO, False)
        elif log_level >= logging.DEBUG:
            cpp.scc.init_logging(cpp.logging.DEBUG, False)
        else:
            cpp.scc.init_logging(cpp.logging.TRACE, False)
    except Exception: # fall back: use basic SystemC logging setup
        verb_lut={
            logging.FATAL:100, #SC_LOW
            logging.ERROR: 200, #SC_MEDIUM
            logging.WARNING: 300, #SC_HIGH
            logging.INFO: 400, #SC_FULL
            logging.DEBUG: 500 #SC_DEBUG
            }
        cpp.sc_core.sc_report_handler.set_verbosity_level(verb_lut[log_level]);
    cpp.sc_core.sc_report_handler.set_actions(cpp.sc_core.SC_ID_MORE_THAN_ONE_SIGNAL_DRIVER_, cpp.sc_core.SC_DO_NOTHING);

def configure(name="", enable_trace=False):
    if len(name) and os.path.isfile(name):
        cfgs.append(cpp.scc.configurer(cpp.std.string(name)));
        if enable_trace:
            trace_name = os.path.basename(name)
            trace = cpp.scc.configurable_tracer(trace_name, 1, True, True)
            trace.add_control()
            traces.append(trace)
    else:
        if enable_trace:
            trace = cpp.scc.tracer('vcd_trace', 1, True)
            traces.append(trace)

