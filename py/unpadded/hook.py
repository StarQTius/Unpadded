import logging
import os
import sys
import traceback
from pathlib import Path

from cppimport.find import find_module_cpppath
from cppimport.importer import build_safely, load_module, setup_module_data
from cppimport.templating import setup_pybind11

INCLUDE_DIRECTORY = str(Path(__file__).parent.absolute()) + "/include"

_extra_include_dirs = []
_logger = logging.getLogger(__name__)


class Hook:
    def __init__(self):
        self._running = False

    def find_spec(self, fullname, path, target=None):
        if self._running:
            return

        try:
            self._running = True
            filepath = os.path.abspath(find_module_cpppath(fullname, opt_in=True))
            if fullname is None:
                fullname = os.path.splitext(os.path.basename(filepath))[0]
            module_data = setup_module_data(fullname, filepath)
            module_data["setup_unpadded"] = setup_unpadded
            if os.path.exists(module_data["ext_path"]):
                os.remove(module_data["ext_path"])
            build_safely(filepath, module_data)
            load_module(module_data)
            return module_data["module"]
        except ImportError:
            _logger.debug(traceback.format_exc())
        finally:
            self._running = False


def setup_unpadded(cfg):
    setup_pybind11(cfg)
    cfg["include_dirs"] += [INCLUDE_DIRECTORY] + _extra_include_dirs
    cfg["extra_compile_args"] += ["-std=c++17"]


def set_extra_include_dirs(include_dirs):
    global _extra_include_dirs
    _extra_include_dirs = include_dirs


sys.meta_path.insert(0, Hook())
