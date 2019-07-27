from conans import ConanFile, CMake, tools
import os
import shutil

class CrucianConan(ConanFile):
    name = "crucian"
    version = "0.01"
    license = "AGPL-3.0"
    url = "https://github.com/snakelizzard/crucian"
    description = "Simplified HTM implementation"
    settings = "os", "compiler", "build_type", "arch"
    generators = "cmake"

    def source(self):
        source_url = "https://github.com/snakelizzard/crucian/archive/v{0}.tar.gz".format(self.version)
        archive_name = "crucian-{0}".format(self.version)
        tools.get(source_url)
        os.rename(archive_name, "crucian")

        tools.replace_in_file("crucian/CMakeLists.txt", "project(crucian CXX)",
                              '''project(crucian CXX)
include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()''')

    def build(self):
        os.mkdir("build")
        shutil.move("conanbuildinfo.cmake", "build/")

        cmake = CMake(self)
        cmake.definitions["CRU_SHARED"] = False
        cmake.definitions["CRU_TEST"] = False
        cmake.configure(source_folder="crucian", build_folder="build")
        cmake.build()

    def package(self):
        self.copy("*", dst="include", src="crucian/include")
        self.copy("*", dst="lib", src="build/lib")

    def package_info(self):
        self.cpp_info.libs = ["crucian"]
