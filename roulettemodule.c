#include <Python.h>
#include "roulette_module.hpp"

/*this library contains a c extension implementation of the roulette object*/

/*roulette structure*/
typedef struct {
    PyObject_HEAD
    void* roulette;
} PyRoulette;

/*roulette dealloc function*/
/*free's the memory and deallocate's the roulette object*/
static void
PyRoulette_dealloc(PyRoulette* self)
{
    void* iterator;
    PyObject* obj;
    if(self->roulette != NULL){/*no real reason not to be able to enter here*/
        /*allocate iterator, maybe need to check if memory allocated for iterator
        ,but if memory didn't allocate it is bad, because then you cannot decrease reference counter*/
        iterator = roulette_init_iterator(self->roulette);
        /*foreach PyObject inserted decrease reference counter*/
        while(roulette_iterator_hasNext(self->roulette,iterator)){

            obj = (PyObject*)roulette_deref_iterator(iterator);
            /*PyObject_Print(obj,stdout,Py_PRINT_RAW);
            printf("\n");*/
            Py_XDECREF(obj);
            roulette_iterator_adv(iterator);
        }

        //printf("is roulette empty? %d\n",roulette_is_empty(self->roulette));

        /*free iterator*/
        roulette_free_iterator(iterator);

        /*free roulette object*/
        roulette_free(self->roulette);
    }

    /*free PyRoulette*/
    Py_TYPE(self)->tp_free((PyObject*)self);
}

/*roulette new function*/
/*creates a new roulette object, no parameters are needed*/
static PyObject *
PyRoulette_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{

    PyRoulette *self;
    /*allocate self*/
    self = (PyRoulette *)type->tp_alloc(type, 0);
    if (self == NULL) {
        PyErr_SetString(PyExc_MemoryError, "couldn't allocate PyRoulette");
        return NULL;
    }

    /*allocate the c++ object*/
    self->roulette = roulette_init();
    if(self->roulette == NULL){
        Py_DECREF(self);
        PyErr_SetString(PyExc_MemoryError, "couldn't allocate PyRoulette");
        return NULL;
        }

    return (PyObject *)self;
}

/*returns the amount of objects stored currently in roulette
**NOTE: maybe should change it from a simple function to len()*/
static PyObject *
PyRoulette_getSize(PyRoulette* self)
{
    return PyInt_FromSize_t(roulette_size(self->roulette));
}

/*inserts new PyObject into the roulette with a chance(chance must be >0) to roll it
, and increases it's reference counter*/
static PyObject*
PyRoulette_insert(PyRoulette* self, PyObject* args,PyObject *kwds){

    PyObject *obj=NULL;
    double chance;

    static char *kwlist[] = {"object", "chance", NULL};
    /*parse PyObject to insert, and chance of throwing it*/
    if (! PyArg_ParseTupleAndKeywords(args, kwds, "Od", kwlist,&obj, &chance)){
        PyErr_SetString(PyExc_TypeError, "wrong values, needs obj,chance(double)");
        return NULL;
    }
    /*chance must be >0*/
    if(chance <= 0){
        PyErr_SetString(PyExc_ValueError, "chance must be greater than 0");
        return NULL;
    }

    /*if obj != NULL, increase reference counter and insert to roulette object*/
     if (obj) {
        Py_XINCREF(obj);
        roulette_insert(self->roulette,obj,chance);
    }else{
        PyErr_SetString(PyExc_ValueError, "must receive obj");
        return NULL;
    }

    Py_RETURN_NONE;
}

/*randomly chooses a value while considering each object weight(chance) */
static PyObject *
PyRoulette_roll(PyRoulette* self)
{
    /*if roulette is empty, cannot choose object*/
    if(roulette_size(self->roulette) == 0){
        PyErr_SetString(PyExc_StandardError, "cannot roll an empty roulette");
        return NULL;
    }

    /*chose a random object*/
    void* roll_obj = (PyObject *)roulette_roll(self->roulette);

    if(roll_obj == NULL){
        printf("error = %s\n",roulette_err_msg);
        PyErr_SetString(PyExc_StandardError, roulette_err_msg);
    }

    return roll_obj;
}

static PyObject *
PyRoulette_printValues(PyRoulette* self){

    PyObject* obj;
    void* iterator;
    iterator = roulette_init_iterator(self->roulette);
    /*foreach PyObject inserted decrease reference counter*/
    while(roulette_iterator_hasNext(self->roulette,iterator)){

        obj = (PyObject*)roulette_deref_iterator(iterator);
        PyObject_Print(obj,stdout,Py_PRINT_RAW);
        printf("\n");
        roulette_iterator_adv(iterator);
    }
    /*free iterator*/
    roulette_free_iterator(iterator);

    Py_RETURN_NONE;
}

/*PyRoulette methods*/
static PyMethodDef roulette_methods[] = {
    {"objectAmount", (PyCFunction)PyRoulette_getSize, METH_NOARGS,"return the amount of objects stored in the roulette"},
    {"insert", (PyCFunction)PyRoulette_insert, METH_KEYWORDS,"insert new value"},
    {"roll", (PyCFunction)PyRoulette_roll, METH_NOARGS,"rolls a value from the roulette"},
    {"printValues", (PyCFunction)PyRoulette_printValues, METH_NOARGS,"prints all stored values"},
    {NULL}  /* Sentinel */
};

/*PyRoulette type description*/
static PyTypeObject PyRouletteType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "hello.Roulette",             /* tp_name */
    sizeof(PyRoulette), /* tp_basicsize */
    0,                         /* tp_itemsize */
    (destructor)PyRoulette_dealloc,                         /* tp_dealloc */
    0,                         /* tp_print */
    0,                         /* tp_getattr */
    0,                         /* tp_setattr */
    0,                         /* tp_compare */
    0,                         /* tp_repr */
    0,                         /* tp_as_number */
    0,                         /* tp_as_sequence */
    0,                         /* tp_as_mapping */
    0,                         /* tp_hash */
    0,                         /* tp_call */
    0,                         /* tp_str */
    0,                         /* tp_getattro */
    0,                         /* tp_setattro */
    0,                         /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT ,        /* tp_flags */
    "Roulette Object, allows to give each object a different chance to be chosen",           /* tp_doc */
    0,                         /* tp_traverse */
    0,                         /* tp_clear */
    0,                         /* tp_richcompare */
    0,                         /* tp_weaklistoffset */
    0,                         /* tp_iter */
    0,                         /* tp_iternext */
    roulette_methods,          /* tp_methods */
    0,                         /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    0,                         /* tp_init */
    0,                         /* tp_alloc */
    PyRoulette_new,                 /* tp_new */
};

/*module methods none*/
static PyMethodDef roulette_module_methods[] = {
    {NULL}  /* Sentinel */
};

#ifndef PyMODINIT_FUNC	/* declarations for DLL import/export */
#define PyMODINIT_FUNC void
#endif

/*init the module*/
PyMODINIT_FUNC
initroulette(void)
{
    PyObject* m;

    /*check if new type is ready*/
    if (PyType_Ready(&PyRouletteType) < 0)
        return;

    /*init the module methods*/
    m = Py_InitModule3("roulette", roulette_module_methods,
                       "a Roulette object to random choose.");

    /*check if module methods initialized properly*/
    if(m == NULL)
        return;

    /*add new type to the global python types*/
    Py_INCREF(&PyRouletteType);
    PyModule_AddObject(m, "Roulette", (PyObject *)&PyRouletteType);
}