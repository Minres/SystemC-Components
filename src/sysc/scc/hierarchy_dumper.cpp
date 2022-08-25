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
#include "configurer.h"
#include "tracer.h"
#include "perf_estimator.h"
#include <fstream>
#include "report.h"
#include <tlm>
#include <unordered_set>
#include <iostream>
#include <fstream>
#include <regex>
#include <sstream>
#include <rapidjson/rapidjson.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/prettywriter.h>
#include <fmt/format.h>

#include <string>
#include <typeinfo>
#ifdef __GNUG__
#include <cstdlib>
#include <memory>
#include <cxxabi.h>
#endif

namespace scc {
using namespace rapidjson;
using writer_type = PrettyWriter<OStreamWrapper>;

namespace {
#ifdef __GNUG__
std::string demangle(const char* name) {
    int status = -4; // some arbitrary value to eliminate the compiler warning
    // enable c++11 by passing the flag -std=c++11 to g++
    std::unique_ptr<char, void(*)(void*)> res {
        abi::__cxa_demangle(name, NULL, NULL, &status),
                std::free
    };
    return (status==0) ? res.get() : name ;
}
#else
// does nothing if not g++
std::string demangle(const char* name) {
    return name;
}
#endif
std::string demangle(const char* name);

template <class T>
std::string type(const T& t) {
    return demangle(typeid(t).name());
}

unsigned object_counter{0};

struct Port {
    std::string const fullname;
    std::string const name;
    sc_core::sc_interface const* port_if{nullptr};
    bool input{false};
    std::string const type;
    std::string const sig_name;
    std::string const id{fmt::format("{}", ++object_counter)};

    Port(std::string const& fullname, std::string const& name, sc_core::sc_interface  const* ptr, bool input, std::string const& type, std::string const& sig_name ="")
    : fullname(fullname)
    , name(name)
    , port_if(ptr)
    , input(input)
    , type(type)
    , sig_name(sig_name)
    { }
};

struct Module {
    std::string const fullname;
    std::string const name;
    std::string const type;
    bool const topModule{false};
    std::string const id{fmt::format("{}", ++object_counter)};
    std::vector<Module> submodules;
    std::vector<Port> ports;

    Module(std::string const& fullname, std::string const& name, std::string const& type, bool top)
    : fullname(fullname)
    , name(name)
    , type(type)
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
        if(std::string(obj->basename()).substr(0, 3) == "$$$")
            return {};
        auto* name = obj->name();
        currentModule->submodules.push_back(Module(name, obj->basename(), type(*obj), false));
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
        currentModule->submodules.push_back(Module(obj->name(), obj->basename(), type(*obj), false));
        currentModule->submodules.back().ports.push_back(
                Port(std::string(obj->name())+"."+obj->basename(), obj->basename(), iface, false, obj->kind(), obj->basename()));
#ifndef NO_TLM_EXTRACT
    } else if(auto const* tptr = dynamic_cast<tlm::tlm_base_socket_if const*>(obj)) {
        auto cat = tptr->get_socket_category();
        bool input = (cat & tlm::TLM_TARGET_SOCKET) == tlm::TLM_TARGET_SOCKET;
        currentModule->ports.push_back(Port(obj->name(), obj->basename(), input?GET_EXPORT_IF(tptr):GET_PORT_IF(tptr), input, obj->kind()));
        return {
            std::string(obj->basename())+"_port", std::string(obj->basename())+"_export",
            std::string(obj->basename())+"_port_0", std::string(obj->basename())+"_export_0"
        };
#endif
    } else if (auto const* optr = dynamic_cast<sc_core::sc_port_base const*>(obj)) {
        if(std::string(optr->basename()).substr(0, 3)!="$$$") {
            sc_core::sc_interface const* if_ptr = optr->get_interface();
            sc_core::sc_prim_channel const* if_obj = dynamic_cast<sc_core::sc_prim_channel const*>(if_ptr);
            bool is_input = kind == "sc_in" || kind == "sc_fifo_in";
            currentModule->ports.push_back(
                    Port(obj->name(), obj->basename(), if_ptr, is_input, obj->kind(), if_obj?if_obj->basename():""));
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
                        e << indent*level << "edge " << srcport.fullname << " -> " << tgtport.fullname << "\n";
                }
            }
    }
    // Draw edges submodule -> submodule:
    for (auto srcmod : module.submodules) {
        for (auto srcport : srcmod.ports) {
            if(!srcport.input && srcport.port_if)
                for (auto tgtmod : module.submodules) {
                    for (auto tgtport : tgtmod.ports) {
                        if(srcmod.fullname == tgtmod.fullname && tgtport.fullname == srcport.fullname)
                            continue;
                        if (tgtport.port_if == srcport.port_if && tgtport.input)
                            e << indent*level << "edge " << srcport.fullname << " -> " << tgtport.fullname << "\n";
                    }
                }
        }
    }
    level--;
    e << indent*level << "}\n" << "\n";
}

