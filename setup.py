import runpy
import subprocess
from pathlib import Path
from tempfile import TemporaryDirectory

from pybind11.setup_helpers import Pybind11Extension
from setuptools import setup

WORKING_DIRECTORY = str(Path(__file__).parent.absolute())

with TemporaryDirectory() as tmp:
    cmake_install_directory = WORKING_DIRECTORY + "/py/unpadded"

    runpy.run_path("py/unpadded")

    subprocess.run(
        [
            "cmake",
            "-B",
            tmp,
            "-DCMAKE_BUILD_TYPE=Release",
            "-DCMAKE_INSTALL_PREFIX=" + cmake_install_directory,
            "-DUnpadded_INSTALL=ON",
        ],
        cwd=WORKING_DIRECTORY,
        check=True,
    )
    subprocess.run(
        ["cmake", "--build", tmp, "-t install"], cwd=WORKING_DIRECTORY, check=True
    )

    ext_modules = [
        Pybind11Extension(
            "unpadded._details",
            sources=["py/unpadded/_details.cpp"],
            include_dirs=[cmake_install_directory + "/include"],
        ),
    ]

    setup(ext_modules=ext_modules)
