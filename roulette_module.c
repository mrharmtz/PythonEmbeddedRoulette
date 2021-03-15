#define PY_SSIZE_T_CLEAN
#include <Python.h>
//#include "roulette.hpp"

#define RLT_DEBUG

#ifdef RLT_DEBUG

#define RLT_FORMAT_LINE(FORMAT,...) PySys_WriteStdout("%05d:" FORMAT "\n", __LINE__, __VA_ARGS__)
#define RLT_PRINT_LINE(FORMAT)      PySys_WriteStdout("%05d:" FORMAT "\n", __LINE__)

#else

#define RLT_FORMAT_LINE(FORMAT,...)
#define RLT_PRINT_LINE(FORMAT)     

#endif

static PyObject* rlt_random_range(PyObject *self, PyObject *args){

    double min, max;

    if(!PyArg_ParseTuple(args, "dd", &min, &max)) {
        return NULL;
    }

    if (min >= max)
    {
        PyErr_SetString(PyExc_ArithmeticError, "min cannot be greater than max");
        return NULL;

    }
    

    RLT_FORMAT_LINE("min = %lf, max = %lf", min, max);
    return Py_BuildValue("d", 5.0);
}


static PyMethodDef roulette_methods[] = {

    {"random_range", (PyCFunction)rlt_random_range, METH_VARARGS, "returns value in passed range"},
    {NULL,NULL,0,NULL} 

};

static struct PyModuleDef roulette_module = {
    PyModuleDef_HEAD_INIT,
    .m_name = "roulette",   /* name of module */
    .m_doc = "wighted random chooser module", /* module documentation, may be NULL */
    .m_size = -1,       /* size of per-interpreter state of the module, or -1 if the module keeps state in global variables. */
    roulette_methods
};


PyMODINIT_FUNC PyInit_roulette(void)
{
    PyObject *module;

    if (!(module = PyModule_Create(&roulette_module)))
        return NULL;

    return module;
}
