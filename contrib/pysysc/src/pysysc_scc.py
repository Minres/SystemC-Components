#
# Copyright (c) 2019 -2021 MINRES Technolgies GmbH
#
# SPDX-License-Identifier: Apache-2.0
#

import os.path
import logging
import cppyy
import pysysc

logger = logging.getLogger(__name__)

def load_scc(project_dir):
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