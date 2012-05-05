import agros2d

# model
problem = agros2d.problem(clear = True)
problem.coordinate_type = "axisymmetric"
problem.mesh_type = "triangle"
problem.matrix_solver = "umfpack"

# fields
current = agros2d.field("current")
current.analysis_type = "steadystate"
current.number_of_refinements = 1
current.polynomial_order = 4
current.linearity_type = "linear"
current.weak_forms = "compiled"

current.add_boundary("Neumann", "current_inward_current_flow", {"current_inward_current_flow" : 0})
current.add_boundary("Ground", "current_potential", {"current_potential" : 0})
current.add_boundary("Voltage", "current_potential", {"current_potential" : 10})
current.add_boundary("Inlet", "current_inward_current_flow", {"current_inward_current_flow" : -3e9})

current.add_material("Copper", {"current_conductivity" : 5.7e7})

# geometry
geometry = agros2d.geometry

# edges
geometry.add_edge(0, 0.45, 0, 0, boundaries = {"current" : "Neumann"})
geometry.add_edge(0, 0, 0.2, 0, boundaries = {"current" : "Ground"})
geometry.add_edge(0.2, 0, 0.2, 0.15, boundaries = {"current" : "Inlet"})
geometry.add_edge(0.2, 0.15, 0.35, 0.45, boundaries = {"current" : "Neumann"})
geometry.add_edge(0.35, 0.45, 0, 0.45, boundaries = {"current" : "Voltage"})

# labels
geometry.add_label(0.0933957, 0.350253, materials = {"current" : "Copper"})


geometry.zoom_best_fit()

# solve problem
problem.solve()

# point value
point = current.local_values(0.213175, 0.25045)
testV = agros2d.test("Scalar potential", point["V"], 5.566438)
testE = agros2d.test("Electric field", point["E"], 32.059116)
testEr = agros2d.test("Electric field - r", point["Er"], -11.088553)
testEz = agros2d.test("Electric field - z", point["Ez"], -30.080408)
testJ = agros2d.test("Current density", point["J"], 1.82737e9)
testJr = agros2d.test("Current density - r", point["Jr"], -6.320475e8)
testJz = agros2d.test("Current density - z", point["Jz"], -1.714583e9)
testpj = agros2d.test("Losses", point["pj"], 5.858385e10)	

# volume integral
volume = current.volume_integrals([0])
testPj = agros2d.test("Losses", volume["Pj"], 4.542019e9)

# surface integral
surface = current.surface_integrals([1])
testI = agros2d.test("Current", surface["I"], -2.166256e8)

print("Test: Current field - axisymmetric: " + str(testV and testE and testEr and testEz and testJ and testJr and testJz and testpj and testI))