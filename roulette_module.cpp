#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <utility>

#define ROULETTE_DEBUG_PYTHON
#include "roulette.hpp"

#define RLT_DEBUG

#ifdef RLT_DEBUG

#define RLT_FORMAT_LINE(FORMAT,...) PySys_WriteStdout("%05d:%s:" FORMAT "\n", __LINE__, __func__, __VA_ARGS__)
#define RLT_PRINT_LINE(FORMAT)      PySys_WriteStdout("%05d:%s:" FORMAT "\n", __LINE__, __func__)

#else

#define RLT_FORMAT_LINE(FORMAT,...)
#define RLT_PRINT_LINE(FORMAT)     

#endif

/********************************************************** python smart pointer **********************************************************/

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
        
        if(!_py_object)
            return false;

        return PyObject_RichCompareBool(_py_object, other._py_object, Py_EQ);
    }

    bool operator!=(const PythonSmartPointer& other)const{

        if(!_py_object)
            return true;

        return PyObject_RichCompareBool(_py_object, other._py_object, Py_NE);
    }

    operator PyObject*() const{
        return _py_object;
    }

    operator std::string()const{
        PyObject* objects_representation = PyObject_Str(_py_object);

        if(!objects_representation)
            return "";

        const char* s = PyUnicode_AsUTF8(objects_representation);

        if(!s)
            return "";

        return s;

    }

    ~PythonSmartPointer(){
        if(_py_object)
            Py_XDECREF(_py_object);
    }

};

/********************************************************** python smart pointer **********************************************************/

/********************************************************** type decleration **********************************************************/

//--------------------------- PyRoulette ---------------------------//
typedef struct 
{
    PyObject_HEAD

    Roulette<PythonSmartPointer>* roulette_handler;

}PyRoulette;

static PyTypeObject RouletteType = { PyVarObject_HEAD_INIT(NULL, 0) };
static PySequenceMethods RouletteTypeSequenceMethods;

static Py_ssize_t rlt_roulette_len(PyRoulette *self);
static void rlt_roulette_dealloc(PyRoulette *self);
static PyObject* rlt_roulette_new(PyTypeObject *type, PyObject *args, PyObject *kwds);
static PyObject * rlt_roulette_insert(PyRoulette *self, PyObject *args);
static int rlt_roulette_init(PyRoulette *self, PyObject *args, PyObject *kwds);
static PyObject * rlt_roulette_roll(PyRoulette *self, PyObject *Py_UNUSED(ignored));
static PyObject * rlt_roulette_remove(PyRoulette *self, PyObject *args);
static PyObject* rlt_roulette_iterator(PyRoulette* self);

//--------------------------- PyRoulette ---------------------------//

//--------------------------- PyRouletteIterator ---------------------------//

typedef struct 
{
    PyObject_HEAD

    Roulette<PythonSmartPointer>::iterator* begin_iterator;
    Roulette<PythonSmartPointer>::iterator* end_iterator;

}PyRouletteIterator;

static PyTypeObject RouletteIteratorType = { PyVarObject_HEAD_INIT(NULL, 0) };

static void rlt_roulette_iterator_dealloc(PyRouletteIterator *self);
static PyObject* rlt_roulette_iterator_new(PyTypeObject *type, PyObject *args, PyObject *kwds);
static int rlt_roulette_iterator_init(PyRouletteIterator *self, PyObject *args, PyObject *kwds);
static PyObject* rlt_roulette_iterator_next (PyRouletteIterator * self);

//--------------------------- PyRouletteIterator ---------------------------//

/********************************************************** type decleration **********************************************************/

/********************************************************** roulette type **********************************************************/

static void rlt_roulette_dealloc(PyRoulette *self)
{
    delete self->roulette_handler;
    Py_TYPE(self)->tp_free((PyObject *) self);
}


static PyObject* rlt_roulette_new(PyTypeObject *type, PyObject *args, PyObject *kwds){

    PyRoulette *self;
    if(!(self = (PyRoulette *) type->tp_alloc(type, 0)))
        return NULL;
    
    self->roulette_handler = new Roulette<PythonSmartPointer>();

    return (PyObject *)self;
}

static PyObject * rlt_roulette_insert(PyRoulette *self, PyObject *args)
{
    PyObject* object;
    double chance;

    if(!PyArg_ParseTuple(args, "Od", &object, &chance)) {
        return NULL;
    }

    PythonSmartPointer ptr(object);
    self->roulette_handler->insert(ptr, chance);

    Py_RETURN_NONE;
}

