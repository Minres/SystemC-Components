/*******************************************************************************
 * Copyright 2022 MINRES Technologies GmbH
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *******************************************************************************/

#include "hierarchy_dumper.h"
#include <fstream>
#include "report.h"
#include <tlm>
#include <unordered_set>
#include <iostream>
#include <fstream>
#include <regex>
#include <sstream>

namespace scc {
namespace {

struct Port {
    std::string id;
    std::string name;
    sc_core::sc_interface const* port_if{nullptr};
    bool input{false};
    std::string type;

    Port(std::string const& id, std::string const& name, sc_core::sc_interface  const* ptr, bool input, std::string const& type)
    : id(id)
    , name(name)
    , port_if(ptr)
    , input(input)
    , type(type)
    { }
};

struct Module {
    std::string id;
    std::string name;
    bool topModule{false};
    std::vector<Module> submodules;
    std::vector<Port> ports;

    Module(std::string id, std::string name, bool top)
    : id(id)
    , name(name)
    , topModule(top)
    { }

};

const std::unordered_set<std::string> know_entities = {
        "tlm_initiator_socket", "sc_export", "sc_thread_process", "sc_signal",
        "sc_object", "sc_fifo", "sc_method_process", "sc_mutex", "sc_vector",
        "sc_semaphore_ordered", "sc_variable", "sc_prim_channel", "tlm_signal"
};

std::string indent{"    "};
std::string operator* (std::string const& str, const unsigned int level) {
    std::ostringstream ss;
    for (unsigned int i = 0; i < level; i++) ss << str;
    return ss.str();
}

#if TLM_VERSION_MAJOR==2 and TLM_VERSION_MINOR == 0           ///< version minor level ( numeric )
#if TLM_VERSION_PATCH == 6           ///< version patch level ( numeric )
#define GET_EXPORT_IF(tptr) tptr->get_base_export().get_interface()
#define GET_PORT_IF(tptr)   tptr->get_base_port().get_interface()
#elif TLM_VERSION_PATCH == 5
#define GET_EXPORT_IF(tptr) tptr->get_export_base().get_interface()
#define GET_PORT_IF(tptr)   tptr->get_port_base().get_interface()
#else
#define NO_TLM_EXTRACT
#endif
#endif

std::vector<std::string> scanModule(sc_core::sc_object const* obj, Module *currentModule, unsigned const level) {
    SCCDEBUG() << indent*level<< obj->name() << "(" << obj->kind() << ")";
    std::string kind{obj->kind()};
    if (kind == "sc_module") {
        auto* name = obj->name();
        currentModule->submodules.push_back(Module(name, obj->basename(), false));
        std::unordered_set<std::string> keep_outs;
        for (auto* child : obj->get_child_objects()) {
            const std::string child_name{child->basename()};
            if(child_name.substr(0, 3)=="$$$")
                continue;
            if(!keep_outs.empty()) {
                auto it = std::find_if(std::begin(keep_outs), std::end(keep_outs), [&child_name](std::string const& e){
                    return child_name.size() > e.size() && child_name.substr(0, e.size()) == e;
                });
                if(it!=std::end(keep_outs))
                    continue;
            }
            auto ks = scanModule(child, &currentModule->submodules.back(), level+1);
            if(ks.size())
                for(auto& s:ks) keep_outs.insert(s);
        }
    } else if(kind == "sc_clock"){
        auto const* iface = dynamic_cast<sc_core::sc_interface const*>(obj);
        currentModule->submodules.push_back(Module(obj->name(), obj->basename(), false));
        currentModule->submodules.back().ports.push_back(Port(std::string(obj->name())+"."+obj->basename(), obj->basename(), iface, false, obj->kind()));
#ifndef NO_TLM_EXTRACT
    } else if(auto const* tptr = dynamic_cast<tlm::tlm_base_socket_if const*>(obj)) {
        auto cat = tptr->get_socket_category();
        bool input = (cat & tlm::TLM_TARGET_SOCKET) == tlm::TLM_TARGET_SOCKET;
        currentModule->ports.push_back(Port(obj->name(), obj->basename(), input?GET_EXPORT_IF(tptr):GET_PORT_IF(tptr), input, obj->kind()));
        return {std::string(obj->basename())+"_port", std::string(obj->basename())+"_export"};
#endif
    } else if (auto const* optr = dynamic_cast<sc_core::sc_port_base const*>(obj)) {
        if(std::string(optr->basename()).substr(0, 3)!="$$$") {
            sc_core::sc_interface const* pointer = optr->get_interface();
            bool input = kind == "sc_in" || kind == "sc_fifo_in";
            currentModule->ports.push_back(Port(obj->name(), obj->basename(), pointer, input, obj->kind()));
        }
    } else if (auto const* optr = dynamic_cast<sc_core::sc_export_base const*>(obj)) {
        if(std::string(optr->basename()).substr(0, 3)!="$$$") {
            sc_core::sc_interface const* pointer = optr->get_interface();
            currentModule->ports.push_back(Port(obj->name(), obj->basename(), pointer, true, obj->kind()));
        }
    } else if (know_entities.find(std::string(obj->kind())) == know_entities.end()) {
        SCCWARN() << "object not known (" << std::string(obj->kind()) << ")";
    }
    return {};
}

void generateElk(std::ostream& e, Module const& module, unsigned level=0) {
    SCCDEBUG() << module.name;
    unsigned num_in{0}, num_out{0};
    for (auto port : module.ports) if(port.input) num_in++; else num_out++;
    if(!module.ports.size() && !module.submodules.size()) return;
    e << indent*level << "node " << module.name << " {" << "\n";
    level++;
    e << indent*level << "layout [ size: 50, "<<std::max(80U, std::max(num_in, num_out)*20)<<" ]\n";
    e << indent*level << "portConstraints: FIXED_SIDE\n";
    e << indent*level << "label \"" << module.name << "\"\n";

    for (auto port : module.ports) {
        SCCDEBUG() << "    " << port.name << "\n";
        auto side = port.input?"WEST":"EAST";
        e << indent*level << "port " << port.name << " { ^port.side: "<<side<<" label '" << port.name<< "' }\n";
    }

    for (auto m : module.submodules)
        generateElk(e, m, level);
    // Draw edges module <-> submodule:
    for (auto srcport : module.ports) {
        if(srcport.port_if)
            for (auto tgtmod : module.submodules) {
                for (auto tgtport : tgtmod.ports) {
                    if (tgtport.port_if == srcport.port_if)
                        e << indent*level << "edge " << srcport.id << " -> " << tgtport.id << "\n";
                }
            }
    }
    // Draw edges submodule -> submodule:
    for (auto srcmod : module.submodules) {
        for (auto srcport : srcmod.ports) {
            if(!srcport.input && srcport.port_if)
                for (auto tgtmod : module.submodules) {
                    for (auto tgtport : tgtmod.ports) {
                        if(srcmod.id == tgtmod.id && tgtport.id == srcport.id)
                            continue;
                        if (tgtport.port_if == srcport.port_if && tgtport.input)
                            e << indent*level << "edge " << srcport.id << " -> " << tgtport.id << "\n";
                    }
                }
        }
    }
    level--;
    e << indent*level << "}\n" << "\n";
}
}

void dump_structure(std::ostream& e) {
    std::vector<Module> topModules;
    std::vector<sc_core::sc_object*> tops = sc_core::sc_get_top_level_objects();
    e<<"algorithm: org.eclipse.elk.layered\n";
    e<<"edgeRouting: ORTHOGONAL\n";
    for (auto t : tops) {
        if (std::string(t->kind()) == "sc_module") {
            SCCDEBUG() << t->name() << "(" << t->kind() << ")";
            topModules.push_back(Module(t->name(), t->basename(), true));
            for (auto* child : t->get_child_objects())
                scanModule(child, &topModules.back(), 1);
        }
    }
    for (auto module : topModules)
        generateElk(e, module);
    SCCINFO() << "SystemC Structure Dumped to ELK file";
}

hierarchy_dumper::hierarchy_dumper(const std::string &filename, unsigned format)
: sc_core::sc_module(sc_core::sc_module_name("hierarchy_dumper"))
, dump_hier_file_name{filename}
{
}

hierarchy_dumper::~hierarchy_dumper() {
}

void hierarchy_dumper::start_of_simulation() {
    if(dump_hier_file_name.size()) {
        std::ofstream of{dump_hier_file_name};
        if(of.is_open())
            dump_structure(of);
    }
}
}
