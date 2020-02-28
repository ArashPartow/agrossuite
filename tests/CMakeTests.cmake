IF(TEST_fields)
    add_test(NAME tests.fields.acoustic.TestAcousticHarmonicAxisymmetric WORKING_DIRECTORY . COMMAND python3 -m unittest tests.fields.acoustic.TestAcousticHarmonicAxisymmetric)
    add_test(NAME tests.fields.acoustic.TestAcousticHarmonicPlanar WORKING_DIRECTORY . COMMAND python3 -m unittest tests.fields.acoustic.TestAcousticHarmonicPlanar)
    add_test(NAME tests.fields.acoustic.TestAcousticTransientAxisymmetric WORKING_DIRECTORY . COMMAND python3 -m unittest tests.fields.acoustic.TestAcousticTransientAxisymmetric)
    add_test(NAME tests.fields.acoustic.TestAcousticTransientPlanar WORKING_DIRECTORY . COMMAND python3 -m unittest tests.fields.acoustic.TestAcousticTransientPlanar)
    add_test(NAME tests.fields.current.TestCurrentAxisymmetric WORKING_DIRECTORY . COMMAND python3 -m unittest tests.fields.current.TestCurrentAxisymmetric)
    add_test(NAME tests.fields.current.TestCurrentPlanar WORKING_DIRECTORY . COMMAND python3 -m unittest tests.fields.current.TestCurrentPlanar)
    add_test(NAME tests.fields.elasticity.TestElasticityAxisymmetric WORKING_DIRECTORY . COMMAND python3 -m unittest tests.fields.elasticity.TestElasticityAxisymmetric)
    add_test(NAME tests.fields.elasticity.TestElasticityPlanar WORKING_DIRECTORY . COMMAND python3 -m unittest tests.fields.elasticity.TestElasticityPlanar)
    add_test(NAME tests.fields.electrostatic.TestElectrostaticAxisymmetric WORKING_DIRECTORY . COMMAND python3 -m unittest tests.fields.electrostatic.TestElectrostaticAxisymmetric)
    add_test(NAME tests.fields.electrostatic.TestElectrostaticPlanar WORKING_DIRECTORY . COMMAND python3 -m unittest tests.fields.electrostatic.TestElectrostaticPlanar)
    add_test(NAME tests.fields.flow.TestFlowAxisymmetric WORKING_DIRECTORY . COMMAND python3 -m unittest tests.fields.flow.TestFlowAxisymmetric)
    add_test(NAME tests.fields.flow.TestFlowPlanar WORKING_DIRECTORY . COMMAND python3 -m unittest tests.fields.flow.TestFlowPlanar)
    add_test(NAME tests.fields.heat.BenchmarkHeatTransientAxisymmetric WORKING_DIRECTORY . COMMAND python3 -m unittest tests.fields.heat.BenchmarkHeatTransientAxisymmetric)
    add_test(NAME tests.fields.heat.TestHeatAxisymmetric WORKING_DIRECTORY . COMMAND python3 -m unittest tests.fields.heat.TestHeatAxisymmetric)
    add_test(NAME tests.fields.heat.TestHeatNonlinPlanarNewton WORKING_DIRECTORY . COMMAND python3 -m unittest tests.fields.heat.TestHeatNonlinPlanarNewton)
    add_test(NAME tests.fields.heat.TestHeatNonlinPlanarPicard WORKING_DIRECTORY . COMMAND python3 -m unittest tests.fields.heat.TestHeatNonlinPlanarPicard)
    add_test(NAME tests.fields.heat.TestHeatPlanar WORKING_DIRECTORY . COMMAND python3 -m unittest tests.fields.heat.TestHeatPlanar)
    add_test(NAME tests.fields.heat.TestHeatTransientAxisymmetric WORKING_DIRECTORY . COMMAND python3 -m unittest tests.fields.heat.TestHeatTransientAxisymmetric)
    add_test(NAME tests.fields.magnetic_harmonic.TestMagneticHarmonicAxisymmetric WORKING_DIRECTORY . COMMAND python3 -m unittest tests.fields.magnetic_harmonic.TestMagneticHarmonicAxisymmetric)
    add_test(NAME tests.fields.magnetic_harmonic.TestMagneticHarmonicAxisymmetricTotalCurrent WORKING_DIRECTORY . COMMAND python3 -m unittest tests.fields.magnetic_harmonic.TestMagneticHarmonicAxisymmetricTotalCurrent)
    add_test(NAME tests.fields.magnetic_harmonic.TestMagneticHarmonicNonlinAxisymmetric WORKING_DIRECTORY . COMMAND python3 -m unittest tests.fields.magnetic_harmonic.TestMagneticHarmonicNonlinAxisymmetric)
    add_test(NAME tests.fields.magnetic_harmonic.TestMagneticHarmonicNonlinPlanar WORKING_DIRECTORY . COMMAND python3 -m unittest tests.fields.magnetic_harmonic.TestMagneticHarmonicNonlinPlanar)
    add_test(NAME tests.fields.magnetic_harmonic.TestMagneticHarmonicPlanar WORKING_DIRECTORY . COMMAND python3 -m unittest tests.fields.magnetic_harmonic.TestMagneticHarmonicPlanar)
    add_test(NAME tests.fields.magnetic_harmonic.TestMagneticHarmonicPlanarTotalCurrent WORKING_DIRECTORY . COMMAND python3 -m unittest tests.fields.magnetic_harmonic.TestMagneticHarmonicPlanarTotalCurrent)
    add_test(NAME tests.fields.magnetic_steady.TestMagneticAxisymmetric WORKING_DIRECTORY . COMMAND python3 -m unittest tests.fields.magnetic_steady.TestMagneticAxisymmetric)
    add_test(NAME tests.fields.magnetic_steady.TestMagneticNonlinAxisymmetric WORKING_DIRECTORY . COMMAND python3 -m unittest tests.fields.magnetic_steady.TestMagneticNonlinAxisymmetric)
    add_test(NAME tests.fields.magnetic_steady.TestMagneticNonlinPlanar WORKING_DIRECTORY . COMMAND python3 -m unittest tests.fields.magnetic_steady.TestMagneticNonlinPlanar)
    add_test(NAME tests.fields.magnetic_steady.TestMagneticPlanar WORKING_DIRECTORY . COMMAND python3 -m unittest tests.fields.magnetic_steady.TestMagneticPlanar)
    add_test(NAME tests.fields.magnetic_transient.TestMagneticTransientAxisymmetric WORKING_DIRECTORY . COMMAND python3 -m unittest tests.fields.magnetic_transient.TestMagneticTransientAxisymmetric)
    add_test(NAME tests.fields.magnetic_transient.TestMagneticTransientPlanar WORKING_DIRECTORY . COMMAND python3 -m unittest tests.fields.magnetic_transient.TestMagneticTransientPlanar)
    add_test(NAME tests.fields.math_coeff.TestMathCoeffAxisymmetric WORKING_DIRECTORY . COMMAND python3 -m unittest tests.fields.math_coeff.TestMathCoeffAxisymmetric)
    add_test(NAME tests.fields.math_coeff.TestMathCoeffPlanar WORKING_DIRECTORY . COMMAND python3 -m unittest tests.fields.math_coeff.TestMathCoeffPlanar)
    add_test(NAME tests.fields.math_coeff.TestMathCoeffTransientAxisymmetric WORKING_DIRECTORY . COMMAND python3 -m unittest tests.fields.math_coeff.TestMathCoeffTransientAxisymmetric)
    add_test(NAME tests.fields.math_coeff.TestMathCoeffTransientPlanar WORKING_DIRECTORY . COMMAND python3 -m unittest tests.fields.math_coeff.TestMathCoeffTransientPlanar)
    add_test(NAME tests.fields.rf_te.TestRFTEHarmonicAxisymmetric WORKING_DIRECTORY . COMMAND python3 -m unittest tests.fields.rf_te.TestRFTEHarmonicAxisymmetric)
    add_test(NAME tests.fields.rf_te.TestRFTEHarmonicPlanar WORKING_DIRECTORY . COMMAND python3 -m unittest tests.fields.rf_te.TestRFTEHarmonicPlanar)
    add_test(NAME tests.fields.rf_tm.TestRFTMHarmonicAxisymmetric WORKING_DIRECTORY . COMMAND python3 -m unittest tests.fields.rf_tm.TestRFTMHarmonicAxisymmetric)
    add_test(NAME tests.fields.rf_tm.TestRFTMHarmonicPlanar WORKING_DIRECTORY . COMMAND python3 -m unittest tests.fields.rf_tm.TestRFTMHarmonicPlanar)
