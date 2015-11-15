cdef extern from "../../agros2d-library/pythonlab/pyproblem.h":
    cdef cppclass PyComputation:
        PyComputation(bool newComputation, string name)
        void setComputation(string &computation) except +

        void clear() except +

        void mesh() except +
        void solve() except +

        double timeElapsed() except +
        void timeStepsLength(vector[double] &steps) except +
        void timeStepsTimes(vector[double] &times) except +

    cdef cppclass PySolution:
        PySolution()

        void setComputation(PyComputation *computation)
        void setField(string &fieldId) except +

        void localValues(double x, double y, int timeStep, int adaptivityStep,
                         map[string, double] &results) except +
        void surfaceIntegrals(vector[int], int timeStep, int adaptivityStep,
                              map[string, double] &results) except +
        void volumeIntegrals(vector[int], int timeStep, int adaptivityStep,
                             map[string, double] &results) except +

        void initialMeshInfo(map[string , int] &info) except +
        void solutionMeshInfo(int timeStep, int adaptivityStep, map[string , int] &info) except +

        void solverInfo(int timeStep, int adaptivityStep, 
                        vector[double] &solution_change, vector[double] &residual,
                        vector[double] &dampingCoeff, int &jacobianCalculations) except +

        void adaptivityInfo(int timeStep, vector[double] &error, vector[int] &dofs) except +

        string filenameMatrix(int timeStep, int adaptivityStep) except +
        string filenameRHS(int timeStep, int adaptivityStep) except +
        string filenameSLN(int timeStep, int adaptivityStep) except +

cdef class __Computation__(__ProblemBase__):
    cdef PyComputation *_computation
    cdef object _solutions

    def __cinit__(self, new_computation = True, name = ""):
        __ProblemBase__.__init__(self)
        self._computation = new PyComputation(new_computation, name)
        self._solutions = dict()           
        
    def __dealloc__(self):
        del self._computation

    def clear(self):
        """Clear solution."""
        self._computation.clear()

    def mesh(self):
        """Area discretization."""
        self._computation.mesh()

    def solve(self):
        """Solve problem."""
        self._computation.solve()

    def solution(self, field_id):
        """Add new solution to Computation.

        solution(field_id)

        Keyword arguments:
        field_id -- field keyword 
        """

        if (not field_id in self._solutions):
            solution = __Solution__()
            solution._solution.setComputation(self._computation)
            solution._solution.setField(field_id.encode())

            self._solutions[field_id] = solution

        return self._solutions[field_id]

    def elapsed_time(self):
        """Return elapsed time in seconds."""
        return self._computation.timeElapsed()

    def time_steps_length(self):
        """Return a list of time steps length."""
        cdef vector[double] steps_vector
        self._computation.timeStepsLength(steps_vector)

        steps = list()
        for i in range(steps_vector.size()):
            steps.append(steps_vector[i])
        return steps

    def time_steps_total(self):
        """Return a list of time steps."""
        cdef vector[double] times_vector
        self._computation.timeStepsTimes(times_vector)

        times = list()
        for i in range(times_vector.size()):
            times.append(times_vector[i])
        return times

