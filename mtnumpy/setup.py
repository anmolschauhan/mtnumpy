from distutils.core import setup, Extension, DEBUG


extra_compile_args = ["-DNDEBUG", "-O3"]
sfc_module = Extension('mtnumpy', sources = ['mtnumpy.cpp', 'mtmath.cpp'],
                       extra_compile_args=extra_compile_args)

setup(name = 'mtnumpy', version = '1.0',
    description = 'Python Package with superfastcode C++ extension',
    ext_modules = [sfc_module]
    )