ENDIF(TEST_fields)

IF(TEST_coupled)
    add_test(NAME tests.coupled_problems.basic_coupled_problems.TestCoupledProblemsBasic1 WORKING_DIRECTORY . COMMAND python3 -m unittest tests.coupled_problems.basic_coupled_problems.TestCoupledProblemsBasic1)
    add_test(NAME tests.coupled_problems.basic_coupled_problems.TestCoupledProblemsBasic2 WORKING_DIRECTORY . COMMAND python3 -m unittest tests.coupled_problems.basic_coupled_problems.TestCoupledProblemsBasic2)
    add_test(NAME tests.coupled_problems.basic_coupled_problems.TestCoupledProblemsBasic3 WORKING_DIRECTORY . COMMAND python3 -m unittest tests.coupled_problems.basic_coupled_problems.TestCoupledProblemsBasic3)
    add_test(NAME tests.coupled_problems.basic_coupled_problems.TestCoupledProblemsBasic4 WORKING_DIRECTORY . COMMAND python3 -m unittest tests.coupled_problems.basic_coupled_problems.TestCoupledProblemsBasic4)
    add_test(NAME tests.coupled_problems.unrealistic_coupled_problems.TestCoupledProblemsManyDomains WORKING_DIRECTORY . COMMAND python3 -m unittest tests.coupled_problems.unrealistic_coupled_problems.TestCoupledProblemsManyDomains)
ENDIF(TEST_coupled)

IF(TEST_nonlin)
    add_test(NAME tests.fields.heat.TestHeatNonlinPlanarNewton WORKING_DIRECTORY . COMMAND python3 -m unittest tests.fields.heat.TestHeatNonlinPlanarNewton)
    add_test(NAME tests.fields.heat.TestHeatNonlinPlanarPicard WORKING_DIRECTORY . COMMAND python3 -m unittest tests.fields.heat.TestHeatNonlinPlanarPicard)
    add_test(NAME tests.fields.magnetic_steady.TestMagneticNonlinPlanar WORKING_DIRECTORY . COMMAND python3 -m unittest tests.fields.magnetic_steady.TestMagneticNonlinPlanar)
    add_test(NAME tests.fields.magnetic_harmonic.TestMagneticHarmonicNonlinPlanar WORKING_DIRECTORY . COMMAND python3 -m unittest tests.fields.magnetic_harmonic.TestMagneticHarmonicNonlinPlanar)
    add_test(NAME tests.fields.magnetic_harmonic.TestMagneticHarmonicNonlinAxisymmetric WORKING_DIRECTORY . COMMAND python3 -m unittest tests.fields.magnetic_harmonic.TestMagneticHarmonicNonlinAxisymmetric)
    add_test(NAME tests.fields.flow.TestFlowPlanar WORKING_DIRECTORY . COMMAND python3 -m unittest tests.fields.flow.TestFlowPlanar)
    add_test(NAME tests.fields.flow.TestFlowAxisymmetric WORKING_DIRECTORY . COMMAND python3 -m unittest tests.fields.flow.TestFlowAxisymmetric)
ENDIF(TEST_nonlin)

