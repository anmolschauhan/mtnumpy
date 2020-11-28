#include <Python.h>
#include "array.h"
#include <vector>

/*
 * Implements an example function.
 */
PyDoc_STRVAR(mtnumpy_example_doc, "example(obj, number)\
\
Example function");

static PyObject*
mtnumpy_random(PyObject* self, PyObject* args) {
    long long n;
    PyArg_ParseTuple(args, "|L", &n);
    ArrayObject* tmp = (ArrayObject*)Custom_new(&CustomType, NULL, NULL);
    tmp->size = n;
    tmp->array = malloc(n * sizeof(double));
    tmp->dtype = "float64";
    mtrandom_helper<double>((double*)(tmp->array), tmp->size, 24);
    return (PyObject*)tmp;
}

/*
 * List of functions to add to mtnumpy in exec_mtnumpy().
 */
static PyMethodDef mtnumpy_functions[] = {
    {"random", (PyCFunction)mtnumpy_random, METH_VARARGS, "Random array"},
    { NULL, NULL, 0, NULL } /* marks end of array */
};

PyObject* AddArray(PyObject* m)
{
    if (PyType_Ready(&CustomType) < 0) {
        return NULL;
    }
    Py_INCREF(&CustomType);
    if (PyModule_AddObject(m, "array", (PyObject*)&CustomType) < 0) {
        Py_DECREF(&CustomType);
        Py_DECREF(m);
        return NULL;
    }
}

/*
 * Initialize mtnumpy. May be called multiple times, so avoid
 * using static state.
 */
int exec_mtnumpy(PyObject *module) {
    PyModule_AddFunctions(module, mtnumpy_functions);
    AddArray(module);
    PyModule_AddStringConstant(module, "__author__", "anmol");
    PyModule_AddStringConstant(module, "__version__", "1.0.0");
    PyModule_AddIntConstant(module, "year", 2020);

    return 0; /* success */
}


static PyModuleDef_Slot mtnumpy_slots[] = {
    { Py_mod_exec, exec_mtnumpy },
    { 0, NULL }
};

static PyModuleDef mtnumpy_def = {
    PyModuleDef_HEAD_INIT,
    "mtnumpy",
    NULL,           /* doc */
    0,              /* m_size */
    NULL,           /* m_methods */
    mtnumpy_slots,
    NULL,           /* m_traverse */
    NULL,           /* m_clear */
    NULL,           /* m_free */
};

PyMODINIT_FUNC PyInit_mtnumpy() {
    Py_INCREF(&CustomType);
    return PyModuleDef_Init(&mtnumpy_def);
}
