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

def load_scc(project_dir, build_type):
    """
    Find and load SCC libs in PySysC.
    :param project_dir: Toplevel project directory.
    :param build_type: Current CMake build type e.g: Debug or Release.
    """
    logging.debug("Loading SC-Components lib")
    pysysc.add_include_path(os.path.join(project_dir, 'scc/src/common'))
    pysysc.add_library('scc_util.h', os.path.join(project_dir, 'build/%s/scc/src/common/libscc-util.so'%build_type))
    pysysc.add_include_path(os.path.join(project_dir, 'scc/third_party'))
    pysysc.add_include_path(os.path.join(project_dir, 'scc/third_party/scv-tr/src'))
    pysysc.add_library('scv-tr.h', os.path.join(project_dir, 'build/%s/scc/third_party/scv-tr/src/libscv-tr.so'%build_type))
    pysysc.add_include_path(os.path.join(project_dir, 'scc/src/sysc'))
    pysysc.add_library('scc_sysc.h', os.path.join(project_dir, 'build/%s/scc/src/sysc/libscc-sysc.so'%build_type))
    pysysc.add_include_path(os.path.join(project_dir, 'scc/src/components'))
    cppyy.include('scc_components.h')