IF(TEST_adaptivity)
    add_test(NAME tests.adaptivity.adaptivity.TestAdaptivityAcoustic WORKING_DIRECTORY . COMMAND python3 -m unittest tests.adaptivity.adaptivity.TestAdaptivityAcoustic)
    add_test(NAME tests.adaptivity.adaptivity.TestAdaptivityElasticityBracket WORKING_DIRECTORY . COMMAND python3 -m unittest tests.adaptivity.adaptivity.TestAdaptivityElasticityBracket)
    add_test(NAME tests.adaptivity.adaptivity.TestAdaptivityElectrostatic WORKING_DIRECTORY . COMMAND python3 -m unittest tests.adaptivity.adaptivity.TestAdaptivityElectrostatic)
    add_test(NAME tests.adaptivity.adaptivity.TestAdaptivityHLenses WORKING_DIRECTORY . COMMAND python3 -m unittest tests.adaptivity.adaptivity.TestAdaptivityHLenses)
    add_test(NAME tests.adaptivity.adaptivity.TestAdaptivityMagneticProfileConductor WORKING_DIRECTORY . COMMAND python3 -m unittest tests.adaptivity.adaptivity.TestAdaptivityMagneticProfileConductor)
    add_test(NAME tests.adaptivity.adaptivity.TestAdaptivityPAndHCoupled WORKING_DIRECTORY . COMMAND python3 -m unittest tests.adaptivity.adaptivity.TestAdaptivityPAndHCoupled)
    add_test(NAME tests.adaptivity.adaptivity.TestAdaptivityRF_TE WORKING_DIRECTORY . COMMAND python3 -m unittest tests.adaptivity.adaptivity.TestAdaptivityRF_TE)
ENDIF(TEST_adaptivity)

IF(TEST_tracing)
    add_test(NAME tests.particle_tracing.particle_tracing.TestParticleTracingAxisymmetric WORKING_DIRECTORY . COMMAND python3 -m unittest tests.particle_tracing.particle_tracing.TestParticleTracingAxisymmetric)
    add_test(NAME tests.particle_tracing.particle_tracing.TestParticleTracingPlanar WORKING_DIRECTORY . COMMAND python3 -m unittest tests.particle_tracing.particle_tracing.TestParticleTracingPlanar)
ENDIF(TEST_tracing)

IF(TEST_script)
    add_test(NAME tests.script.benchmark.BenchmarkGeometryTransformation WORKING_DIRECTORY . COMMAND python3 -m unittest tests.script.benchmark.BenchmarkGeometryTransformation)
    add_test(NAME tests.script.field.TestField WORKING_DIRECTORY . COMMAND python3 -m unittest tests.script.field.TestField)
    add_test(NAME tests.script.field.TestFieldAdaptivity WORKING_DIRECTORY . COMMAND python3 -m unittest tests.script.field.TestFieldAdaptivity)
    add_test(NAME tests.script.field.TestFieldAdaptivityInfo WORKING_DIRECTORY . COMMAND python3 -m unittest tests.script.field.TestFieldAdaptivityInfo)
    add_test(NAME tests.script.field.TestFieldBoundaries WORKING_DIRECTORY . COMMAND python3 -m unittest tests.script.field.TestFieldBoundaries)
    add_test(NAME tests.script.field.TestFieldIntegrals WORKING_DIRECTORY . COMMAND python3 -m unittest tests.script.field.TestFieldIntegrals)
    add_test(NAME tests.script.field.TestFieldLocalValues WORKING_DIRECTORY . COMMAND python3 -m unittest tests.script.field.TestFieldLocalValues)
    add_test(NAME tests.script.field.TestFieldMaterials WORKING_DIRECTORY . COMMAND python3 -m unittest tests.script.field.TestFieldMaterials)
    add_test(NAME tests.script.field.TestFieldMatrixSolver WORKING_DIRECTORY . COMMAND python3 -m unittest tests.script.field.TestFieldMatrixSolver)
    add_test(NAME tests.script.field.TestFieldNewtonSolver WORKING_DIRECTORY . COMMAND python3 -m unittest tests.script.field.TestFieldNewtonSolver)
    add_test(NAME tests.script.field.TestFieldSolverInfo WORKING_DIRECTORY . COMMAND python3 -m unittest tests.script.field.TestFieldSolverInfo)
    add_test(NAME tests.script.geometry.TestGeometry WORKING_DIRECTORY . COMMAND python3 -m unittest tests.script.geometry.TestGeometry)
    add_test(NAME tests.script.geometry.TestGeometryTransformations WORKING_DIRECTORY . COMMAND python3 -m unittest tests.script.geometry.TestGeometryTransformations)
    add_test(NAME tests.script.problem.TestProblem WORKING_DIRECTORY . COMMAND python3 -m unittest tests.script.problem.TestProblem)
    add_test(NAME tests.script.problem.TestProblemSolution WORKING_DIRECTORY . COMMAND python3 -m unittest tests.script.problem.TestProblemSolution)
    add_test(NAME tests.script.problem.TestProblemTime WORKING_DIRECTORY . COMMAND python3 -m unittest tests.script.problem.TestProblemTime)
ENDIF(TEST_script)

IF(TEST_core)
    add_test(NAME numpy.testing._private.nosetester.NoseTester WORKING_DIRECTORY . COMMAND python3 -m unittest numpy.testing._private.nosetester.NoseTester)
    add_test(NAME tests.core.matrix_solvers.TestMatrixSolversExternal WORKING_DIRECTORY . COMMAND python3 -m unittest tests.core.matrix_solvers.TestMatrixSolversExternal)
    add_test(NAME tests.core.matrix_solvers.TestMatrixSolversGeneral WORKING_DIRECTORY . COMMAND python3 -m unittest tests.core.matrix_solvers.TestMatrixSolversGeneral)
    add_test(NAME tests.core.matrix_solvers.TestMatrixSolversInternal WORKING_DIRECTORY . COMMAND python3 -m unittest tests.core.matrix_solvers.TestMatrixSolversInternal)
    add_test(NAME tests.core.mesh_generator.TestMeshGenerator WORKING_DIRECTORY . COMMAND python3 -m unittest tests.core.mesh_generator.TestMeshGenerator)
ENDIF(TEST_core)

