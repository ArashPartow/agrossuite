import agros2d
from test_suite.scenario import Agros2DTestCase
from test_suite.scenario import Agros2DTestResult

class TestBayesOptBooth(Agros2DTestCase):
    def setUp(self):  
        # problem
        problem = agros2d.problem(clear = True)
        problem.parameters["px"] = 0
        problem.parameters["py"] = 0
        
        # studies
        study_bayesopt = problem.add_study("bayesopt")
        study_bayesopt.add_parameter("px", -10, 10)
        study_bayesopt.add_parameter("py", -10, 10)
        study_bayesopt.add_functional("OF", "(px+2*py-7)**2+(2*px+py-5)**2", 100)
        
        study_bayesopt.clear_solution = True
        study_bayesopt.solve_problem = False
        
        study_bayesopt.settings["n_init_samples"] = 5
        study_bayesopt.settings["n_iterations"] = 20
        study_bayesopt.settings["n_iter_relearn"] = 5
        study_bayesopt.settings["init_method"] = "lhs"
        study_bayesopt.settings["surr_name"] = "sGaussianProcessML"
        study_bayesopt.settings["surr_noise"] = 1e-10
        study_bayesopt.settings["l_type"] = "emperical"
        study_bayesopt.settings["sc_type"] = "map"

        study_bayesopt.solve()
        
        self.computation = study_bayesopt.find_extreme("functional", "OF", True)
        
    def test_values(self):    
        self.value_test("px", self.computation.parameters["px"], 1.0)
        self.value_test("py", self.computation.parameters["py"], 3.0)
        self.lower_then_test("OF", self.computation.results["OF"], 2e-5)

class TestNLoptBooth(Agros2DTestCase):
    def setUp(self):  
        # problem
        problem = agros2d.problem(clear = True)
        problem.parameters["px"] = 0
        problem.parameters["py"] = 0
        
        # studies
        study_nlopt = problem.add_study("nlopt")
        study_nlopt.add_parameter("px", -10, 10)
        study_nlopt.add_parameter("py", -10, 10)
        study_nlopt.add_functional("OF", "(px+2*py-7)**2+(2*px+py-5)**2", 100)
        
        study_nlopt.clear_solution = True
        study_nlopt.solve_problem = False
     
        study_nlopt.settings["n_iterations"] = 30
        study_nlopt.settings["xtol_rel"] = 1e-06
        study_nlopt.settings["xtol_abs"] = 1e-12
        study_nlopt.settings["ftol_rel"] = 1e-06
        study_nlopt.settings["ftol_abs"] = 1e-12
        study_nlopt.settings["algorithm"] = "ln_bobyqa"
     
        study_nlopt.solve()
        
        self.computation = study_nlopt.find_extreme("functional", "OF", True)
        
    def test_values(self):    
        self.value_test("px", self.computation.parameters["px"], 1.0)
        self.value_test("py", self.computation.parameters["py"], 3.0)
        self.lower_then_test("OF", self.computation.results["OF"], 1e-10)

class TestNSGA2Sphere(Agros2DTestCase):
    def setUp(self):  
        # problem
        problem = agros2d.problem(clear = True)
        problem.parameters["px"] = 0
        problem.parameters["py"] = 0
        
        # studies
        study_nsga2 = problem.add_study("nsga2")
        study_nsga2.add_parameter("px", -10, 10)
        study_nsga2.add_parameter("py", -10, 10)
        study_nsga2.add_functional("OF", "px**2+py**2", 100)

        study_nsga2.clear_solution = True
        study_nsga2.solve_problem = False
        
        study_nsga2.settings["popsize"] = 12
        study_nsga2.settings["ngen"] = 100
        study_nsga2.settings["pcross"] = 0.6
        study_nsga2.settings["pmut"] = 0.2
        study_nsga2.settings["eta_c"] = 10
        study_nsga2.settings["eta_m"] = 20
        study_nsga2.settings["crowdobj"] = False

        study_nsga2.solve()
        
        self.computation = study_nsga2.find_extreme("functional", "OF", True)
       
    def test_values(self):    
        self.lower_then_test("px", abs(self.computation.parameters["px"]), 1e-1)
        self.lower_then_test("py", abs(self.computation.parameters["py"]), 1e-1)
        self.lower_then_test("OF", self.computation.results["OF"], 1e-2)
        
                                                 
class TestNSGA3Sphere(Agros2DTestCase):
    def setUp(self):  
        # problem
        problem = agros2d.problem(clear = True)
        problem.parameters["px"] = 0
        problem.parameters["py"] = 0
        
        # studies
        study_nsga3 = problem.add_study("nsga3")
        study_nsga3.add_parameter("px", -10, 10)
        study_nsga3.add_parameter("py", -10, 10)
        study_nsga3.add_functional("OF", "px**2+py**2", 100)

        study_nsga3.clear_solution = True
        study_nsga3.solve_problem = False
        
        study_nsga3.settings["popsize"] = 12
        study_nsga3.settings["ngen"] = 100
        study_nsga3.settings["pcross"] = 0.6
        study_nsga3.settings["eta_c"] = 10
        study_nsga3.settings["eta_m"] = 20

        study_nsga3.solve()
        
        self.computation = study_nsga3.find_extreme("functional", "OF", True)
       
    def test_values(self):    
        self.lower_then_test("px", abs(self.computation.parameters["px"]), 1e-1)
        self.lower_then_test("py", abs(self.computation.parameters["py"]), 1e-1)
        self.lower_then_test("OF", self.computation.results["OF"], 1e-2)
                                                 
if __name__ == '__main__':        
    import unittest as ut
    
    suite = ut.TestSuite()
    result = Agros2DTestResult()
    suite.addTest(ut.TestLoader().loadTestsFromTestCase(TestBayesOptBooth))
    suite.addTest(ut.TestLoader().loadTestsFromTestCase(TestNLoptBooth))
    suite.addTest(ut.TestLoader().loadTestsFromTestCase(TestNSGA2Sphere))
    suite.addTest(ut.TestLoader().loadTestsFromTestCase(TestNSGA3Sphere))
    suite.run(result)