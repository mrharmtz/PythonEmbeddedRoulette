#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include "roulette.hpp"

#define RLT_DEBUG

#ifdef RLT_DEBUG

#define RLT_FORMAT_LINE(FORMAT,...) PySys_WriteStdout("%05d:" FORMAT "\n", __LINE__, __VA_ARGS__)
#define RLT_PRINT_LINE(FORMAT)      PySys_WriteStdout("%05d:" FORMAT "\n", __LINE__)

#else

#define RLT_FORMAT_LINE(FORMAT,...)
#define RLT_PRINT_LINE(FORMAT)     

#endif

/********************************************************** roulette type **********************************************************/

typedef struct 
{
    PyObject_HEAD

    Roulette<PyObject*> roulette_handler;

}PyRoulette;

static PyTypeObject RouletteType = { PyVarObject_HEAD_INIT(NULL, 0) };


PyTypeObject* rlt_init_roulette_type(bool init){
    
    if(init){
        RouletteType.tp_name = "roulette.roulette";
        RouletteType.tp_basicsize = sizeof(PyRoulette);
        RouletteType.tp_itemsize = 0;
        RouletteType.tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE;
        RouletteType.tp_doc = "roulette object";
        RouletteType.tp_new = PyType_GenericNew;
    }

    return &RouletteType;
}

/********************************************************** roulette type **********************************************************/

/********************************************************** roulette module **********************************************************/

static PyObject* rlt_random_range(PyObject *self, PyObject *args){

    double min, max;
    NewRand randomizer;

    if(!PyArg_ParseTuple(args, "dd", &min, &max)) {
        return NULL;
    }

    if (min >= max)
    {
        PyErr_SetString(PyExc_ArithmeticError, "min cannot be greater than max");
        return NULL;
    }
    
    return Py_BuildValue("d", randomizer(min, max));
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

/********************************************************** roulette module **********************************************************/

PyMODINIT_FUNC PyInit_roulette(void)
{
    PyObject *module = NULL;
    PyTypeObject* roullete_type = NULL;

    bool complete = false;
    do{
        //ready new type
        if (PyType_Ready(rlt_init_roulette_type(true)) < 0)
            break;

        //ready module
        if (!(module = PyModule_Create(&roulette_module)))
            break;

        roullete_type = rlt_init_roulette_type(false);
        Py_INCREF(roullete_type);

        if (PyModule_AddObject(module, "roulette", (PyObject *) roullete_type) < 0)
            break;
        
        complete = true;

    }while(0);

    if(!complete){
        if(roullete_type){
            Py_DECREF(roullete_type);
            roullete_type = NULL;
        }

        if(module){
            Py_DECREF(module);
            module = NULL;
        }
    }

    return module;
}