IF(TEST_complete)
    add_test(NAME tests.fields.acoustic.TestAcousticHarmonicAxisymmetric WORKING_DIRECTORY . COMMAND python3 -m unittest tests.fields.acoustic.TestAcousticHarmonicAxisymmetric)
    add_test(NAME tests.fields.acoustic.TestAcousticHarmonicPlanar WORKING_DIRECTORY . COMMAND python3 -m unittest tests.fields.acoustic.TestAcousticHarmonicPlanar)
    add_test(NAME tests.fields.acoustic.TestAcousticTransientAxisymmetric WORKING_DIRECTORY . COMMAND python3 -m unittest tests.fields.acoustic.TestAcousticTransientAxisymmetric)
    add_test(NAME tests.fields.acoustic.TestAcousticTransientPlanar WORKING_DIRECTORY . COMMAND python3 -m unittest tests.fields.acoustic.TestAcousticTransientPlanar)
    add_test(NAME tests.fields.current.TestCurrentAxisymmetric WORKING_DIRECTORY . COMMAND python3 -m unittest tests.fields.current.TestCurrentAxisymmetric)
    add_test(NAME tests.fields.current.TestCurrentPlanar WORKING_DIRECTORY . COMMAND python3 -m unittest tests.fields.current.TestCurrentPlanar)
    add_test(NAME tests.fields.elasticity.TestElasticityAxisymmetric WORKING_DIRECTORY . COMMAND python3 -m unittest tests.fields.elasticity.TestElasticityAxisymmetric)
    add_test(NAME tests.fields.elasticity.TestElasticityPlanar WORKING_DIRECTORY . COMMAND python3 -m unittest tests.fields.elasticity.TestElasticityPlanar)
    add_test(NAME tests.fields.electrostatic.TestElectrostaticAxisymmetric WORKING_DIRECTORY . COMMAND python3 -m unittest tests.fields.electrostatic.TestElectrostaticAxisymmetric)
    add_test(NAME tests.fields.electrostatic.TestElectrostaticPlanar WORKING_DIRECTORY . COMMAND python3 -m unittest tests.fields.electrostatic.TestElectrostaticPlanar)
    add_test(NAME tests.fields.flow.TestFlowAxisymmetric WORKING_DIRECTORY . COMMAND python3 -m unittest tests.fields.flow.TestFlowAxisymmetric)
    add_test(NAME tests.fields.flow.TestFlowPlanar WORKING_DIRECTORY . COMMAND python3 -m unittest tests.fields.flow.TestFlowPlanar)
    add_test(NAME tests.fields.heat.BenchmarkHeatTransientAxisymmetric WORKING_DIRECTORY . COMMAND python3 -m unittest tests.fields.heat.BenchmarkHeatTransientAxisymmetric)
    add_test(NAME tests.fields.heat.TestHeatAxisymmetric WORKING_DIRECTORY . COMMAND python3 -m unittest tests.fields.heat.TestHeatAxisymmetric)
    add_test(NAME tests.fields.heat.TestHeatNonlinPlanarNewton WORKING_DIRECTORY . COMMAND python3 -m unittest tests.fields.heat.TestHeatNonlinPlanarNewton)
    add_test(NAME tests.fields.heat.TestHeatNonlinPlanarPicard WORKING_DIRECTORY . COMMAND python3 -m unittest tests.fields.heat.TestHeatNonlinPlanarPicard)
    add_test(NAME tests.fields.heat.TestHeatPlanar WORKING_DIRECTORY . COMMAND python3 -m unittest tests.fields.heat.TestHeatPlanar)
    add_test(NAME tests.fields.heat.TestHeatTransientAxisymmetric WORKING_DIRECTORY . COMMAND python3 -m unittest tests.fields.heat.TestHeatTransientAxisymmetric)
    add_test(NAME tests.fields.magnetic_harmonic.TestMagneticHarmonicAxisymmetric WORKING_DIRECTORY . COMMAND python3 -m unittest tests.fields.magnetic_harmonic.TestMagneticHarmonicAxisymmetric)
    add_test(NAME tests.fields.magnetic_harmonic.TestMagneticHarmonicAxisymmetricTotalCurrent WORKING_DIRECTORY . COMMAND python3 -m unittest tests.fields.magnetic_harmonic.TestMagneticHarmonicAxisymmetricTotalCurrent)
    add_test(NAME tests.fields.magnetic_harmonic.TestMagneticHarmonicNonlinAxisymmetric WORKING_DIRECTORY . COMMAND python3 -m unittest tests.fields.magnetic_harmonic.TestMagneticHarmonicNonlinAxisymmetric)
    add_test(NAME tests.fields.magnetic_harmonic.TestMagneticHarmonicNonlinPlanar WORKING_DIRECTORY . COMMAND python3 -m unittest tests.fields.magnetic_harmonic.TestMagneticHarmonicNonlinPlanar)
    add_test(NAME tests.fields.magnetic_harmonic.TestMagneticHarmonicPlanar WORKING_DIRECTORY . COMMAND python3 -m unittest tests.fields.magnetic_harmonic.TestMagneticHarmonicPlanar)
    add_test(NAME tests.fields.magnetic_harmonic.TestMagneticHarmonicPlanarTotalCurrent WORKING_DIRECTORY . COMMAND python3 -m unittest tests.fields.magnetic_harmonic.TestMagneticHarmonicPlanarTotalCurrent)
    add_test(NAME tests.fields.magnetic_steady.TestMagneticAxisymmetric WORKING_DIRECTORY . COMMAND python3 -m unittest tests.fields.magnetic_steady.TestMagneticAxisymmetric)
    add_test(NAME tests.fields.magnetic_steady.TestMagneticNonlinAxisymmetric WORKING_DIRECTORY . COMMAND python3 -m unittest tests.fields.magnetic_steady.TestMagneticNonlinAxisymmetric)
    add_test(NAME tests.fields.magnetic_steady.TestMagneticNonlinPlanar WORKING_DIRECTORY . COMMAND python3 -m unittest tests.fields.magnetic_steady.TestMagneticNonlinPlanar)
    add_test(NAME tests.fields.magnetic_steady.TestMagneticPlanar WORKING_DIRECTORY . COMMAND python3 -m unittest tests.fields.magnetic_steady.TestMagneticPlanar)
    add_test(NAME tests.fields.magnetic_transient.TestMagneticTransientAxisymmetric WORKING_DIRECTORY . COMMAND python3 -m unittest tests.fields.magnetic_transient.TestMagneticTransientAxisymmetric)
    add_test(NAME tests.fields.magnetic_transient.TestMagneticTransientPlanar WORKING_DIRECTORY . COMMAND python3 -m unittest tests.fields.magnetic_transient.TestMagneticTransientPlanar)
    add_test(NAME tests.fields.math_coeff.TestMathCoeffAxisymmetric WORKING_DIRECTORY . COMMAND python3 -m unittest tests.fields.math_coeff.TestMathCoeffAxisymmetric)
    add_test(NAME tests.fields.math_coeff.TestMathCoeffPlanar WORKING_DIRECTORY . COMMAND python3 -m unittest tests.fields.math_coeff.TestMathCoeffPlanar)
    add_test(NAME tests.fields.math_coeff.TestMathCoeffTransientAxisymmetric WORKING_DIRECTORY . COMMAND python3 -m unittest tests.fields.math_coeff.TestMathCoeffTransientAxisymmetric)
    add_test(NAME tests.fields.math_coeff.TestMathCoeffTransientPlanar WORKING_DIRECTORY . COMMAND python3 -m unittest tests.fields.math_coeff.TestMathCoeffTransientPlanar)
    add_test(NAME tests.fields.rf_te.TestRFTEHarmonicAxisymmetric WORKING_DIRECTORY . COMMAND python3 -m unittest tests.fields.rf_te.TestRFTEHarmonicAxisymmetric)
    add_test(NAME tests.fields.rf_te.TestRFTEHarmonicPlanar WORKING_DIRECTORY . COMMAND python3 -m unittest tests.fields.rf_te.TestRFTEHarmonicPlanar)
    add_test(NAME tests.fields.rf_tm.TestRFTMHarmonicAxisymmetric WORKING_DIRECTORY . COMMAND python3 -m unittest tests.fields.rf_tm.TestRFTMHarmonicAxisymmetric)
    add_test(NAME tests.fields.rf_tm.TestRFTMHarmonicPlanar WORKING_DIRECTORY . COMMAND python3 -m unittest tests.fields.rf_tm.TestRFTMHarmonicPlanar)
    add_test(NAME tests.coupled_problems.basic_coupled_problems.TestCoupledProblemsBasic1 WORKING_DIRECTORY . COMMAND python3 -m unittest tests.coupled_problems.basic_coupled_problems.TestCoupledProblemsBasic1)
    add_test(NAME tests.coupled_problems.basic_coupled_problems.TestCoupledProblemsBasic2 WORKING_DIRECTORY . COMMAND python3 -m unittest tests.coupled_problems.basic_coupled_problems.TestCoupledProblemsBasic2)
    add_test(NAME tests.coupled_problems.basic_coupled_problems.TestCoupledProblemsBasic3 WORKING_DIRECTORY . COMMAND python3 -m unittest tests.coupled_problems.basic_coupled_problems.TestCoupledProblemsBasic3)
    add_test(NAME tests.coupled_problems.basic_coupled_problems.TestCoupledProblemsBasic4 WORKING_DIRECTORY . COMMAND python3 -m unittest tests.coupled_problems.basic_coupled_problems.TestCoupledProblemsBasic4)
    add_test(NAME tests.coupled_problems.unrealistic_coupled_problems.TestCoupledProblemsManyDomains WORKING_DIRECTORY . COMMAND python3 -m unittest tests.coupled_problems.unrealistic_coupled_problems.TestCoupledProblemsManyDomains)
    add_test(NAME tests.fields.heat.TestHeatNonlinPlanarNewton WORKING_DIRECTORY . COMMAND python3 -m unittest tests.fields.heat.TestHeatNonlinPlanarNewton)
    add_test(NAME tests.fields.heat.TestHeatNonlinPlanarPicard WORKING_DIRECTORY . COMMAND python3 -m unittest tests.fields.heat.TestHeatNonlinPlanarPicard)
    add_test(NAME tests.fields.magnetic_steady.TestMagneticNonlinPlanar WORKING_DIRECTORY . COMMAND python3 -m unittest tests.fields.magnetic_steady.TestMagneticNonlinPlanar)
    add_test(NAME tests.fields.magnetic_harmonic.TestMagneticHarmonicNonlinPlanar WORKING_DIRECTORY . COMMAND python3 -m unittest tests.fields.magnetic_harmonic.TestMagneticHarmonicNonlinPlanar)
    add_test(NAME tests.fields.magnetic_harmonic.TestMagneticHarmonicNonlinAxisymmetric WORKING_DIRECTORY . COMMAND python3 -m unittest tests.fields.magnetic_harmonic.TestMagneticHarmonicNonlinAxisymmetric)
    add_test(NAME tests.fields.flow.TestFlowPlanar WORKING_DIRECTORY . COMMAND python3 -m unittest tests.fields.flow.TestFlowPlanar)
    add_test(NAME tests.fields.flow.TestFlowAxisymmetric WORKING_DIRECTORY . COMMAND python3 -m unittest tests.fields.flow.TestFlowAxisymmetric)
    add_test(NAME tests.adaptivity.adaptivity.TestAdaptivityAcoustic WORKING_DIRECTORY . COMMAND python3 -m unittest tests.adaptivity.adaptivity.TestAdaptivityAcoustic)
    add_test(NAME tests.adaptivity.adaptivity.TestAdaptivityElasticityBracket WORKING_DIRECTORY . COMMAND python3 -m unittest tests.adaptivity.adaptivity.TestAdaptivityElasticityBracket)
    add_test(NAME tests.adaptivity.adaptivity.TestAdaptivityElectrostatic WORKING_DIRECTORY . COMMAND python3 -m unittest tests.adaptivity.adaptivity.TestAdaptivityElectrostatic)
    add_test(NAME tests.adaptivity.adaptivity.TestAdaptivityHLenses WORKING_DIRECTORY . COMMAND python3 -m unittest tests.adaptivity.adaptivity.TestAdaptivityHLenses)
    add_test(NAME tests.adaptivity.adaptivity.TestAdaptivityMagneticProfileConductor WORKING_DIRECTORY . COMMAND python3 -m unittest tests.adaptivity.adaptivity.TestAdaptivityMagneticProfileConductor)
    add_test(NAME tests.adaptivity.adaptivity.TestAdaptivityPAndHCoupled WORKING_DIRECTORY . COMMAND python3 -m unittest tests.adaptivity.adaptivity.TestAdaptivityPAndHCoupled)
    add_test(NAME tests.adaptivity.adaptivity.TestAdaptivityRF_TE WORKING_DIRECTORY . COMMAND python3 -m unittest tests.adaptivity.adaptivity.TestAdaptivityRF_TE)
    add_test(NAME tests.particle_tracing.particle_tracing.TestParticleTracingAxisymmetric WORKING_DIRECTORY . COMMAND python3 -m unittest tests.particle_tracing.particle_tracing.TestParticleTracingAxisymmetric)
    add_test(NAME tests.particle_tracing.particle_tracing.TestParticleTracingPlanar WORKING_DIRECTORY . COMMAND python3 -m unittest tests.particle_tracing.particle_tracing.TestParticleTracingPlanar)
    add_test(NAME tests.script.benchmark.BenchmarkGeometryTransformation WORKING_DIRECTORY . COMMAND python3 -m unittest tests.script.benchmark.BenchmarkGeometryTransformation)
    add_test(NAME tests.script.field.TestField WORKING_DIRECTORY . COMMAND python3 -m unittest tests.script.field.TestField)
    add_test(NAME tests.script.field.TestFieldAdaptivity WORKING_DIRECTORY . COMMAND python3 -m unittest tests.script.field.TestFieldAdaptivity)
    add_test(NAME tests.script.field.TestFieldAdaptivityInfo WORKING_DIRECTORY . COMMAND python3 -m unittest tests.script.field.TestFieldAdaptivityInfo)
    add_test(NAME tests.script.field.TestFieldBoundaries WORKING_DIRECTORY . COMMAND python3 -m unittest tests.script.field.TestFieldBoundaries)
    add_test(NAME tests.script.field.TestFieldIntegrals WORKING_DIRECTORY . COMMAND python3 -m unittest tests.script.field.TestFieldIntegrals)
    add_test(NAME tests.script.field.TestFieldLocalValues WORKING_DIRECTORY . COMMAND python3 -m unittest tests.script.field.TestFieldLocalValues)
    add_test(NAME tests.script.field.TestFieldMaterials WORKING_DIRECTORY . COMMAND python3 -m unittest tests.script.field.TestFieldMaterials)
    add_test(NAME tests.script.field.TestFieldMatrixSolver WORKING_DIRECTORY . COMMAND python3 -m unittest tests.script.field.TestFieldMatrixSolver)
    add_test(NAME tests.script.field.TestFieldNewtonSolver WORKING_DIRECTORY . COMMAND python3 -m unittest tests.script.field.TestFieldNewtonSolver)
    add_test(NAME tests.script.field.TestFieldSolverInfo WORKING_DIRECTORY . COMMAND python3 -m unittest tests.script.field.TestFieldSolverInfo)
    add_test(NAME tests.script.geometry.TestGeometry WORKING_DIRECTORY . COMMAND python3 -m unittest tests.script.geometry.TestGeometry)
    add_test(NAME tests.script.geometry.TestGeometryTransformations WORKING_DIRECTORY . COMMAND python3 -m unittest tests.script.geometry.TestGeometryTransformations)
    add_test(NAME tests.script.problem.TestProblem WORKING_DIRECTORY . COMMAND python3 -m unittest tests.script.problem.TestProblem)
    add_test(NAME tests.script.problem.TestProblemSolution WORKING_DIRECTORY . COMMAND python3 -m unittest tests.script.problem.TestProblemSolution)
    add_test(NAME tests.script.problem.TestProblemTime WORKING_DIRECTORY . COMMAND python3 -m unittest tests.script.problem.TestProblemTime)
    add_test(NAME numpy.testing._private.nosetester.NoseTester WORKING_DIRECTORY . COMMAND python3 -m unittest numpy.testing._private.nosetester.NoseTester)
    add_test(NAME tests.core.matrix_solvers.TestMatrixSolversExternal WORKING_DIRECTORY . COMMAND python3 -m unittest tests.core.matrix_solvers.TestMatrixSolversExternal)
    add_test(NAME tests.core.matrix_solvers.TestMatrixSolversGeneral WORKING_DIRECTORY . COMMAND python3 -m unittest tests.core.matrix_solvers.TestMatrixSolversGeneral)
    add_test(NAME tests.core.matrix_solvers.TestMatrixSolversInternal WORKING_DIRECTORY . COMMAND python3 -m unittest tests.core.matrix_solvers.TestMatrixSolversInternal)
    add_test(NAME tests.core.mesh_generator.TestMeshGenerator WORKING_DIRECTORY . COMMAND python3 -m unittest tests.core.mesh_generator.TestMeshGenerator)
