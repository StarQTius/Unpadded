import subprocess
from pathlib import Path
from tempfile import TemporaryDirectory

from setuptools import setup

WORKING_DIRECTORY = str(Path(__file__).parent.absolute())

with TemporaryDirectory() as tmp:
    cmake_install_directory = WORKING_DIRECTORY + "/py/unpadded"

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

    setup()