void generatePortJson(writer_type &writer, hierarchy_dumper::file_type type, const scc::Port &p) {
    writer.StartObject(); {
        writer.Key("id"); writer.String(p.id.c_str());
        if(type != hierarchy_dumper::D3JSON) {
            writer.Key("labels"); writer.StartArray(); {
                writer.StartObject(); {
                    writer.Key("text"); writer.String(p.name.c_str());
                } writer.EndObject();
            } writer.EndArray();
            writer.Key("width"); writer.Uint(6);
            writer.Key("height"); writer.Uint(6);
            writer.Key("layoutOptions"); writer.StartObject(); {
                writer.Key("port.side"); writer.String(p.input?"WEST":"EAST");
            } writer.EndObject();
            if(type == hierarchy_dumper::DBGJSON) {
                writer.Key("type"); writer.String(p.type.c_str());
                writer.Key("input"); writer.Bool(p.input);
                writer.Key("interface"); writer.Uint64(reinterpret_cast<uintptr_t>(p.port_if));
            }
        } else {
            writer.Key("direction"); writer.String(p.input?"INPUT":"OUTPUT");
            writer.Key("hwMeta"); writer.StartObject(); {
                writer.Key("name"); writer.String(p.name.c_str());
                // writer.Key("cssClass"); writer.String("node-style0");
                // writer.Key("cssStyle"); writer.String("fill:red");
                writer.Key("connectedAsParent"); writer.Bool(false);
            } writer.EndObject();
            writer.Key("properties"); writer.StartObject(); {
                writer.Key("side"); writer.String(p.input?"WEST":"EAST");
                // writer.Key("index"); writer.Int(0);
            } writer.EndObject();
            writer.Key("children"); writer.StartArray(); writer.EndArray();
        }
    } writer.EndObject();
}

void generateEdgeJson(writer_type &writer, const scc::Port &srcport, const scc::Port &tgtport) {
    writer.StartObject(); {
        auto edge_name = fmt::format("{}", ++object_counter);
        writer.Key("id"); writer.String(edge_name.c_str());
        writer.Key("sources"); writer.StartArray(); {
            writer.String(srcport.id.c_str());
        } writer.EndArray();
        writer.Key("targets"); writer.StartArray(); {
            writer.String(tgtport.id.c_str());
        }writer.EndArray();
    } writer.EndObject();
}

