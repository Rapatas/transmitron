from conans import ConanFile

class TransmitronConan(ConanFile):
    generators = [
        "cmake_find_package",
        "cmake_paths"
    ]
    requires = [
        "paho-mqtt-cpp/1.2.0",
        "nlohmann_json/3.9.1",
        "wxwidgets/3.1.5@bincrafters/stable",
        "tinyxml2/8.0.0",
        "fmt/8.0.1",
        "spdlog/1.9.2",
    ]
    default_options = {
        "wxwidgets:webview": False,
    }
