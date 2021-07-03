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
    mutable std::string _obj_string;
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

    PythonSmartPointer& operator=(PyObject* rhs){

        if(_py_object)
            Py_XDECREF(_py_object);
        _py_object = rhs;
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

    PyObject* increase_ref() const{
        
        Py_INCREF(_py_object);
        return _py_object;

    }

    operator std::string()const{
        PyObject* objects_representation = PyObject_Str(_py_object);

        if(!objects_representation)
            return "";

        const char* s = PyUnicode_AsUTF8(objects_representation);

        if(!s)
             _obj_string = "";
        else
            _obj_string = s;

        Py_DECREF(objects_representation);

        return _obj_string.c_str();

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
static PyMappingMethods RouletteTypeMappingMethods;

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
    self->roulette_handler->~Roulette();
    PyMem_RawFree(self->roulette_handler);
    Py_TYPE(self)->tp_free((PyObject *) self);
}


static PyObject* rlt_roulette_new(PyTypeObject *type, PyObject *args, PyObject *kwds){

    PyRoulette *self;

    void* temp_ptr = PyMem_RawMalloc(sizeof(Roulette<PythonSmartPointer>));

    if(!temp_ptr)
        return NULL;

    if(!(self = (PyRoulette *) type->tp_alloc(type, 0))){
        PyMem_RawFree(temp_ptr);
        return NULL;
    }
    
    self->roulette_handler = new(temp_ptr) Roulette<PythonSmartPointer>();

    return (PyObject *)self;
}

static PyObject * rlt_roulette_insert(PyRoulette *self, PyObject *args)
{
    PyObject* object;
    double chance;

    if(!PyArg_ParseTuple(args, "Od", &object, &chance)) {
        return NULL;
    }

    self->roulette_handler->insert(object, chance);

    Py_RETURN_NONE;
}

static PyObject * rlt_roulette_insert_list(PyRoulette *self, PyObject *args)
{
    PyObject* chance_list = NULL, *iterator = NULL, *item = NULL;

    if(!PyArg_ParseTuple(args, "O", &chance_list)) {
        return NULL;
    }

    if(!(iterator = PyObject_GetIter(chance_list))){
        return NULL;
    }

    while ((item = PyIter_Next(iterator)))
    {
        if(!PyTuple_Check(item)){
            Py_DECREF(item);
            Py_DECREF(iterator);
            PyErr_Format(PyExc_TypeError, "item not a tuple of an object and float");
            return NULL;
        }

        if(!rlt_roulette_insert(self, item)) {
            Py_DECREF(item);
            Py_DECREF(iterator);
            return NULL;
        }

        Py_DECREF(item);
    }

    Py_DECREF(iterator);

    Py_RETURN_NONE;
}

static int rlt_roulette_init(PyRoulette *self, PyObject *args, PyObject *kwds){
    static char chance_list_str[] = "chance_list";
    static char *kwlist[] = {chance_list_str, NULL};
    PyObject* chance_list = NULL, *iterator = NULL, *item = NULL, *none_obj = NULL;


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

            if(!(none_obj = rlt_roulette_insert(self, item))) {
                Py_DECREF(item);    
                Py_DECREF(iterator);
                return -1;
            }

            Py_DECREF(none_obj);
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

static PyObject* rlt_roulette_get_item(PyRoulette *self, PyObject *key){

    PythonSmartPointer ptr(key);

    auto iter = self->roulette_handler->find(ptr);

    if(iter == self->roulette_handler->end()){
        PyErr_Format(PyExc_KeyError, "key not found");
        return NULL;
    }

    return PyFloat_FromDouble(iter->get_range());
}

static int rlt_roulette_set_item(PyRoulette *self, PyObject *key, PyObject *value){
    
    double new_chance;
    PythonSmartPointer ptr(key);

    if(NULL == value){ //remove
        if(!self->roulette_handler->remove(ptr)){
            PyErr_Format(PyExc_KeyError, "key not found");
            return -1;
        }
    }else{ //update
        new_chance = PyFloat_AsDouble(value);

        if(PyErr_Occurred())
            return -1;

        if(!self->roulette_handler->update(ptr, new_chance)){
            PyErr_Format(PyExc_KeyError, "key not found");
            return -1;
        }
    }
    return 0;
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
    return ret_val.increase_ref();
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
        if(args){
            Py_DECREF(args);
            Py_DECREF(self);//because Py_BuildValue succeded, self ref count was increased
        }

        if(kwds)
            Py_DECREF(kwds);

        if(iter)
            Py_DECREF(iter);
    }

    return iter;
}

static PyMethodDef rlt_roulette_methods[] = {
    {"insert", (PyCFunction) rlt_roulette_insert, METH_VARARGS, "inserts a python element into the roulette"},
    {"insert_list", (PyCFunction) rlt_roulette_insert_list, METH_VARARGS, "inserts a python sequence of elements into the roulette"},
    {"roll", (PyCFunction) rlt_roulette_roll, METH_NOARGS, "randomly choses an element and returns it"},
    {"remove", (PyCFunction) rlt_roulette_remove, METH_VARARGS, "removes a python element from roulette"},
    {"update", (PyCFunction) rlt_roulette_update, METH_VARARGS, "updates element chance in roulette"},
    {NULL, NULL, 0, NULL}  /* Sentinel */
};

PyTypeObject* rlt_init_roulette_type(bool init){
    
    if(init){
        RouletteTypeMappingMethods.mp_length = (lenfunc) rlt_roulette_len;
        RouletteTypeMappingMethods.mp_subscript = (binaryfunc)rlt_roulette_get_item;
        RouletteTypeMappingMethods.mp_ass_subscript = (objobjargproc)rlt_roulette_set_item;

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
        RouletteType.tp_as_mapping = &RouletteTypeMappingMethods;
    }

    return &RouletteType;
}

/********************************************************** roulette type **********************************************************/

/********************************************************** roulette iterator **********************************************************/

static void rlt_roulette_iterator_dealloc(PyRouletteIterator *self){

    self->begin_iterator->Roulette<PythonSmartPointer>::iterator::~iterator();
    PyMem_RawFree(self->begin_iterator);
    self->end_iterator->Roulette<PythonSmartPointer>::iterator::~iterator();
    PyMem_RawFree(self->end_iterator);
    Py_TYPE(self)->tp_free((PyObject *) self);
}

static PyObject* rlt_roulette_iterator_new(PyTypeObject *type, PyObject *args, PyObject *kwds){

    PyRouletteIterator *self = NULL;
    void* temp_begin = NULL, *temp_end = NULL;
    bool complete = false;
    do{
        
        if(!(temp_begin = PyMem_RawMalloc(sizeof(Roulette<PythonSmartPointer>::iterator))))
            break;

        if(!(temp_end = PyMem_RawMalloc(sizeof(Roulette<PythonSmartPointer>::iterator))))
            break;

        if(!(self = (PyRouletteIterator *) type->tp_alloc(type, 0)))
            break;
        
        complete = true;
        
    }while (0);

    if (!complete)
    {
        if(temp_begin)PyMem_RawFree(temp_begin);

        if(temp_end)PyMem_RawFree(temp_end);

        if(self)Py_TYPE(self)->tp_free((PyObject *) self);

        return NULL;
    }
    
    self->begin_iterator = new(temp_begin) Roulette<PythonSmartPointer>::iterator();
    self->end_iterator = new(temp_end) Roulette<PythonSmartPointer>::iterator();

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

    if(!(ret_val = Py_BuildValue("(Od)",(PyObject*)(*(self->begin_iterator))->get_value(), (*(self->begin_iterator))->get_max() - (*(self->begin_iterator))->get_min()))){
        return NULL;
    }

    ++(*(self->begin_iterator));
    //Py_INCREF((PyObject*)(*(self->begin_iterator))->get_value());


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
    PyModuleDef_HEAD_INIT
};

/********************************************************** roulette module **********************************************************/

PyMODINIT_FUNC PyInit_roulette(void)
{
    PyObject *module = NULL;
    PyTypeObject* roullete_type = NULL;
    PyTypeObject* roulette_iterator_type = NULL;
	
    roulette_module.m_name = "roulette";   /* name of module */
    roulette_module.m_doc = "wighted random chooser module"; /* module documentation, may be NULL */
    roulette_module.m_size = -1;       /* size of per-interpreter state of the module, or -1 if the module keeps state in global variables. */
    roulette_module.m_methods = roulette_methods;

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

