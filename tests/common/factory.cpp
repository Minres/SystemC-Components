#include "factory.h"

#include <stdexcept>

auto factory::get_instance() -> factory& {
    static factory instance{};
    return instance;
}

factory::factory()
: m_constructors{}
, m_objects{} {}

void factory::create() {
    for(const auto& item : m_constructors) {
        m_objects[item.first] = item.second();
    }
}

void factory::destroy() { m_objects.clear(); }

void factory::add_object(const std::string& name, constructor create) {
    auto it = m_constructors.find(name);

    if(it == m_constructors.cend()) {
        m_constructors[name] = create;
    } else {
        throw std::runtime_error("factory::add(): " + name + " object already exist in factory");
    }
}

auto factory::get_object(const std::string& name) -> void* {
    auto it = m_objects.find(name);

    if(it == m_objects.cend()) {
        throw std::runtime_error("factory::get(): " + name + " object doesn't exist in factory");
    }

    return it->second.get();
}
