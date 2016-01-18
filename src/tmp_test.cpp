#include <python2.7/Python.h>
#include <stdio.h>
int main() {
  PyObject* pMod = NULL;
  PyObject* pFunc = NULL;
  PyObject* pParm = NULL;
  PyObject* pRetVal = NULL;
  PyObject* pDict = NULL;
  Py_Initialize();
  PyRun_SimpleString(
      "from time import time,ctime\n"
      "print 'Today is',ctime(time())\n"
      "from ocrus.neural_network.network_test import testPython\n"
      "testPython((2,2))");

  int retVal = 0;
  //PyObject* pName = PyString_FromString("ocrus.neural_network.network_test");
  pMod = PyImport_ImportModule("ocrus.neural_network.network_test");
  //Py_DECREF(pName);
  pDict = PyModule_GetDict(pMod);
  pFunc = PyDict_GetItemString(pDict, "testPython");
  if (!pFunc)
    exit(-2);
  if (PyCallable_Check(pFunc)) {
    PyObject* pParm_tuple = PyTuple_New(3);
    for (int i = 0; i < 3; i++) {
      PyObject* pValue = Py_BuildValue("f", 2.5);
      if (!pValue) {
        PyErr_Print();
        return 1;
      }
      PyTuple_SetItem(pParm_tuple, i, pValue);
    }
    PyObject*ret_list = PyObject_CallObject(pFunc, pParm_tuple);
    //printf("here3");
    for (int i = 0; i < PyTuple_GET_SIZE(ret_list); i++)
    {
        PyObject* value = PyTuple_GetItem(ret_list, i);
        float valueint = 0;
        PyArg_Parse(value, "f", &valueint);
        printf("%f", valueint);
    }
    //printf("%d", retVal);
  }
  Py_Finalize();
  return 0;
}
