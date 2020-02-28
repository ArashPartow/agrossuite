from . import _agros

import os 
# set datadir
_agros._set_datadir(os.path.dirname(os.path.realpath(__file__)))
# read plugins
_agros._read_plugins()

# set properties
cachedir = _agros.cachedir
computation = _agros.computation
datadir = _agros.datadir
get_script_from_model = _agros.get_script_from_model
open_file = _agros.open_file
options = _agros.options
positive_value = _agros.positive_value
problem = _agros.problem
save_file = _agros.save_file
tempdir = _agros.tempdir
value_in_list = _agros.value_in_list
value_in_range = _agros.value_in_range
version = _agros.version
