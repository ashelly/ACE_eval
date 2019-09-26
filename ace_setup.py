import setuptools
from distutils.core import setup
from Cython.Build import cythonize
from Cython.Compiler.Options import get_directive_defaults
import numpy
from Cython.Compiler import Options

Options.generate_cleanup_code=False

setup(ext_modules=cythonize("ace_functions.pyx", annotate=True),
      include_dirs=[numpy.get_include()], build_dir="build",
      script_args=['build'], options={'build': {'build_lib': '.'}, },
      )
