from conan import ConanFile
from conan.tools.cmake import cmake_layout
from conan.tools.cmake.toolchain.toolchain import CMakeToolchain
import os


class Pkg(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = (
        "CMakeDeps"
    )
    default_options = {
        "systemc/*:shared": "True",
        "boost/*:fPIC": "True",
        "boost/*:header_only": "False",
        "boost/*:without_contract": "True",
        "boost/*:without_fiber": "True",
        "boost/*:without_graph": "True",
        "boost/*:without_graph_parallel": "True",
        "boost/*:without_iostreams": "True",
        "boost/*:without_json": "True",
        "boost/*:without_locale": "True",
        "boost/*:without_log": "True",
        "boost/*:without_math": "True",
        "boost/*:without_mpi": "True",
        "boost/*:without_nowide": "True",
        "boost/*:without_python": "True",
        "boost/*:without_random": "True",
        "boost/*:without_regex": "True",
        "boost/*:without_stacktrace": "False",
        "boost/*:without_test": "True",
        "boost/*:without_timer": "True",
        "boost/*:without_type_erasure": "True",
        "boost/*:without_wave": "True",
    }

    def requirements(self):
        if self.info.settings.compiler.cppstd and self.info.settings.compiler.cppstd<17:
            self.requires("systemc/2.3.4")
        else:
            self.requires("systemc/3.0.1")
        self.requires("fmt/8.0.1")
        self.requires("spdlog/1.9.2")
        self.requires("boost/1.85.0")
        self.requires("catch2/3.1.0")
        self.requires("lz4/1.9.4")
        self.requires("yaml-cpp/0.7.0")
        self.requires("jsoncpp/1.9.5")
        self.requires("zlib/1.3.1")

    def build_requirements(self):
        pass

    def layout(self):
        cmake_layout(self)
    
    def generate(self):
        tc = CMakeToolchain(self)
        tc.user_presets_path = False
        tc.generate()
        