cdef class __Solution__:
    cdef PySolution *_solution

    def __cinit__(self):
        self._solution = new PySolution()

    def __dealloc__(self):
        del self._solution

    # local values
    def local_values(self, x, y, time_step = None, adaptivity_step = None):
        """Compute local values in point and return dictionary with results.

        local_values(x, y, time_step = None, adaptivity_step = None)

        Keyword arguments:
        x -- x or r coordinate of point
        y -- y or z coordinate of point
        time_step -- time step (default is None - use last time step)
        adaptivity_step -- adaptivity step (default is None - use adaptive step)        
        """

        out = dict()
        cdef map[string, double] results

        self._solution.localValues(x, y,
                                   int(-1 if time_step is None else time_step),
                                   int(-1 if adaptivity_step is None else adaptivity_step),
                                   results)
        it = results.begin()
        while it != results.end():
            out[deref(it).first.decode()] = deref(it).second
            incr(it)

        return out

    # surface integrals
    def surface_integrals(self, edges = [], time_step = None, adaptivity_step = None):
        """Compute surface integrals on edges and return dictionary with results.

        surface_integrals(edges = [], time_step = None, adaptivity_step = None)

        Keyword arguments:
        edges -- list of edges (default is [] - compute integrals on all edges)
        time_step -- time step (default is None - use last time step)
        adaptivity_step -- adaptivity step (default is None - use adaptive step)        
        """

        cdef vector[int] edges_vector
        for i in edges:
            edges_vector.push_back(i)

        out = dict()
        cdef map[string, double] results

        self._solution.surfaceIntegrals(edges_vector,
                                        int(-1 if time_step is None else time_step),
                                        int(-1 if adaptivity_step is None else adaptivity_step),
                                        results)
        it = results.begin()
        while it != results.end():
            out[deref(it).first.decode()] = deref(it).second
            incr(it)

        return out

    # volume integrals
    def volume_integrals(self, labels = [], time_step = None, adaptivity_step = None):
        """Compute volume integrals on labels and return dictionary with results.

        volume_integrals(labels = [], time_step = None, adaptivity_step = None)

        Keyword arguments:
        labels -- list of labels (default is [] - compute integrals on all labels)
        time_step -- time step (default is None - use last time step)
        adaptivity_step -- adaptivity step (default is None - use adaptive step)        
        """

        cdef vector[int] labels_vector
        for i in labels:
            labels_vector.push_back(i)

        out = dict()
        cdef map[string, double] results

        self._solution.volumeIntegrals(labels_vector,
                                       int(-1 if time_step is None else time_step),
                                       int(-1 if adaptivity_step is None else adaptivity_step),
                                       results)
        it = results.begin()
        while it != results.end():
            out[deref(it).first.decode()] = deref(it).second
            incr(it)

        return out

    # mesh info
    def initial_mesh_info(self):
        """Return dictionary with initial mesh info."""
        info = dict()
        cdef map[string, int] info_map

        self._solution.initialMeshInfo(info_map)
        it = info_map.begin()
        while it != info_map.end():
            info[deref(it).first.decode()] = deref(it).second
            incr(it)

        return info

    def solution_mesh_info(self, time_step = None, adaptivity_step = None):
        """Return dictionary with solution mesh info.

        solution_mesh_info(time_step = None, adaptivity_step = None)

        Keyword arguments:
        time_step -- time step (default is None - use last time step)
        adaptivity_step -- adaptivity step (default is None - use adaptive step)
        """

        info = dict()
        cdef map[string, int] info_map

        self._solution.solutionMeshInfo(int(-1 if time_step is None else time_step),
                                       int(-1 if adaptivity_step is None else adaptivity_step),
                                       info_map)

        it = info_map.begin()
        while it != info_map.end():
            info[deref(it).first.decode()] = deref(it).second
            incr(it)

        return info

    # solver info
    def solver_info(self, time_step = None, adaptivity_step = None):
        """Return dictionary with solver info.

        solver_info(time_step = None, adaptivity_step = None)

        Keyword arguments:
        time_step -- time step (default is None - use last time step)
        adaptivity_step -- adaptivity step (default is None - use adaptive step)
        """

        cdef vector[double] solution_change_vector
        cdef vector[double] residual_vector
        cdef vector[double] damping_vector
        cdef int jacobian_calculations
        jacobian_calculations = -1
        self._solution.solverInfo(int(-1 if time_step is None else time_step),
                                 int(-1 if adaptivity_step is None else adaptivity_step),
                                 solution_change_vector, residual_vector, 
                                 damping_vector, jacobian_calculations)

        solution_change = list()
        for i in range(solution_change_vector.size()):
            solution_change.append(solution_change_vector[i])

        residual = list()
        for i in range(residual_vector.size()):
            residual.append(residual_vector[i])

        damping = list()
        for i in range(damping_vector.size()):
            damping.append(damping_vector[i])

        return {'solution_change' : solution_change, 'residual' : residual, 'damping' : damping, 'jacobian_calculations' : jacobian_calculations}

    # adaptivity info
    def adaptivity_info(self, time_step = None):
        """Return dictionary with adaptivity process info.

        adaptivity_info(time_step = None)

        Keyword arguments:
        time_step -- time step (default is None - use last time step)
        """

        cdef vector[double] error_vector
        cdef vector[int] dofs_vector
        self._solution.adaptivityInfo(int(-1 if time_step is None else time_step),
                                     error_vector, dofs_vector)

        error = list()
        for i in range(error_vector.size()):
            error.append(error_vector[i])

        dofs = list()
        for i in range(dofs_vector.size()):
            dofs.append(dofs_vector[i])

        return {'error' : error, 'dofs' : dofs}

    # filename - matrix
    def filename_matrix(self, time_step = None, adaptivity_step = None):
        return self._solution.filenameMatrix(int(-1 if time_step is None else time_step),
                                             int(-1 if adaptivity_step is None else adaptivity_step)).decode()

    # filename - rhs
    def filename_rhs(self, time_step = None, adaptivity_step = None):
        return self._solution.filenameRHS(int(-1 if time_step is None else time_step),
                                          int(-1 if adaptivity_step is None else adaptivity_step)).decode()
                                        
    # filename - sln
    def filename_sln(self, time_step = None, adaptivity_step = None):
        return self._solution.filenameSLN(int(-1 if time_step is None else time_step),
                                          int(-1 if adaptivity_step is None else adaptivity_step)).decode()

def computation(computation):
    existing_computation = __Computation__(False, computation.encode())
    # existing_computation._computation.setComputation(computation.encode())
    return existing_computation
