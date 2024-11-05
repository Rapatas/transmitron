from conan import ConanFile
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
        "shared": False,
        "openssl/*:no_zlib": True,
    }

    def requirements(self):
        self.requires("paho-mqtt-cpp/1.4.0")
        self.requires("nlohmann_json/3.11.3")
        self.requires("tinyxml2/10.0.0")
        self.requires("fmt/11.0.2", force=True)
        self.requires("spdlog/1.14.1")
        self.requires("cli11/2.4.2")
        self.requires("date/3.0.3")

    def generate(self):
        cmake = CMakeDeps(self)
        cmake.generate()
        cmake = CMakeToolchain(self)
        cmake.generate()
