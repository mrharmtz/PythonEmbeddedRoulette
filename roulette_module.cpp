#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include "roulette.hpp"
#include <utility>

#define RLT_DEBUG

#ifdef RLT_DEBUG

#define RLT_FORMAT_LINE(FORMAT,...) PySys_WriteStdout("%05d:" FORMAT "\n", __LINE__, __VA_ARGS__)
#define RLT_PRINT_LINE(FORMAT)      PySys_WriteStdout("%05d:" FORMAT "\n", __LINE__)

#else

#define RLT_FORMAT_LINE(FORMAT,...)
#define RLT_PRINT_LINE(FORMAT)     

#endif

class PythonSmartPointer{
private:
    mutable PyObject* _py_object;
public:

    PythonSmartPointer()
    :_py_object(NULL)
    { }

    PythonSmartPointer(PyObject* py_object)
    :_py_object(py_object){
        Py_INCREF(_py_object);
    }

    PythonSmartPointer(const PythonSmartPointer& copy)
    :_py_object(copy._py_object){
        Py_INCREF(_py_object);
    }

    PythonSmartPointer& operator=(const PythonSmartPointer& rhs){
        if(_py_object)
            Py_XDECREF(_py_object);
        _py_object = rhs._py_object;
        Py_INCREF(_py_object);
        return *this;
    }

    bool operator==(const PythonSmartPointer& other)const{
        
        if(_py_object)
            return false;

        return PyObject_RichCompareBool(_py_object, other._py_object, Py_EQ);
    }

    bool operator!=(const PythonSmartPointer& other)const{

        if(_py_object)
            return true;

        return PyObject_RichCompareBool(_py_object, other._py_object, Py_NE);
    }

    operator PyObject*() const{
        return _py_object;
    }

    ~PythonSmartPointer(){
        if(_py_object)
            Py_XDECREF(_py_object);
    }

};

/********************************************************** roulette type **********************************************************/

typedef struct 
{
    PyObject_HEAD

    Roulette<PythonSmartPointer> roulette_handler;

}PyRoulette;

static PyTypeObject RouletteType = { PyVarObject_HEAD_INIT(NULL, 0) };

static void rlt_roulette_dealloc(PyRoulette *self)
{
    Py_TYPE(self)->tp_free((PyObject *) self);
}

static PyObject * rlt_roulette_insert(PyRoulette *self, PyObject *args)
{
    PyObject* object;
    double chance;

    if(!PyArg_ParseTuple(args, "Od", &object, &chance)) {
        return NULL;
    }

    //Py_INCREF(object);
    PythonSmartPointer ptr(object);
    self->roulette_handler.insert(ptr, chance);

    RLT_PRINT_LINE("");
    for (auto iter = self->roulette_handler.begin() ; iter != self->roulette_handler.end() ; ++iter){
        RLT_FORMAT_LINE("routlette object at %p with a chance of %lf", (PyObject*)iter->get_value(), iter->get_max() - iter->get_min());
    }

    Py_RETURN_NONE;
}

static PyObject * rlt_roulette_roll(PyRoulette *self, PyObject *Py_UNUSED(ignored))
{   
    PythonSmartPointer ret_val;
    RLT_FORMAT_LINE("function %s ", __func__);
    try{
         ret_val = self->roulette_handler.roll();
    }catch(...){
        RLT_PRINT_LINE("an exception was thrown");
        return NULL;
    }
    RLT_FORMAT_LINE("routlette rolled object at %p ", (PyObject*)ret_val);
    //Py_INCREF(ret_val);
    return (PyObject*)ret_val;
}

static PyObject * rlt_roulette_remove(PyRoulette *self, PyObject *args)
{
    PyObject* object;

    RLT_FORMAT_LINE("function %s ", __func__);

    if(!PyArg_ParseTuple(args, "O", &object)) {
        return NULL;
    }

    RLT_FORMAT_LINE("function %s ", __func__);

    //Py_INCREF(object);
    PythonSmartPointer ptr(object);
    RLT_FORMAT_LINE("function %s ", __func__);
    self->roulette_handler.remove(ptr);
    RLT_FORMAT_LINE("function %s ", __func__);

    RLT_PRINT_LINE("");
    for (auto iter = self->roulette_handler.begin() ; iter != self->roulette_handler.end() ; ++iter){
        RLT_FORMAT_LINE("routlette object at %p with a chance of %lf", (PyObject*)iter->get_value(), iter->get_max() - iter->get_min());
    }

    Py_RETURN_NONE;
}


static PyMethodDef rlt_roulette_methods[] = {
    {"insert", (PyCFunction) rlt_roulette_insert, METH_VARARGS, "inserts a python element into the roulette"},
    {"roll", (PyCFunction) rlt_roulette_roll, METH_NOARGS, "randomly choses an element and returns it"},
    {"remove", (PyCFunction) rlt_roulette_remove, METH_VARARGS, "removes a python element from roulette"},
    {NULL, NULL, 0, NULL}  /* Sentinel */
};

PyTypeObject* rlt_init_roulette_type(bool init){
    
    if(init){
        RouletteType.tp_name = "roulette.roulette";
        RouletteType.tp_basicsize = sizeof(PyRoulette);
        RouletteType.tp_itemsize = 0;
        RouletteType.tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE;
        RouletteType.tp_doc = "roulette object";
        RouletteType.tp_new = PyType_GenericNew;
        RouletteType.tp_dealloc = (destructor) rlt_roulette_dealloc,
        RouletteType.tp_methods = rlt_roulette_methods;
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
    {NULL,NULL,0,NULL} /* Sentinel */

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