ENDIF(TEST_complete)

IF(TEST_deal.II)
    add_test(NAME tests.fields.electrostatic.TestElectrostaticPlanar WORKING_DIRECTORY . COMMAND python3 -m unittest tests.fields.electrostatic.TestElectrostaticPlanar)
    add_test(NAME tests.fields.electrostatic.TestElectrostaticAxisymmetric WORKING_DIRECTORY . COMMAND python3 -m unittest tests.fields.electrostatic.TestElectrostaticAxisymmetric)
    add_test(NAME tests.fields.current.TestCurrentPlanar WORKING_DIRECTORY . COMMAND python3 -m unittest tests.fields.current.TestCurrentPlanar)
    add_test(NAME tests.fields.current.TestCurrentAxisymmetric WORKING_DIRECTORY . COMMAND python3 -m unittest tests.fields.current.TestCurrentAxisymmetric)
    add_test(NAME tests.fields.elasticity.TestElasticityPlanar WORKING_DIRECTORY . COMMAND python3 -m unittest tests.fields.elasticity.TestElasticityPlanar)
    add_test(NAME tests.fields.elasticity.TestElasticityAxisymmetric WORKING_DIRECTORY . COMMAND python3 -m unittest tests.fields.elasticity.TestElasticityAxisymmetric)
    add_test(NAME tests.fields.flow.TestFlowPlanar WORKING_DIRECTORY . COMMAND python3 -m unittest tests.fields.flow.TestFlowPlanar)
    add_test(NAME tests.fields.flow.TestFlowAxisymmetric WORKING_DIRECTORY . COMMAND python3 -m unittest tests.fields.flow.TestFlowAxisymmetric)
    add_test(NAME tests.fields.acoustic.TestAcousticHarmonicPlanar WORKING_DIRECTORY . COMMAND python3 -m unittest tests.fields.acoustic.TestAcousticHarmonicPlanar)
    add_test(NAME tests.fields.acoustic.TestAcousticHarmonicAxisymmetric WORKING_DIRECTORY . COMMAND python3 -m unittest tests.fields.acoustic.TestAcousticHarmonicAxisymmetric)
    add_test(NAME tests.fields.heat.TestHeatPlanar WORKING_DIRECTORY . COMMAND python3 -m unittest tests.fields.heat.TestHeatPlanar)
    add_test(NAME tests.fields.heat.TestHeatAxisymmetric WORKING_DIRECTORY . COMMAND python3 -m unittest tests.fields.heat.TestHeatAxisymmetric)
    add_test(NAME tests.fields.heat.TestHeatNonlinPlanarNewton WORKING_DIRECTORY . COMMAND python3 -m unittest tests.fields.heat.TestHeatNonlinPlanarNewton)
    add_test(NAME tests.fields.heat.TestHeatNonlinPlanarPicard WORKING_DIRECTORY . COMMAND python3 -m unittest tests.fields.heat.TestHeatNonlinPlanarPicard)
    add_test(NAME tests.fields.heat.TestHeatTransientAxisymmetric WORKING_DIRECTORY . COMMAND python3 -m unittest tests.fields.heat.TestHeatTransientAxisymmetric)
    add_test(NAME tests.fields.magnetic_steady.TestMagneticPlanar WORKING_DIRECTORY . COMMAND python3 -m unittest tests.fields.magnetic_steady.TestMagneticPlanar)
    add_test(NAME tests.fields.magnetic_steady.TestMagneticAxisymmetric WORKING_DIRECTORY . COMMAND python3 -m unittest tests.fields.magnetic_steady.TestMagneticAxisymmetric)
    add_test(NAME tests.fields.magnetic_harmonic.TestMagneticHarmonicPlanar WORKING_DIRECTORY . COMMAND python3 -m unittest tests.fields.magnetic_harmonic.TestMagneticHarmonicPlanar)
    add_test(NAME tests.fields.magnetic_harmonic.TestMagneticHarmonicAxisymmetric WORKING_DIRECTORY . COMMAND python3 -m unittest tests.fields.magnetic_harmonic.TestMagneticHarmonicAxisymmetric)
    add_test(NAME tests.fields.magnetic_steady.TestMagneticNonlinPlanar WORKING_DIRECTORY . COMMAND python3 -m unittest tests.fields.magnetic_steady.TestMagneticNonlinPlanar)
    add_test(NAME tests.fields.magnetic_steady.TestMagneticNonlinAxisymmetric WORKING_DIRECTORY . COMMAND python3 -m unittest tests.fields.magnetic_steady.TestMagneticNonlinAxisymmetric)
    add_test(NAME tests.fields.rf_te.TestRFTEHarmonicPlanar WORKING_DIRECTORY . COMMAND python3 -m unittest tests.fields.rf_te.TestRFTEHarmonicPlanar)
    add_test(NAME tests.fields.rf_te.TestRFTEHarmonicAxisymmetric WORKING_DIRECTORY . COMMAND python3 -m unittest tests.fields.rf_te.TestRFTEHarmonicAxisymmetric)
    add_test(NAME tests.fields.rf_tm.TestRFTMHarmonicPlanar WORKING_DIRECTORY . COMMAND python3 -m unittest tests.fields.rf_tm.TestRFTMHarmonicPlanar)
    add_test(NAME tests.fields.rf_tm.TestRFTMHarmonicAxisymmetric WORKING_DIRECTORY . COMMAND python3 -m unittest tests.fields.rf_tm.TestRFTMHarmonicAxisymmetric)
    add_test(NAME tests.fields.math_coeff.TestMathCoeffPlanar WORKING_DIRECTORY . COMMAND python3 -m unittest tests.fields.math_coeff.TestMathCoeffPlanar)
    add_test(NAME tests.fields.math_coeff.TestMathCoeffAxisymmetric WORKING_DIRECTORY . COMMAND python3 -m unittest tests.fields.math_coeff.TestMathCoeffAxisymmetric)
    add_test(NAME tests.adaptivity.adaptivity.TestAdaptivityElectrostatic WORKING_DIRECTORY . COMMAND python3 -m unittest tests.adaptivity.adaptivity.TestAdaptivityElectrostatic)
    add_test(NAME tests.adaptivity.adaptivity.TestAdaptivityAcoustic WORKING_DIRECTORY . COMMAND python3 -m unittest tests.adaptivity.adaptivity.TestAdaptivityAcoustic)
    add_test(NAME tests.adaptivity.adaptivity.TestAdaptivityElasticityBracket WORKING_DIRECTORY . COMMAND python3 -m unittest tests.adaptivity.adaptivity.TestAdaptivityElasticityBracket)
    add_test(NAME tests.adaptivity.adaptivity.TestAdaptivityMagneticProfileConductor WORKING_DIRECTORY . COMMAND python3 -m unittest tests.adaptivity.adaptivity.TestAdaptivityMagneticProfileConductor)
    add_test(NAME tests.adaptivity.adaptivity.TestAdaptivityHLenses WORKING_DIRECTORY . COMMAND python3 -m unittest tests.adaptivity.adaptivity.TestAdaptivityHLenses)
    add_test(NAME tests.particle_tracing.particle_tracing.TestParticleTracingPlanar WORKING_DIRECTORY . COMMAND python3 -m unittest tests.particle_tracing.particle_tracing.TestParticleTracingPlanar)
    add_test(NAME tests.coupled_problems.basic_coupled_problems.TestCoupledProblemsBasic1 WORKING_DIRECTORY . COMMAND python3 -m unittest tests.coupled_problems.basic_coupled_problems.TestCoupledProblemsBasic1)
    add_test(NAME tests.coupled_problems.basic_coupled_problems.TestCoupledProblemsBasic2 WORKING_DIRECTORY . COMMAND python3 -m unittest tests.coupled_problems.basic_coupled_problems.TestCoupledProblemsBasic2)
    add_test(NAME tests.coupled_problems.basic_coupled_problems.TestCoupledProblemsBasic3 WORKING_DIRECTORY . COMMAND python3 -m unittest tests.coupled_problems.basic_coupled_problems.TestCoupledProblemsBasic3)
    add_test(NAME tests.coupled_problems.basic_coupled_problems.TestCoupledProblemsBasic4 WORKING_DIRECTORY . COMMAND python3 -m unittest tests.coupled_problems.basic_coupled_problems.TestCoupledProblemsBasic4)
    add_test(NAME tests.coupled_problems.unrealistic_coupled_problems.TestCoupledProblemsManyDomains WORKING_DIRECTORY . COMMAND python3 -m unittest tests.coupled_problems.unrealistic_coupled_problems.TestCoupledProblemsManyDomains)
    add_test(NAME tests.core.matrix_solvers.TestMatrixSolversInternal WORKING_DIRECTORY . COMMAND python3 -m unittest tests.core.matrix_solvers.TestMatrixSolversInternal)
    add_test(NAME tests.optilab.studies.TestNSGA2Sphere WORKING_DIRECTORY . COMMAND python3 -m unittest tests.optilab.studies.TestNSGA2Sphere)
    add_test(NAME tests.optilab.studies.TestNSGA3Sphere WORKING_DIRECTORY . COMMAND python3 -m unittest tests.optilab.studies.TestNSGA3Sphere)
    add_test(NAME tests.optilab.studies.TestLimboSphere WORKING_DIRECTORY . COMMAND python3 -m unittest tests.optilab.studies.TestLimboSphere)
    add_test(NAME tests.optilab.studies.TestBayesOptBooth WORKING_DIRECTORY . COMMAND python3 -m unittest tests.optilab.studies.TestBayesOptBooth)
    add_test(NAME tests.optilab.studies.TestNLoptBooth WORKING_DIRECTORY . COMMAND python3 -m unittest tests.optilab.studies.TestNLoptBooth)
    add_test(NAME tests.script.benchmark.BenchmarkGeometryTransformation WORKING_DIRECTORY . COMMAND python3 -m unittest tests.script.benchmark.BenchmarkGeometryTransformation)
    add_test(NAME tests.script.field.TestField WORKING_DIRECTORY . COMMAND python3 -m unittest tests.script.field.TestField)
    add_test(NAME tests.script.field.TestFieldAdaptivity WORKING_DIRECTORY . COMMAND python3 -m unittest tests.script.field.TestFieldAdaptivity)
    add_test(NAME tests.script.field.TestFieldAdaptivityInfo WORKING_DIRECTORY . COMMAND python3 -m unittest tests.script.field.TestFieldAdaptivityInfo)
    add_test(NAME tests.script.field.TestFieldBoundaries WORKING_DIRECTORY . COMMAND python3 -m unittest tests.script.field.TestFieldBoundaries)
    add_test(NAME tests.script.field.TestFieldIntegrals WORKING_DIRECTORY . COMMAND python3 -m unittest tests.script.field.TestFieldIntegrals)
    add_test(NAME tests.script.field.TestFieldLocalValues WORKING_DIRECTORY . COMMAND python3 -m unittest tests.script.field.TestFieldLocalValues)
    add_test(NAME tests.script.field.TestFieldMaterials WORKING_DIRECTORY . COMMAND python3 -m unittest tests.script.field.TestFieldMaterials)
    add_test(NAME tests.script.field.TestFieldMatrixSolver WORKING_DIRECTORY . COMMAND python3 -m unittest tests.script.field.TestFieldMatrixSolver)
    add_test(NAME tests.script.field.TestFieldNewtonSolver WORKING_DIRECTORY . COMMAND python3 -m unittest tests.script.field.TestFieldNewtonSolver)
    add_test(NAME tests.script.field.TestFieldSolverInfo WORKING_DIRECTORY . COMMAND python3 -m unittest tests.script.field.TestFieldSolverInfo)
    add_test(NAME tests.script.geometry.TestGeometry WORKING_DIRECTORY . COMMAND python3 -m unittest tests.script.geometry.TestGeometry)
    add_test(NAME tests.script.geometry.TestGeometryTransformations WORKING_DIRECTORY . COMMAND python3 -m unittest tests.script.geometry.TestGeometryTransformations)
    add_test(NAME tests.script.problem.TestProblem WORKING_DIRECTORY . COMMAND python3 -m unittest tests.script.problem.TestProblem)
    add_test(NAME tests.script.problem.TestProblemSolution WORKING_DIRECTORY . COMMAND python3 -m unittest tests.script.problem.TestProblemSolution)
    add_test(NAME tests.script.problem.TestProblemTime WORKING_DIRECTORY . COMMAND python3 -m unittest tests.script.problem.TestProblemTime)
ENDIF(TEST_deal.II)

IF(TEST_optilab)
    add_test(NAME tests.optilab.studies.TestBayesOptBooth WORKING_DIRECTORY . COMMAND python3 -m unittest tests.optilab.studies.TestBayesOptBooth)
    add_test(NAME tests.optilab.studies.TestLimboSphere WORKING_DIRECTORY . COMMAND python3 -m unittest tests.optilab.studies.TestLimboSphere)
    add_test(NAME tests.optilab.studies.TestNLoptBooth WORKING_DIRECTORY . COMMAND python3 -m unittest tests.optilab.studies.TestNLoptBooth)
    add_test(NAME tests.optilab.studies.TestNSGA2Sphere WORKING_DIRECTORY . COMMAND python3 -m unittest tests.optilab.studies.TestNSGA2Sphere)
    add_test(NAME tests.optilab.studies.TestNSGA3Sphere WORKING_DIRECTORY . COMMAND python3 -m unittest tests.optilab.studies.TestNSGA3Sphere)
ENDIF(TEST_optilab)

