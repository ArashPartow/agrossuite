// This file is part of Agros.
//
// Agros is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Agros is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Agros.  If not, see <http://www.gnu.org/licenses/>.
//
//
// University of West Bohemia, Pilsen, Czech Republic
// Email: info@agros2d.org, home page: http://agros2d.org/

#include <Python.h>

#include "compile.h"
#include "frameobject.h"

#include <iostream>
#include "pythonengine.h"

#include "../resources_source/python/pythonlab.cpp"

#ifdef Q_WS_X11
#include <csignal>
#endif

static PythonEngine *pythonEngine = NULL;

// silent mode
static bool m_silentMode = false;

// create custom python engine
void createPythonEngine(int argc, char *argv[], PythonEngine *custom)
{
    if (custom)
        pythonEngine = custom;
    else
        pythonEngine = new PythonEngine();

    pythonEngine->init(argc, argv);
}

// current python engine
PythonEngine *currentPythonEngine()
{
    return pythonEngine;
}

// silent mode
void setSilentMode(bool mode)
{
    m_silentMode = mode;
}

bool silentMode()
{
    return m_silentMode;
}

// ****************************************************************************

// print stdout
PyObject* pythonStdout(PyObject* self, PyObject* pArgs)
{
    char *str = NULL;
    if (PyArg_ParseTuple(pArgs, "s", &str))
    {
        emit currentPythonEngine()->pythonShowMessageCommand(QString(str));
        Py_RETURN_NONE;
    }
    return NULL;
}

// show image
PyObject* pythonShowFigure(PyObject* self, PyObject* pArgs)
{
    char *str = NULL;
    int width = 0;
    int height = 0;
    if (PyArg_ParseTuple(pArgs, "s|ii", &str, &width, &height))
    {
        emit currentPythonEngine()->pythonShowImageCommand(QString(str), width, height);
        Py_RETURN_NONE;
    }
    return NULL;
}

// print html
PyObject* pythonInsertHtml(PyObject* self, PyObject* pArgs)
{
    char *str = NULL;
    if (PyArg_ParseTuple(pArgs, "s", &str))
    {
        emit currentPythonEngine()->pythonShowHtmlCommand(QString(str));
        Py_RETURN_NONE;
    }
    return NULL;
}

// clear
static PyObject* pythonClear(PyObject* self, PyObject* pArgs)
{
    emit currentPythonEngine()->pythonClearCommand();

    Py_RETURN_NONE;
    return NULL;
}

int scriptQuit(void *)
{
    PyErr_SetString(PyExc_SystemError, "Script interrupted.");
    PyErr_SetInterrupt();

    return 0;
}

static PyObject *pythonTempname(PyObject* self, PyObject* pArgs)
{
    QString tempDir = tempProblemDir() + "/temp/";
    QDir(tempDir).mkdir(tempDir);

    QString tempName = QUuid::createUuid().toString().remove("{").remove("}");
    QString fn = QFileInfo(tempDir + tempName).absoluteFilePath();

    char *str = NULL;
    if (PyArg_ParseTuple(pArgs, "|s", &str))
    {
        if (!QString(str).trimmed().isEmpty())
            fn = fn + "." + str;
    }

    return PyUnicode_FromString(compatibleFilename(fn).toLatin1().data());
}