static int rlt_roulette_init(PyRoulette *self, PyObject *args, PyObject *kwds){
    static char chance_list_str[] = "chance_list";
    static char *kwlist[] = {chance_list_str, NULL};
    PyObject* chance_list = NULL, *iterator = NULL, *item = NULL;


    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|O", kwlist, &chance_list))
        return -1;

    if(chance_list){

        if(!(iterator = PyObject_GetIter(chance_list))){
            return -1;
        }

        while ((item = PyIter_Next(iterator)))
        {
            if(!PyTuple_Check(item)){
                Py_DECREF(item);    
                Py_DECREF(iterator);
                PyErr_Format(PyExc_TypeError, "item not a tuple of an object and float");
                return -1;
            }

            if(!rlt_roulette_insert(self, item)) {
                Py_DECREF(item);    
                Py_DECREF(iterator);
                return -1;
            }

            Py_DECREF(item);
        }

        Py_DECREF(iterator);
    }

    if (PyErr_Occurred())
        return -1;
            
    return 0;
}   

static Py_ssize_t rlt_roulette_len(PyRoulette *self){
    return self->roulette_handler->size();
}

static PyObject * rlt_roulette_roll(PyRoulette *self, PyObject *Py_UNUSED(ignored))
{   
    PythonSmartPointer ret_val;
    try{
         ret_val = self->roulette_handler->roll();
    }catch(...){
        RLT_PRINT_LINE("an exception was thrown");
        return NULL;
    }
    Py_INCREF(ret_val);
    return (PyObject*)ret_val;
}

static PyObject * rlt_roulette_remove(PyRoulette *self, PyObject *args)
{
    PyObject* object;

    if(!PyArg_ParseTuple(args, "O", &object)) {
        return NULL;
    }

    PythonSmartPointer ptr(object);
    
    if(self->roulette_handler->remove(ptr))
        Py_RETURN_TRUE;

    Py_RETURN_FALSE;
}

static PyObject* rlt_roulette_update(PyRoulette *self, PyObject *args){

    PyObject* object;
    double new_chance;

    if(!PyArg_ParseTuple(args, "Od", &object, &new_chance)) {
        return NULL;
    }

    PythonSmartPointer ptr(object);

    if(self->roulette_handler->update(ptr, new_chance))
        Py_RETURN_TRUE;

    Py_RETURN_FALSE;
}

static PyObject* rlt_roulette_iterator(PyRoulette* self){

    PyObject *args = NULL, *kwds = NULL, *iter = NULL;
    bool complete = false;

    do{
        if( !(args = Py_BuildValue("(O)", self)))
            break;

        if( !(kwds = Py_BuildValue("{}")))
            break;

        if(!(iter = rlt_roulette_iterator_new(&RouletteIteratorType, NULL, NULL)))
            break;

        if(rlt_roulette_iterator_init((PyRouletteIterator*)iter, args, kwds) < 0)
            break;

        complete = true;
    }while(0);

    if(!complete){
        if(args)
            Py_DECREF(args);

        if(kwds)
            Py_DECREF(kwds);

        if(iter)
            Py_DECREF(iter);
    }

    return iter;
}

static PyMethodDef rlt_roulette_methods[] = {
    {"insert", (PyCFunction) rlt_roulette_insert, METH_VARARGS, "inserts a python element into the roulette"},
    {"roll", (PyCFunction) rlt_roulette_roll, METH_NOARGS, "randomly choses an element and returns it"},
    {"remove", (PyCFunction) rlt_roulette_remove, METH_VARARGS, "removes a python element from roulette"},
    {"update", (PyCFunction) rlt_roulette_update, METH_VARARGS, "updates element chance in roulette"},
    {NULL, NULL, 0, NULL}  /* Sentinel */
};

PyTypeObject* rlt_init_roulette_type(bool init){
    
    if(init){
        RouletteTypeSequenceMethods.sq_length = (lenfunc) rlt_roulette_len;

        RouletteType.tp_name = "roulette.roulette";
        RouletteType.tp_basicsize = sizeof(PyRoulette);
        RouletteType.tp_itemsize = 0;
        RouletteType.tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE;
        RouletteType.tp_doc = "roulette object";
        RouletteType.tp_new = rlt_roulette_new;
        RouletteType.tp_init = (initproc)rlt_roulette_init;
        RouletteType.tp_dealloc = (destructor) rlt_roulette_dealloc;
        RouletteType.tp_iter = (getiterfunc)rlt_roulette_iterator;
        RouletteType.tp_methods = rlt_roulette_methods;
        RouletteType.tp_as_sequence = &RouletteTypeSequenceMethods;
    }

    return &RouletteType;
}

