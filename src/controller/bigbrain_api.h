/*
 * JSONSimulatorAPI.h
 *
 *  Created on: Sep 10, 2012
 *      Author: blakely
 */

#ifndef SRC_CONTROLLER_BIGBRAIN_API_H_
#define SRC_CONTROLLER_BIGBRAIN_API_H_

#include <Python.h>

#include <base_types.h>
#include <string>

#include "controller/apilayer.h"

namespace bigbrain {

class BigBrain {
 public:
  BigBrain();
  virtual ~BigBrain();
  virtual void TearDown() = 0;
  virtual void XLProcessAgents() = 0;

  static void Update();

 protected:
  void SetSigHandlers();
};

extern BigBrain* g_bbapi_singleton;

template<class T = void>
class BigBrainAPI : public BigBrain {
 public:
  explicit BigBrainAPI(T* object);
  virtual ~BigBrainAPI();
  void Initialize(std::string name = "__AUTO__", std::string port = "__AUTO__",
                  std::string srv_host = "__AUTO__", std::string srv_port =
                      "__AUTO__");
  void Run(bool blocking);
  bool Running();

  virtual void XLProcessAgents();
  virtual void TearDown();
 private:
  T* obj_;
  std::string name_;
  std::string port_;
  std::string srv_host_;
  std::string srv_port_;
  bool blocking_;
  PyObject* file_;
  PyObject* module_;
  bool shutdown_;
};

template<class T> BigBrainAPI<T>::BigBrainAPI(T* object = NULL) {
  obj_ = object;
  blocking_ = false;
  g_bbapi_singleton = this;
  file_ = NULL;
  module_ = NULL;
  name_ = "__AUTO__";
  port_ = "__AUTO__";
  srv_host_ = "__AUTO__";
  srv_port_ = "__AUTO__";
  shutdown_ = false;
}

template<class T> BigBrainAPI<T>::~BigBrainAPI() {
}

template<class T> void BigBrainAPI<T>::Initialize(std::string name,
                                                  std::string port,
                                                  std::string srv_host,
                                                  std::string srv_port) {
  name_ = name;
  port_ = port;
  srv_host_ = srv_host;
  srv_port_ = srv_port;
  Py_InitializeEx(0);
  PyRun_SimpleString("import sys");
  PyRun_SimpleString("sys.path.append(\".\")");
  // Inject the apilayer Cython module before we start the server
  initapilayer();
  // Now, import the python code that calls it
  file_ = PyString_FromString("bigbrain_server");
  module_ = PyImport_Import(file_);
  if (!module_) {
    PyErr_Print();
  }
  if (obj_) {
    SetRPCObject(obj_);
  }
}

template<class T> void BigBrainAPI<T>::Run(bool blocking = false) {
  blocking_ = blocking;
  StartWebserver(name_, port_, srv_host_, srv_port_, blocking_);
  PyEval_SaveThread();
  SetSigHandlers();
}

template<class T> bool BigBrainAPI<T>::Running() {
  if (!shutdown_) {
    return WebserverRunning();
  }
  return false;
}

template<class T> void BigBrainAPI<T>::TearDown() {
  if (shutdown_) {
    return;
  }
  printf("Shutting down BigBrain Interface\n");
  PyGILState_Ensure();
  StopWebserver();
  Py_DECREF(module_);
  Py_DECREF(file_);
  Py_Finalize();
  shutdown_ = true;
}

template<class T> void BigBrainAPI<T>::XLProcessAgents() {
  BBProcessAgents();
}

}  // namespace bigbrain
#endif  // SRC_CONTROLLER_BIGBRAIN_API_H_
