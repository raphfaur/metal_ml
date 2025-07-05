from setuptools import setup, Extension
setup(
    name="metal",
    version="1.0",
    description="This is a demo package",
    ext_modules=[
        Extension("metal", sources=["metalml.cpp"], include_dirs=['../metal-cpp'], extra_compile_args = ["-std=c++20"])],
    package_data={"": ["*.pyi"]},
    include_package_data=True
)