static PyMethodDef pythonEngineFuntions[] =
{
    {"__stdout__", pythonStdout, METH_VARARGS, "__stdout__(str)"},
    {"image", pythonShowFigure, METH_VARARGS, "image(file)"},
    {"clear", pythonClear, METH_NOARGS, "clear()"},
    {"html", pythonInsertHtml, METH_VARARGS, "html(str)"},
    {"tempname", pythonTempname, METH_VARARGS, "tempname(extension = \"\")"},
    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef pythonEngineDef = {
    PyModuleDef_HEAD_INIT,
    "pythonlab",
    "pythonlab doc",
    -1,
    pythonEngineFuntions
};

// ****************************************************************************

// tracing
int traceFunction(PyObject *obj, _frame *frame, int what, PyObject *arg)
{
    PyObject *str = PyObject_Str(frame->f_code->co_filename);
    if (str)
    {
        QString fileName = QString::fromWCharArray(PyUnicode_AsUnicode(str));
        Py_DECREF(str);

        if (!currentPythonEngine()->profilerFileName().isEmpty() && fileName.contains(currentPythonEngine()->profilerFileName()))
        {
            // catch line change events, print the filename and line number
            if (what == PyTrace_LINE)
            {
                currentPythonEngine()->profilerAddLine(frame->f_lineno);
                // qDebug() << "PyTrace_LINE: " << fileName << ": " << frame->f_lineno;
            }
            // catch line change events, print the filename and line number
            if (what == PyTrace_CALL)
            {
                // qDebug() << "PyTrace_CALL: " << fileName << ": " << frame->f_lineno;
            }
            // catch line change events, print the filename and line number
            if (what == PyTrace_RETURN)
            {
                // qDebug() << "PyTrace_RETURN: " << fileName << ": " << frame->f_lineno;
            }
        }
    }

    return 0;
}

void PythonEngineProfiler::startProfiler()
{
    // set trace callback
    PyEval_SetTrace(traceFunction, NULL);
    m_profilerTime.restart();
    profilerAddLine(0);
}

void PythonEngineProfiler::finishProfiler()
{
    // add last time
    profilerAddLine(m_profilerLines.last() + 1);

    // remove trace callback
    PyEval_SetTrace(NULL, NULL);

    assert(m_profilerLines.count() == m_profilerTimes.count());
    // clear accumulated lines and times
    m_profilerAccumulatedLines.clear();
    m_profilerAccumulatedTimes.clear();
    // store accumulated time
    m_profilerMaxAccumulatedCall = 0;
    m_profilerMaxAccumulatedTime = 0;
    for (int i = 1; i < m_profilerLines.count(); i++)
    {
        m_profilerAccumulatedLines[m_profilerLines.at(i-1)]++;
        m_profilerAccumulatedTimes[m_profilerLines.at(i-1)] += (m_profilerTimes.at(i) - m_profilerTimes.at(i - 1));

        if (m_profilerAccumulatedTimes[m_profilerLines.at(i-1)] > m_profilerMaxAccumulatedTime)
        {
            m_profilerMaxAccumulatedLine = m_profilerLines.at(i-1);
            m_profilerMaxAccumulatedTime = m_profilerAccumulatedTimes[m_profilerLines.at(i-1)];
        }
        if (m_profilerAccumulatedLines[m_profilerLines.at(i-1)] > m_profilerMaxAccumulatedCall)
        {
            m_profilerMaxAccumulatedCallLine = m_profilerLines.at(i-1);
            m_profilerMaxAccumulatedCall = m_profilerAccumulatedLines[m_profilerLines.at(i-1)];
        }
    }

    // clear temp variables
    m_profilerLines.clear();
    m_profilerTimes.clear();
}


// ****************************************************************************

PythonEngine::~PythonEngine()
{
    // finalize and garbage python
    Py_DECREF(dict());
    Py_DECREF(dict());

    if (Py_IsInitialized())
        Py_Finalize();

    for(int i = 0; i < this->argc; i++)
        delete [] this->argvw[i];
    delete [] this->argvw;
}

void PythonEngine::init(int argc, char *argv[])
{
    m_isScriptRunning = false;

    // init python
    Py_Initialize();
    PyEval_InitThreads();

    // args
    /// \todo Better
    this->argc = argc;
    this->argvw = new wchar_t* [this->argc];
    for(int i = 0; i < this->argc; i++)
    {
        this->argvw[i] = new wchar_t[1000];
        swprintf(argvw[i], 1000, L"%hs", argv[i]);
    }

    PySys_SetArgv(argc, argvw);


    // read pythonlab functions
    addFunctions(readFileContent(datadir() + "/resources/python/functions_pythonlab.py"));
    addCustomFunctions();

    // m_dict = PyDict_New();
    // PyDict_SetItemString(m_dict, "__builtins__", PyEval_GetBuiltins());

    PyObject *main = PyImport_ImportModule("__main__");
    Py_INCREF(main);
    m_dictGlobal = PyModule_GetDict(main);
    Py_INCREF(m_dictGlobal);

    // init engine extensions
    PyObject *m = PyModule_Create(&pythonEngineDef);
    PyDict_SetItemString(PyImport_GetModuleDict(), pythonEngineDef.m_name, m);

    addCustomExtensions();

    // custom modules
    PyObject *import = PyRun_String(QString("import sys; sys.path.insert(0, \"" + datadir() + "/resources/python" + "\")").toLatin1().data(), Py_file_input, dict(), dict());
    Py_XDECREF(import);

    // functions.py
    PyObject *func = PyRun_String(m_functions.toLatin1().data(), Py_file_input, dict(), dict());
    Py_XDECREF(func);
}

void PythonEngine::useLocalDict()
{
    m_useGlobalDict = false;

    m_dictLocal = PyDict_New();
    Py_INCREF(m_dictLocal);

    // init engine extensions
    PyObject *m = PyModule_Create(&pythonEngineDef);
    PyDict_SetItemString(PyImport_GetModuleDict(), pythonEngineDef.m_name, m);
}

void PythonEngine::useGlobalDict()
{
    m_useGlobalDict = true;

    if (m_dictLocal)
    {
        Py_XDECREF(m_dictLocal);
        m_dictLocal = NULL;
    }
}

void PythonEngine::abortScript()
{
    Py_AddPendingCall(&scriptQuit, NULL);
}

void PythonEngine::pythonShowMessageCommand(const QString &message)
{
    emit pythonShowMessage(message);
}

void PythonEngine::pythonShowImageCommand(const QString &fileName, int width, int height)
{
    emit pythonShowImage(fileName, width, height);
}

void PythonEngine::pythonShowHtmlCommand(const QString &fileName)
{
    emit pythonShowHtml(fileName);
}

void PythonEngine::pythonClearCommand()
{
    emit pythonClear();
}

void PythonEngine::deleteUserModules()
{
    // delete all user modules
    //
    // When working with Python scripts interactively, one must keep in mind that Python
    // import a module from its source code (on disk) only when parsing the first corresponding
    // import statement. During this first import, the byte code is generated (.pyc file)
    // if necessary and the imported module code object is cached in sys.modules. Then, when
    // re-importing the same module, this cached code object will be directly used even
    // if the source code file (.py[w] file) has changed meanwhile.
    //
    // This behavior is sometimes unexpected when working with the Python interpreter in
    // interactive mode, because one must either always restart the interpreter or remove manually the .pyc
    // files to be sure that changes made in imported modules were taken into account.

    QStringList filter_name;
    filter_name << "pythonlab" << "agros2d" << "variant" << "sys" << "test_suite" << "jedi" << "pylint" << "pyflakes";

    QList<PythonVariable> list = variableList();

    foreach (PythonVariable variable, list)
    {
        if (variable.type == "module")
        {
            if (filter_name.contains(variable.name))
                continue;

            QString exp = QString("del %1; import sys; del sys.modules[\"%1\"]").arg(variable.name);
#pragma omp critical(del)
            {
                PyObject *del = PyRun_String(exp.toLatin1().data(), Py_single_input, dict(), dict());
                Py_XDECREF(del);
            }
        }
    }

    PyErr_Clear();
}

bool PythonEngine::runScript(const QString &script, const QString &fileName)
{
    m_isScriptRunning = true;

    PyGILState_STATE gstate = PyGILState_Ensure();

    emit startedScript();

    bool successfulRun = false;

    QSettings settings;
    // enable user module deleter
    if (settings.value("PythonEngine/UserModuleDeleter", true).toBool())
        deleteUserModules();

    runPythonHeader();

    PyObject *output = NULL;
    if (QFile::exists(fileName))
    {
        QString str = QString("from os import chdir; chdir(u'" + QFileInfo(fileName).absolutePath() + "')");
#pragma omp critical(import)
        {
            PyObject *import = PyRun_String(str.toLatin1().data(), Py_single_input, dict(), dict());
            Py_XDECREF(import);
        }
    }

    // compile
    PyObject *code = Py_CompileString(script.toLatin1().data(), fileName.toLatin1().data(), Py_file_input);
    // run
    if (m_useProfiler)
    {
        setProfilerFileName(fileName);
        startProfiler();
    }

    if (code) output = PyEval_EvalCode(code, dict(), dict());
    
    if (m_useProfiler)
        finishProfiler();

    if (output)
    {
        successfulRun = true;
        Py_XDECREF(output);
    }
    else
    {
        // error traceback
        Py_XDECREF(errorType);
        Py_XDECREF(errorValue);
        Py_XDECREF(errorTraceback);
        PyErr_Fetch(&errorType, &errorValue, &errorTraceback);
        if (errorTraceback)
        {
            successfulRun = false;

            ErrorResult result = parseError(false);
            if (!result.error().isEmpty())
                qDebug() << result.tracebackToString();
        }
    }

    Py_XDECREF(code);

    m_isScriptRunning = false;

    // release the thread, no Python API allowed beyond this point
    PyGILState_Release(gstate);

    emit executedScript();

    return successfulRun;
}

bool PythonEngine::runExpression(const QString &expression, double *value, const QString &command)
{
    bool successfulRun = false;

    PyObject *output = NULL;
    runPythonHeader();

    if (value)
    {
        // return value
        QString exp;
        if (command.isEmpty())
            exp = QString("result_pythonlab = %1").arg(expression);
        else
            exp = QString("%1; result_pythonlab = %2").arg(command).arg(expression);

#pragma omp critical(expression)
        {
            output = PyRun_String(exp.toLatin1().data(), Py_single_input, dict(), dict());
        }

        if (output)
        {
            // parse result
            PyObject *result = PyDict_GetItemString(dict(), "result_pythonlab");

            if (result)
            {
                if ((QString(result->ob_type->tp_name) == "bool") ||
                        (QString(result->ob_type->tp_name) == "int") ||
                        (QString(result->ob_type->tp_name) == "float"))
                {
                    Py_INCREF(result);
                    PyArg_Parse(result, "d", value);
                    if (fabs(*value) < EPS_ZERO)
                        *value = 0.0;
                    Py_XDECREF(result);

                    successfulRun = true;
                }
                else
                {
                    qDebug() << tr("Type '%1' is not supported.").arg(result->ob_type->tp_name).arg(expression);

                    successfulRun = false;
                }
            }

            // speed up?
            // PyRun_String("del result_pythonlab", Py_single_input, m_dict, m_dict);
        }
    }
    else
    {
#pragma omp critical(expression)
        {
            output = PyRun_String(expression.toLatin1().data(), Py_single_input, dict(), dict());
        }
        if (output)
            successfulRun = true;
    }

    if (!output)
    {
        // error traceback
        Py_XDECREF(errorType);
        Py_XDECREF(errorValue);
        Py_XDECREF(errorTraceback);
        PyErr_Fetch(&errorType, &errorValue, &errorTraceback);
        if (errorTraceback)
        {
            ErrorResult result = parseError(false);
            if (!result.error().isEmpty())
                qDebug() << result.tracebackToString();

            successfulRun = false;
        }
    }

    Py_XDECREF(output);

    return successfulRun;
}

bool PythonEngine::runExpressionConsole(const QString &expression)
{
    bool successfulRun = runExpression(expression);
    if (successfulRun)
        emit executedScript();

    return successfulRun;
}

QStringList PythonEngine::codeCompletionScript(const QString& code, int row, int column, const QString& fileName)
{
    runPythonHeader();

    QString fn = "";
    if (QFile::exists(fileName))
        fn = fileName;

    QString exp = QString("result_jedi_pythonlab = python_engine_get_completion_script(\"\"\"%1\"\"\", %2, %3, \"%4\")").
            arg(code).
            arg(row).
            arg(column).
            arg(fn);

    return codeCompletion(exp);
}

QStringList PythonEngine::codeCompletionInterpreter(const QString& code)
{
    runPythonHeader();

    QString exp = QString("result_jedi_pythonlab = python_engine_get_completion_interpreter(\"%1\")").
            arg(code);

    return codeCompletion(exp);
}

QStringList PythonEngine::codeCompletion(const QString& command)
{
    QStringList out;

    runPythonHeader();

#pragma omp critical(completion)
    {
        PyObject *output = PyRun_String(command.toLatin1().data(), Py_single_input, dict(), dict());

        // parse result
        if (output)
        {
            PyObject *result = PyDict_GetItemString(dict(), "result_jedi_pythonlab");
            if (result)
            {
                Py_INCREF(result);
                PyObject *list;
                if (PyArg_Parse(result, "O", &list))
                {
                    int count = PyList_Size(list);
                    for (int i = 0; i < count; i++)
                    {
                        PyObject *item = PyList_GetItem(list, i);

                        QString str = QString::fromWCharArray(PyUnicode_AsUnicode(item));

                        // remove builtin methods
                        if (!str.startsWith("__"))
                        {
                            //qDebug() << str;
                            out.append(str);
                        }
                    }
                }
                Py_DECREF(result);
            }

            PyObject *del = PyRun_String("del result_jedi_pythonlab", Py_single_input, dict(), dict());
            Py_XDECREF(del);
        }
        else
        {
            PyErr_Clear();
        }

        Py_XDECREF(output);
    }

    return out;
}

QStringList PythonEngine::codePyFlakes(const QString& fileName)
{
    QStringList out;

    if (!m_isScriptRunning)
    {
        QString exp = QString("result_pyflakes_pythonlab = python_engine_pyflakes_check(\"%1\")").arg(compatibleFilename(fileName));

#pragma omp critical(flakes)
        {
            PyObject *run = PyRun_String(exp.toLatin1().data(), Py_single_input, dict(), dict());
            // parse result
            PyObject *result = PyDict_GetItemString(dict(), "result_pyflakes_pythonlab");
            if (result)
            {
                Py_INCREF(result);
                PyObject *list;
                if (PyArg_Parse(result, "O", &list))
                {
                    int count = PyList_Size(list);
                    for (int i = 0; i < count; i++)
                    {
                        PyObject *item = PyList_GetItem(list, i);

                        QString str = QString::fromWCharArray(PyUnicode_AsUnicode(item));
                        out.append(str);
                    }
                }
                Py_DECREF(result);
            }
            Py_XDECREF(run);

            PyObject *del = PyRun_String("del result_pyflakes_pythonlab", Py_single_input, dict(), dict());
            Py_XDECREF(del);
        }
    }

    return out;
}

PythonGotoDefinition PythonEngine::codeGotoDefinition(const QString& filename, int row, int column)
{
    PythonGotoDefinition out;

#pragma omp critical(completion)
    {
        QString str = QString("agros2d_goto_definition = python_engine_goto_definition(path = '%1', line = %2, column = %3)").arg(filename).arg(row).arg(column);
        runExpression(str);

        // parse result
        PyObject *result = PyDict_GetItemString(dict(), "agros2d_goto_definition");
        if (result)
        {
            Py_INCREF(result);

            PyObject *dmodule_path = PyList_GetItem(result, 0);
            Py_INCREF(dmodule_path);
            out.module_path = QString::fromWCharArray(PyUnicode_AsUnicode(dmodule_path));
            Py_XDECREF(dmodule_path);

            PyObject *dline = PyList_GetItem(result, 1);
            Py_INCREF(dline);
            out.line = PyInt_AsLong(dline);
            Py_XDECREF(dline);

            PyObject *dfull_name = PyList_GetItem(result, 2);
            Py_INCREF(dfull_name);
            out.full_name = QString::fromWCharArray(PyUnicode_AsUnicode(dfull_name));
            Py_XDECREF(dfull_name);

            PyObject *dname = PyList_GetItem(result, 3);
            Py_INCREF(dname);
            out.name = QString::fromWCharArray(PyUnicode_AsUnicode(dname));
            Py_XDECREF(dname);

            Py_XDECREF(result);
        }

        PyObject *del = PyRun_String("del agros2d_goto_definition", Py_single_input, dict(), dict());
        Py_XDECREF(del);
    }

    return out;
}

QString PythonEngine::codeHelp(const QString& filename, int row, int column)
{
    QString help = "";

#pragma omp critical(completion)
    {
        QString str = QString("agros2d_code_help = python_engine_code_help(path = '%1', line = %2, column = %3)").arg(filename).arg(row).arg(column);
        runExpression(str);

        // parse result
        PyObject *result = PyDict_GetItemString(dict(), "agros2d_code_help");
        if (result)
        {
            Py_INCREF(result);
            help = QString::fromWCharArray(PyUnicode_AsUnicode(result));
            Py_XDECREF(result);
        }

        PyObject *del = PyRun_String("del agros2d_code_help", Py_single_input, dict(), dict());
        Py_XDECREF(del);
    }

    return help;
}

ErrorResult PythonEngine::parseError(bool clear)
{
    QList<ErrorResult::ErrorTraceback> traceback;
    QString text;
    int line = -1;

    PyErr_NormalizeException(&errorType, &errorValue, &errorTraceback);

    if (errorTraceback)
    {
        PyTracebackObject *tb = (PyTracebackObject *) errorTraceback;
        line = tb->tb_lineno;
        text.append(QString("Line %1: ").arg(tb->tb_lineno));

        while (tb)
        {
            PyFrameObject *frame = tb->tb_frame;

            QString fileName;
            int errorLine = -1;
            QString name;

            if (frame && frame->f_code)
            {
                PyCodeObject* codeObject = frame->f_code;
                if (PyString_Check(codeObject->co_filename))
                    fileName = QString::fromWCharArray(PyUnicode_AsUnicode(codeObject->co_filename));

                errorLine = PyCode_Addr2Line(codeObject, frame->f_lasti);

                if (PyString_Check(codeObject->co_name))
                    name = QString::fromWCharArray(PyUnicode_AsUnicode(codeObject->co_name));
            }

            traceback.append(ErrorResult::ErrorTraceback(fileName, errorLine, name));

            tb = tb->tb_next;
        }
    }

    PyObject *errorString = NULL;
    if (errorType != NULL && (errorString = PyObject_Str(errorType)) != NULL && (PyString_Check(errorString)))
    {
        Py_INCREF(errorString);
        text.append(QString::fromWCharArray(PyUnicode_AsUnicode(errorString)));
        Py_XDECREF(errorString);
    }
    else
    {
        text.append("\n<unknown exception type>");
    }

    if (errorValue != NULL && (errorString = PyObject_Str(errorValue)) != NULL && (PyString_Check(errorString)))
    {
        Py_INCREF(errorString);
        text.append("\n");
        text.append(QString::fromWCharArray(PyUnicode_AsUnicode(errorString)));
        Py_XDECREF(errorString);
    }
    else
    {
        text.append("\n<unknown exception data>");
    }

    if (clear)
    {
        Py_XDECREF(errorType);
        errorType = NULL;
        Py_XDECREF(errorValue);
        errorValue = NULL;
        Py_XDECREF(errorTraceback);
        errorTraceback = NULL;

        PyErr_Clear();
    }

    return ErrorResult(text.trimmed(), traceback, line);
}

void PythonEngine::addCustomExtensions()
{
    // init pythonlab cython extensions
    PyObject *m = PyInit_pythonlab();
    PyDict_SetItemString(PyImport_GetModuleDict(), "pythonlab_cython", m);

    // merge modules
    PyObject *del = PyRun_String("import pythonlab_cython; import pythonlab; pythonlab.__dict__.update(pythonlab_cython.__dict__); del pythonlab_cython;", Py_single_input, dict(), dict());
    Py_XDECREF(del);
}

QList<PythonVariable> PythonEngine::variableList()
{
    QStringList filter_name;
    filter_name << "__builtins__"
                << "StdoutCatcher"
                << "python_engine_stdout"
                << "result_pythonlab"
                << "python_engine_get_completion_interpreter"
                << "python_engine_get_completion_script"
                << "PythonLabRopeProject"
                << "pythonlab_rope_project"
                << "python_engine_pyflakes_check"
                << "CatchOutErr"
                << "agros2d_material_eval"
                << "chart"
                << "memory_chart"
                << "sgn"
                << "test"
                << "vtk_contours_actor"
                << "vtk_figure"
                << "vtk_geometry_actor"
                << "vtk_scalar_actor"
                << "help"
                << "__a2d_help__";

    QStringList filter_type;
    filter_type << "builtin_function_or_method";

    QList<PythonVariable> list;

    PyObject *keys = PyDict_Keys(dict());
    for (int i = 0; i < PyList_Size(keys); ++i)
    {
        PyObject *key = PyList_GetItem(keys, i);
        PyObject *value = PyDict_GetItem(dict(), key);

        // variable
        PythonVariable var;

        // variable name
        var.name = QString::fromWCharArray(PyUnicode_AsUnicode(key));

        // variable type
        var.type = value->ob_type->tp_name;

        // variable value
        if (var.type == "bool")
        {
            var.value = PyInt_AsLong(value) ? "True" : "False";
        }
        else if (var.type == "int")
        {
            var.value = (int) PyInt_AsLong(value);
        }
        else if (var.type == "float")
        {
            var.value = PyFloat_AsDouble(value);
        }
        else if (var.type == "str")
        {
            var.value = QString::fromWCharArray(PyUnicode_AsUnicode(value));
        }
        else if (var.type == "list")
        {
            var.value = QString("%1 items").arg(PyList_Size(value));
        }
        else if (var.type == "tuple")
        {
            var.value = QString("%1 items").arg(PyTuple_Size(value));
        }
        else if (var.type == "dict")
        {
            var.value = QString("%1 items").arg(PyDict_Size(value));
        }
        else if (var.type == "numpy.ndarray")
        {
            var.value = ""; //TODO count
        }
        else if (var.type == "module")
        {
            var.value = QString::fromWCharArray(PyUnicode_AsUnicode(PyObject_GetAttrString(value, "__name__")));
        }
        else if (var.type == "function"
                 || var.type == "instance"
                 || var.type == "classobj")
        {
            // qDebug() << value->ob_type->tp_name;
        }

        // append
        if (!filter_name.contains(var.name) && !filter_type.contains(var.type))
        {
            list.append(var);
        }
    }
    Py_DECREF(keys);

    return list;
}