void generateEdgeD3Json(writer_type &writer, const scc::Module &srcmod, const scc::Port &srcport, const scc::Module &tgtmod, const scc::Port &tgtport) {
    writer.StartObject(); {
        auto edge_name = fmt::format("{}", ++object_counter);
        writer.Key("id"); writer.String(edge_name.c_str());
        writer.Key("source");writer.String(srcmod.id.c_str());
        writer.Key("sourcePort");writer.String(srcport.id.c_str());
        writer.Key("target");writer.String(tgtmod.id.c_str());
        writer.Key("targetPort");writer.String(tgtport.id.c_str());
        writer.Key("hwMeta"); writer.StartObject(); {
            if(srcport.sig_name.size()) {
                writer.Key("name"); writer.String(srcport.sig_name.c_str());
            } else if(tgtport.sig_name.size()) {
                writer.Key("name"); writer.String(tgtport.sig_name.c_str());
            } else {
                writer.Key("name"); writer.String(fmt::format("{}_to_{}", srcport.name, tgtport.name).c_str());
            }
            // writer.Key("cssClass"); writer.String("link-style0");
            // writer.Key("cssStyle"); writer.String("stroke:red");
        } writer.EndObject();
    } writer.EndObject();
}

void generateModJson(writer_type& writer, hierarchy_dumper::file_type type, Module const& module, unsigned level=0) {
    unsigned num_in{0}, num_out{0};
    for (auto port : module.ports) if(port.input) num_in++; else num_out++;
    writer.StartObject(); {
        writer.Key("id"); writer.String(module.id.c_str());
        // process ports
        writer.Key("ports"); writer.StartArray(); {
            for(auto& p: module.ports) generatePortJson(writer, type, p);
        } writer.EndArray();
        // process modules
        if(type==hierarchy_dumper::D3JSON && !module.topModule) writer.Key("_children"); else
        writer.Key("children"); writer.StartArray(); {
            for(auto& c: module.submodules) generateModJson(writer, type, c, level*1);
        } writer.EndArray();
        // process connections
        if(type==hierarchy_dumper::D3JSON && !module.topModule) writer.Key("_edges"); else
        writer.Key("edges"); writer.StartArray(); {
            // Draw edges module <-> submodule:
            for (auto srcport : module.ports) {
                if(srcport.port_if) {
                    for (auto tgtmod : module.submodules) {
                        for (auto tgtport : tgtmod.ports) {
                            if (tgtport.port_if == srcport.port_if)
                                if(type == hierarchy_dumper::D3JSON)
                                    generateEdgeD3Json(writer, module, srcport, tgtmod, tgtport);
                                else
                                    generateEdgeJson(writer, srcport, tgtport);
                        }
                    }
                }
            }
            // Draw edges submodule -> submodule:
            for (auto srcmod : module.submodules) {
                for (auto srcport : srcmod.ports) {
                    if(!srcport.input && srcport.port_if) {
                        for (auto tgtmod : module.submodules) {
                            for (auto tgtport : tgtmod.ports) {
                                if(srcmod.fullname == tgtmod.fullname && tgtport.fullname == srcport.fullname)
                                    continue;
                                if (tgtport.port_if == srcport.port_if && tgtport.input)
                                    if(type == hierarchy_dumper::D3JSON)
                                        generateEdgeD3Json(writer, srcmod, srcport, tgtmod, tgtport);
                                    else
                                        generateEdgeJson(writer, srcport, tgtport);
                            }
                        }
                    }
                }
            }
        } writer.EndArray();
        if(type != hierarchy_dumper::D3JSON) {
            writer.Key("labels"); writer.StartArray(); {
                writer.StartObject(); {
                    writer.Key("text"); writer.String(module.name.c_str());
                } writer.EndObject();
            } writer.EndArray();
            writer.Key("width"); writer.Uint(50);
            writer.Key("height"); writer.Uint(std::max(80U, std::max(num_in, num_out)*20));
            if(type == hierarchy_dumper::DBGJSON) {
                writer.Key("name"); writer.String(module.name.c_str());
                writer.Key("type"); writer.String(module.type.c_str());
                writer.Key("topmodule"); writer.Bool(module.topModule);
            }
        } else {
            writer.Key("hwMeta"); writer.StartObject(); {
                writer.Key("name"); writer.String(module.name.c_str());
                writer.Key("cls"); writer.String(module.type.c_str());
                // writer.Key("bodyText"); writer.String(module.type.c_str());
                writer.Key("maxId"); writer.Uint(object_counter);
                writer.Key("isExternalPort"); writer.Bool(false);
                // writer.Key("cssClass"); writer.String("node-style0");
                // writer.Key("cssStyle"); writer.String("fill:red");
            } writer.EndObject();
            writer.Key("properties"); writer.StartObject(); {
                writer.Key("org.eclipse.elk.layered.mergeEdges");  writer.Uint(1);
                writer.Key("org.eclipse.elk.portConstraints"); writer.String("FIXED_SIDE");
            } writer.EndObject();
        }
    } writer.EndObject();
}

