/*****************************************************************************

  Licensed to Accellera Systems Initiative Inc. (Accellera) under one or
  more contributor license agreements.  See the NOTICE file distributed
  with this work for additional information regarding copyright ownership.
  Accellera licenses this file to you under the Apache License, Version 2.0
  (the "License"); you may not use this file except in compliance with the
  License.  You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
  implied.  See the License for the specific language governing
  permissions and limitations under the License.

 ****************************************************************************/

#include "cci/cfg/cci_report_handler.h"
#include "cci/cfg/cci_config_macros.h"

#include <cstring> // std::strncmp

CCI_OPEN_NAMESPACE_

void
cci_report_handler::report( sc_core::sc_severity severity, const char* msg_type
                          , const char* msg, const char* file, int line)
{
  std::string cci_msg_type = CCI_SC_REPORT_MSG_TYPE_PREFIX_;
  cci_msg_type.append(msg_type);
  sc_core::sc_report_handler::report(severity,cci_msg_type.c_str(),msg,file,line);
}

//functions that throw a report for each cci_param_failure type
void
cci_report_handler::set_param_failed(const char* msg, const char* file, int line)
{
  report(sc_core::SC_ERROR,"SET_PARAM_FAILED",msg,file,line);
}

void
cci_report_handler::get_param_failed(const char* msg, const char* file, int line)
{
  report(sc_core::SC_ERROR,"GET_PARAM_FAILED",msg,file,line);
}

void
cci_report_handler::add_param_failed(const char* msg, const char* file, int line)
{
  report(sc_core::SC_ERROR,"ADD_PARAM_FAILED",msg,file,line);
}

void
cci_report_handler::remove_param_failed(const char* msg, const char* file, int line)
{
  report(sc_core::SC_ERROR,"REMOVE_PARAM_FAILED",msg,file,line);
}

void
cci_report_handler::cci_value_failure(const char* msg, const char* file, int line)
{
  report(sc_core::SC_ERROR,"CCI_VALUE_FAILURE",msg,file,line);
}

/* ------------------------------------------------------------------------ */

// function to return cci_param_failure that matches thrown (or cached) report
cci_param_failure
cci_report_handler::decode_param_failure(const sc_core::sc_report& rpt)
{
  using namespace std;
  static const char   cci_msg_type_prefix[] = CCI_SC_REPORT_MSG_TYPE_PREFIX_;
  static const size_t cci_msg_type_prefix_len = strlen(cci_msg_type_prefix);

  const char* rpt_msg_type = rpt.get_msg_type();

  if( strncmp(cci_msg_type_prefix, rpt_msg_type, cci_msg_type_prefix_len) == 0 )
  {
    static struct cci_msg_type
    {
      const char*       str;
      cci_param_failure val;
    }
    const cci_msg_types[] =
    {
      { "SET_PARAM_FAILED",    CCI_SET_PARAM_FAILURE    },
      { "GET_PARAM_FAILED",    CCI_GET_PARAM_FAILURE    },
      { "ADD_PARAM_FAILED",    CCI_ADD_PARAM_FAILURE    },
      { "REMOVE_PARAM_FAILED", CCI_REMOVE_PARAM_FAILURE },
      { "CCI_VALUE_FAILURE",   CCI_VALUE_FAILURE        },
      { NULL,                  CCI_UNDEFINED_FAILURE    }
    };

    const cci_msg_type* cci_type = cci_msg_types;
    const char*         msg_type = rpt_msg_type + cci_msg_type_prefix_len;

    for( ; cci_type->str != NULL; ++cci_type ) {
      if (strcmp(cci_type->str, msg_type) == 0)
        break;
    }
    return cci_type->val;
  }

  //not a CCI failure report
  return CCI_NOT_FAILURE;
}

/* ------------------------------------------------------------------------ */

cci_param_failure cci_handle_exception(cci_param_failure expect)
{
  try
  {
    /*re-*/throw; // current exception
  }
  catch(const sc_core::sc_report& rpt)
  {
    cci_param_failure err = cci_report_handler::decode_param_failure(rpt);

    if (err == CCI_NOT_FAILURE)
      /*re-*/throw; // unmatched sc_report

    if (err != expect && expect != CCI_ANY_FAILURE)
      /*re-*/throw; // unmatched CCI failure

    // OK, found expected error - return error code
    return err;
  }
  // unmatched exceptions will continue to propagate here
}

/* ------------------------------------------------------------------------ */

void cci_abort()
{
# if CCI_SYSTEMC_VERSION_CODE_ >= CCI_VERSION_HELPER_(2,3,2)
    sc_core::sc_abort();
# else
    std::terminate();
# endif
}

CCI_CLOSE_NAMESPACE_
