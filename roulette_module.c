#define PY_SSIZE_T_CLEAN
#include <Python.h>
//#include "roulette.hpp"


static PyObject* rlt_random_range(PyObject *self, PyObject *args){


    Py_RETURN_NONE;
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
