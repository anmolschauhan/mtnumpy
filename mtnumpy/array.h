#pragma once
#include <Python.h>
#include "structmember.h"
#include <vector>
#include "mtmath.h"
#include "mtmath.cpp"
#include <functional>

typedef struct {
    PyObject_HEAD
    void* array;
    long long size;
    std::string dtype;
} ArrayObject;

static void
Custom_dealloc(ArrayObject* self)
{
    free(self->array);
    Py_TYPE(self)->tp_free((PyObject*)self);
}

static PyObject*
Custom_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
    ArrayObject* self;
    self = (ArrayObject*)type->tp_alloc(type, 0);
    if (self != NULL) {
        self->dtype.reserve(7);
    }
    return (PyObject*)self;
}

template<typename T>
void PyList_ToArray(T* array, PyObject* list, std::function<T(PyObject*)> py_to_c)
{
    PyObject* iterator = PyObject_GetIter(list);
    PyObject* item;

    if (iterator == NULL) {
        /* propagate error */
    }

    long long i = 0;
    while ((item = PyIter_Next(iterator))) {
        double val = py_to_c(item);
        array[i] = val;
        Py_DECREF(item);
        i++;
    }

    Py_DECREF(iterator);
}

static int
Custom_init(ArrayObject* self, PyObject* args, PyObject* kwds)
{
    static char* kwlist[] = { "array", "dtype", NULL };
    PyObject *list = NULL, *dtype = NULL;


    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|OO", kwlist,
        &list, &dtype))
        return -1;

    if (dtype)
        self->dtype = PyBytes_AsString(PyUnicode_AsEncodedString(dtype, "UTF-8", "strict"));
    else
        self->dtype = "float64";

    if (list) {
        long long len = PyObject_Size(list);
        
        if (self->dtype == "int32")
        {
            self->array = malloc(len * sizeof(int));
            int* array = (int*)self->array;
            PyList_ToArray<int>(array, list, PyLong_AsLong);
        }
        else if (self->dtype == "int64")
        {
            self->array = malloc(len * sizeof(long long));
            long long* array = (long long*)self->array;
            PyList_ToArray<long long>(array, list, PyLong_AsLongLong);
        }
        else if (self->dtype == "uint32")
        {
            self->array = malloc(len * sizeof(unsigned int));
            unsigned int* array = (unsigned int*)self->array;
            PyList_ToArray<unsigned int>(array, list, PyLong_AsUnsignedLong);
        }
        else if (self->dtype == "uint64")
        {
            self->array = malloc(len * sizeof(unsigned long long));
            unsigned long long* array = (unsigned long long*)self->array;
            PyList_ToArray<unsigned long long>(array, list, PyLong_AsUnsignedLongLong);
        }
        else if (self->dtype == "float32")
        {
            self->array = malloc(len * sizeof(float));
            float* array = (float*)self->array;
            PyList_ToArray<float>(array, list, PyFloat_AsDouble);
        }
        else if (self->dtype == "float64")
        {
            self->array = malloc(len * sizeof(double));
            double* array = (double*)self->array;
            PyList_ToArray<double>(array, list, PyFloat_AsDouble);
        }

        self->size = len;

    }
    return 0;
}

static PyMemberDef Custom_members[] = {
    {"array", T_OBJECT_EX, offsetof(ArrayObject, array), 0,
     "underlying array"},
    {NULL}  /* Sentinel */
};

static PyObject*
Array_sum(ArrayObject* self, PyObject* args);

static PyObject*
Array_product(ArrayObject* self, PyObject* args);

static PyMethodDef Custom_methods[] = {
    {"sum", (PyCFunction)Array_sum, METH_VARARGS,
     "Return the sum of array"
    },   
    {"product", (PyCFunction)Array_product, METH_VARARGS,
     "Return the sum of array"
    },
    {NULL}  /* Sentinel */
};

static PyMemberDef Array_members[] = {
    {"dtype", T_STRING, offsetof(ArrayObject, dtype), 0,
     "dtype"},
    {NULL}  /* Sentinel */
};