/********************************************************** roulette type **********************************************************/

/********************************************************** roulette iterator **********************************************************/

static void rlt_roulette_iterator_dealloc(PyRouletteIterator *self){

    delete self->begin_iterator;
    delete self->end_iterator;
    Py_TYPE(self)->tp_free((PyObject *) self);
}

static PyObject* rlt_roulette_iterator_new(PyTypeObject *type, PyObject *args, PyObject *kwds){

    PyRouletteIterator *self;
    if(!(self = (PyRouletteIterator *) type->tp_alloc(type, 0)))
        return NULL;
    
    self->begin_iterator = new Roulette<PythonSmartPointer>::iterator;
    self->end_iterator = new Roulette<PythonSmartPointer>::iterator;

    return (PyObject *)self;
}

static int rlt_roulette_iterator_init(PyRouletteIterator *self, PyObject *args, PyObject *kwds){
    static char roulette_str[] = "roulette";
    static char *kwlist[] = {roulette_str, NULL};
    PyObject* py_roulette = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O|", kwlist, &py_roulette))
        return -1;

    if(!PyObject_TypeCheck(py_roulette, &RouletteType)){
        PyErr_Format(PyExc_TypeError, "expecting object of type roulette");
        return -1;
    }

    *(self->begin_iterator) = ((PyRoulette*)py_roulette)->roulette_handler->begin();
    *(self->end_iterator) = ((PyRoulette*)py_roulette)->roulette_handler->end();

    return 0;
}   

static PyObject* rlt_roulette_iterator_next (PyRouletteIterator * self){

    PyObject* ret_val = NULL;

    if(*(self->begin_iterator) == *(self->end_iterator)){
        PyErr_Format(PyExc_StopIteration, "");
        return NULL;
    }

    if(!(ret_val = Py_BuildValue("(Od)",(PyObject*)(*(self->begin_iterator))->get_value(), (*(self->begin_iterator))->get_max() - (*(self->begin_iterator))->get_min())))
        return NULL;

    ++(*(self->begin_iterator));
    Py_INCREF((PyObject*)(*(self->begin_iterator))->get_value());

    return ret_val;

}

static PyMethodDef rlt_roulette_iterator_methods[] = {
    {NULL, NULL, 0, NULL}  /* Sentinel */
};

PyTypeObject* rlt_init_roulette_iterator_type(bool init){
    
    if(init){
        RouletteIteratorType.tp_name = "roulette.rlt_iter";
        RouletteIteratorType.tp_basicsize = sizeof(PyRouletteIterator);
        RouletteIteratorType.tp_itemsize = 0;
        RouletteIteratorType.tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE;
        RouletteIteratorType.tp_doc = "roulette iterator object";
        RouletteIteratorType.tp_new = rlt_roulette_iterator_new;
        RouletteIteratorType.tp_init = (initproc)rlt_roulette_iterator_init;
        RouletteIteratorType.tp_dealloc = (destructor) rlt_roulette_iterator_dealloc;
        RouletteIteratorType.tp_methods = rlt_roulette_iterator_methods;
        RouletteIteratorType.tp_iternext = (iternextfunc)rlt_roulette_iterator_next;
    }

    return &RouletteIteratorType;
}

/********************************************************** roulette iterator **********************************************************/

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
    PyTypeObject* roulette_iterator_type = NULL;

    bool complete = false;
    do{
        //ready new type
        if (PyType_Ready(rlt_init_roulette_type(true)) < 0)
            break;

        //ready new type
        if (PyType_Ready(rlt_init_roulette_iterator_type(true)) < 0)
            break;

        //ready module
        if (!(module = PyModule_Create(&roulette_module)))
            break;

        roullete_type = rlt_init_roulette_type(false);
        Py_INCREF(roullete_type);

        if (PyModule_AddObject(module, "roulette", (PyObject *) roullete_type) < 0)
            break;

        roulette_iterator_type = rlt_init_roulette_iterator_type(false);
        Py_INCREF(roulette_iterator_type);

        if (PyModule_AddObject(module, "rlt_iter", (PyObject *) roulette_iterator_type) < 0)
            break;
        
        complete = true;

    }while(0);

    if(!complete){
        if(roullete_type){
            Py_DECREF(roullete_type);
            roullete_type = NULL;
        }

        if(roulette_iterator_type){
            Py_DECREF(roulette_iterator_type);
            roulette_iterator_type = NULL;
        }

        if(module){
            Py_DECREF(module);
            module = NULL;
        }
    }

    return module;
}

