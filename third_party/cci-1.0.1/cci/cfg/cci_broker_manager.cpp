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

#include "cci/cfg/cci_broker_manager.h"
#include "cci/cfg/cci_broker_if.h"
#include "cci/cfg/cci_originator.h"
#include "cci/cfg/cci_report_handler.h"

CCI_OPEN_NAMESPACE_

std::map<const sc_core::sc_object*, cci_broker_if*> cci_broker_manager::m_brokers;

cci_broker_handle
cci_broker_manager::get_broker(const cci_originator &originator)
{
  const sc_core::sc_object *org=originator.get_object();
  while (true) {
    std::map<const sc_core::sc_object*, cci_broker_if*>::iterator it =
            m_brokers.find(org);
    if(it != m_brokers.end()) {
      cci_broker_handle broker_handle=(it->second)->create_broker_handle(originator);
      return broker_handle;
    }
    if (!org) {
      CCI_REPORT_ERROR("cci_broker_manager","No global broker found!");
      // Abort since no reasonable recovery when the exception is suppressed.
      cci_abort();
    }
    org = org->get_parent_object();
  }
}

cci_broker_handle
cci_broker_manager::register_broker(cci_broker_if& broker)
{
    const sc_core::sc_object *obj=sc_core::sc_get_current_object();

    cci_broker_if* & broker_entry = m_brokers[obj]; // find broker position in registry
    if( !broker_entry ) {
        broker_entry = &broker; // store broker in registry
    }
    if( &broker != broker_entry ) { // we already had a different entry!
        CCI_REPORT_ERROR("cci_broker_manager/register_broker",
                         "A broker is already registered in the current"
                                 " hierarchy.");
        /* If this error is supproessed, our recovery is to pass back a handle to the existing broker */
    }
    if (obj) return broker_entry->create_broker_handle(cci_originator());
    else return  broker_entry->create_broker_handle(cci_originator(CCI_UNKNOWN_ORIGINATOR_STRING_));
}


cci_broker_handle cci_get_broker() {
  if (!sc_core::sc_get_current_object()) {
    CCI_REPORT_ERROR("cci_get_broker","You may not request a broker from outside the SystemC hierarchy (use cci::cci_get_global_broker)");
    /* get broker may well work, so we'll not abort */
  }
  return cci_broker_manager::get_broker(cci_originator());
}
cci_broker_handle cci_register_broker(cci_broker_if& broker) 
{
  return cci_broker_manager::register_broker(broker);
}
cci_broker_handle cci_register_broker(cci_broker_if* broker) 
{
  return cci_broker_manager::register_broker(broker);
}
cci_broker_handle cci_get_global_broker(const cci_originator &originator)
{
  if (sc_core::sc_get_current_object() || originator.get_object()) {
    CCI_REPORT_ERROR("cci_get_global_broker","You may not request the global broker from within the SystemC hierarchy (use cci::cci_get_broker)");
  }
  return cci_broker_manager::get_broker(originator).create_broker_handle(originator);
}

CCI_CLOSE_NAMESPACE_
