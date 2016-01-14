#include <python2.7/Python.h>
#include <stdio.h>
int main()
{
    PyObject* pMod = NULL;
    PyObject* pFunc = NULL;
    PyObject* pParm = NULL;
    PyObject* pRetVal = NULL;
    PyObject* pDict = NULL;
    Py_Initialize();
    PyRun_SimpleString("from time import time,ctime\n"
                         "print 'Today is',ctime(time())\n"
                         "from ocrus.neural_network.network_test import testPython\n"
                         "testPython(2)"  );

    int retVal = 0;
    //PyObject* pName = PyString_FromString("ocrus.neural_network.network_test");
    pMod =  PyImport_ImportModule("ocrus.neural_network.network_test");
    //Py_DECREF(pName);
    pFunc = PyObject_GetAttrString(pMod, "testPython");
    if (!pFunc)
       exit(-1);
    PyObject* list = PyList_New(0);
    //for (size_t i = 0; i < 3; i++)
    //  PyList_Append(list, Py_BuildValue("i", i));
    pParm = Py_BuildValue("(i)", 5);
    pRetVal = PyEval_CallObject(pFunc, pParm);
    PyArg_Parse(pRetVal, "i", &retVal);
    printf("%d", retVal);
    Py_Finalize();
    return 0;
}
