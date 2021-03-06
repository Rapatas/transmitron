from conans import ConanFile

class SaladConan(ConanFile):
    generators = [
        "cmake_find_package",
        "cmake_paths"
    ]
    requires = [
        "paho-mqtt-cpp/1.2.0",
        "nlohmann_json/3.9.1",
        "wxwidgets/3.1.4@bincrafters/stable",
        "tinyxml2/8.0.0",
        "cppcodec/0.2",
        "fmt/7.1.3"
    ]
    default_options = {
        "wxwidgets:webview": False
    }