static PyTypeObject CustomType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "mtnumpy.array",                                    /* tp_name */
    sizeof(ArrayObject),                                /* tp_basicsize */
    0,                                                  /* tp_itemsize */
    (destructor)Custom_dealloc,                         /* tp_dealloc */
    0,                                                  /* tp_vectorcall_offset */
    0,                                                  /* tp_getattr */
    0,                                                  /* tp_setattr */
    0,                                                  /* tp_as_async */
    0,                                                  /* tp_repr */
    0,                                                  /* tp_as_number */
    0,                                                  /* tp_as_sequence */
    0,                                                  /* tp_as_mapping */
    0,                                                  /* tp_hash */
    0,                                                  /* tp_call */
    0,                                                  /* tp_str */
    0,                                                  /* tp_getattro */
    0,                                                  /* tp_setattro */
    0,                                                  /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,           /* tp_flags */
    "multi threaded numpy clone objects",               /* tp_doc */
    0,                                                  /* tp_traverse */
    0,                                                  /* tp_clear */
    0,                                                  /* tp_richcompare */
    0,                                                  /* tp_weaklistoffset */
    0,                                                  /* tp_iter */
    0,                                                  /* tp_iternext */
    Custom_methods,                                     /* tp_methods */
    Custom_members,                                     /* tp_members */
    0,                                                  /* tp_getset */
    0,                                                  /* tp_base */
    0,                                                  /* tp_dict */
    0,                                                  /* tp_descr_get */
    0,                                                  /* tp_descr_set */
    0,                                                  /* tp_dictoffset */
    (initproc)Custom_init,                              /* tp_init */
    0,                                                  /* tp_alloc */
    Custom_new,                                         /* tp_new */
};

template <typename T>
struct Add { T operator()(T a, T b) { return a + b; } };

template <typename T>
struct Product { T operator()(T a, T b) { return a * b; } };

template<typename T>
static PyObject*
array_agg_helper(ArrayObject* self, PyObject* other, double init, 
                 std::function<PyObject*(T)> c_to_py,
                 std::function<T(T, T)> Op)
{
    if (other == NULL)
    {
        T sum_result = mtagg_helper<T>((T*)self->array, self->size, 24, init, Op);
        return c_to_py(sum_result);
    }
    else if (PyFloat_Check(other) || PyLong_Check(other))
    {
        T other_double = PyFloat_AsDouble(other);
        ArrayObject* tmp = (ArrayObject*)Custom_new(&CustomType, NULL, NULL);
        tmp->array = (T*)malloc(self->size * sizeof(T));
        tmp->size = self->size;
        tmp->dtype = self->dtype;
        mtagg_helper<T>((T*)tmp->array, (T*)self->array, self->size, other_double, 24, Op);
        return (PyObject*)tmp;
    }
    else if ((PyTypeObject*)other->ob_type == &CustomType)
    {
        // add code to make it return ArrayObject
        return Py_None;
    }
    return Py_None;
}

template<template<class> typename Op>
static PyObject*
Array_agg(ArrayObject* self, PyObject* other, double init)
{
    if (self->dtype == "int32")
    {
        return array_agg_helper<int>(self, other, init, PyLong_FromLong, Op<int>{});
    }
    else if (self->dtype == "int64")
    {
        return array_agg_helper<long long>(self, other, init, PyLong_FromLong, Op<long long>{});
    }
    else if (self->dtype == "uint32")
    {
        return array_agg_helper<unsigned int>(self, other, init, PyLong_FromUnsignedLong, Op<unsigned int>{});
    }
    else if (self->dtype == "uint64")
    {
        return array_agg_helper<unsigned long long>(self, other, init, PyLong_FromUnsignedLongLong, Op<unsigned long long>{});
    }
    else if (self->dtype == "float32")
    {
        return array_agg_helper<float>(self, other, init, PyFloat_FromDouble, Op<float>{});
    }
    else if (self->dtype == "float64")
    {
        return array_agg_helper<double>(self, other, init, PyFloat_FromDouble, Op<double>{});
    }
}



static PyObject*
Array_sum(ArrayObject* self, PyObject* args)
{
    PyObject* other = NULL;
    PyArg_ParseTuple(args, "|O", &other);
    return Array_agg<Add>(self, other, 0);
}


static PyObject*
Array_product(ArrayObject* self, PyObject* args)
{
    PyObject* other = NULL;
    PyArg_ParseTuple(args, "|O", &other);
    return Array_agg<Product>(self, other, 1);
}