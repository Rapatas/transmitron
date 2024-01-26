from conans import ConanFile
from conan.tools.cmake import CMakeDeps, CMake, CMakeToolchain, cmake_layout

class TransmitronConan(ConanFile):

    name = "Transmitron"
    version = "0.0.4"
    settings = [
        "os",
        "compiler",
        "build_type",
        "arch"
    ]
    url = "https://github.com/Rapatas/transmitron"
    license = "GPL3"
    description = "MQTT client for desktop"

    options = {
        "shared": [True, False]
    }

    default_options = {
        "openssl:no_zlib": True,
        "shared": False,
    }

    def requirements(self):
        self.requires("paho-mqtt-cpp/1.2.0")
        self.requires("nlohmann_json/3.9.1")
        self.requires("tinyxml2/8.0.0")
        self.requires("fmt/8.0.1")
        self.requires("spdlog/1.9.2")
        self.requires("cli11/2.1.2")
        self.requires("date/3.0.1")

    def generate(self):
        cmake = CMakeDeps(self)
        cmake.generate()
        cmake = CMakeToolchain(self)
        cmake.generate()