void dump_structure(std::ostream& e, hierarchy_dumper::file_type format) {
    std::vector<Module> topModules;
    std::vector<sc_core::sc_object*> obja = sc_core::sc_get_top_level_objects();
    if(obja.size()==1 && std::string(obja[0]->kind()) == "sc_module" && std::string(obja[0]->basename()).substr(0, 3) != "$$$") {
        SCCDEBUG() << obja[0]->name() << "(" << obja[0]->kind() << ")";
        topModules.push_back(Module(obja[0]->name(), obja[0]->basename(), type(*obja[0]), true));
        for (auto* child : obja[0]->get_child_objects())
            scanModule(child, &topModules.back(), 1);
    } else if(obja.size()>1) {
        SCCDEBUG() << "sc_main ( function sc_main() )";
        topModules.push_back(Module("sc_main", "sc_main", "sc_main()", true));
        for (auto* child : obja)
            scanModule(child, &topModules.back(), 1);
    }
    if(format == hierarchy_dumper::ELKT) {
        e<<"algorithm: org.eclipse.elk.layered\n";
        e<<"edgeRouting: ORTHOGONAL\n";
        for (auto module : topModules)
            generateElk(e, module);
        SCCINFO() << "SystemC Structure Dumped to ELK file";
    } else {
        OStreamWrapper stream(e);
        writer_type writer(stream);
        writer.StartObject(); {
            auto elems = util::split(sc_core::sc_argv()[0], '/');
            writer.Key("id"); writer.String("0");
            writer.Key("labels"); writer.StartArray(); {
                writer.StartObject(); {
                    writer.Key("text"); writer.String(elems[elems.size()-1].c_str());
                } writer.EndObject();
            } writer.EndArray();
            writer.Key("layoutOptions"); writer.StartObject(); {
                writer.Key("algorithm"); writer.String("layered");
            } writer.EndObject();
            writer.Key("children"); writer.StartArray(); {
                for(auto& c:topModules) generateModJson(writer, format, c);
            } writer.EndArray();
            writer.Key("edges");writer.StartArray();
            writer.EndArray();
            if(format == hierarchy_dumper::D3JSON) {
                writer.Key("hwMeta"); writer.StartObject(); {
                    writer.Key("cls"); writer.Null();
                    writer.Key("maxId"); writer.Uint(65536);
                    writer.Key("name"); writer.String(elems[elems.size()-1].c_str());
                } writer.EndObject();
                writer.Key("properties"); writer.StartObject(); {
                    writer.Key("org.eclipse.elk.layered.mergeEdges");  writer.Uint(1);
                    writer.Key("org.eclipse.elk.portConstraints"); writer.String("FIXED_ORDER");
                } writer.EndObject();
            }
        } writer.EndObject();
        SCCINFO() << "SystemC Structure Dumped to JSON file";
    }
}
} // namespace anonymous

hierarchy_dumper::hierarchy_dumper(const std::string &filename, file_type format)
: sc_core::sc_module(sc_core::sc_module_name("$$$hierarchy_dumper$$$"))
, dump_hier_file_name{filename}
, dump_format{format}
{
}

hierarchy_dumper::~hierarchy_dumper() {
}

void hierarchy_dumper::start_of_simulation() {
    if(dump_hier_file_name.size()) {
        std::ofstream of{dump_hier_file_name};
        if(of.is_open())
            dump_structure(of, dump_format);
    }
}
}
