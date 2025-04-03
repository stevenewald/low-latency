from conan import ConanFile


class Recipe(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeToolchain", "CMakeDeps", "VirtualRunEnv"

    def layout(self):
        self.folders.generators = "conan"

    def requirements(self):
        # Exchange
        self.requires("boost/1.87.0")
        self.requires("abseil/20240116.2")

    def configure(self):
        self.options["boost"].without_test=True
