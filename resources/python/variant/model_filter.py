from variant.model_dictionary import ModelDictionary

import collections

class ModelFilter(object):
    def __init__(self):
        self._parameters = dict()
        self._variables = dict()

    @property
    def parameters(self):
        """Return filter parameters."""
        return self._parameters

    @property
    def variables(self):
        """Return filter variables."""
        return self._variables

    def add_parameter_value(self, name, value):
        self._parameters[name] = value

    def add_parameter_range(self, name, start, stop):
        self._parameters[name] = [start, stop]

    def add_variable_value(self, name, value):
        self._variables[name] = value

    def add_variable_range(self, name, start, stop):
        self._variables[name] = [start, stop]

    def filter(self, model_dictionary):
        md = ModelDictionary(model_dictionary.model_class)
        for name, model in model_dictionary.dictionary.items():
            if self._test(model): md.add_model(model, name)

        return md

    def _test(self, model):
        for parameter, value in self._parameters.items():
            if not isinstance(value, collections.Iterable):
                if (model.get_parameter(parameter) != value):
                    return False
            else:
                if (model.get_parameter(parameter) < value[0] or
                    model.get_parameter(parameter) > value[1]):
                    return False

        for variable, value in self._variables.items():
            if not isinstance(value, collections.Iterable):
                if model.get_variable(variable) != value:
                    return False
            else:
                if (model.get_variable(variable) < value[0] or
                    model.get_variable(variable) > value[1]):
                    return False

        return True

if __name__ == '__main__':
    from variant.test_functions import quadratic_function

    md = ModelDictionary(quadratic_function.QuadraticFunction)
    for x in range(10):
        model = quadratic_function.QuadraticFunction()
        model.set_parameter('x', x)
        md.add_model(model)

    md.solve()

    mf = ModelFilter()
    #mf.add_parameter_value('x', 2)
    #mf.add_variable_value('F', 7)
    #mf.add_parameter_range('x', 3, 5)
    mf.add_variable_range('F', 1, 16)
    print(len(mf.filter(md).dictionary))