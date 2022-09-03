import os
from pathlib import Path
from shutil import which
from sysconfig import get_config_var

from .client import *
from .hook import *

if not ("CC" in os.environ and "CXX" in os.environ):
    os.environ["CC"] = get_config_var("CC")
    os.environ["CXX"] = get_config_var("CXX")

if which("ccache") is not None:
    os.environ["CC"] = "ccache " + os.environ["CC"]
else:
    raise EnvironmentError("`Unpadded` requires `ccache` which is not installed.")

""" _details come last in order to be compiled """

from ._details import *
