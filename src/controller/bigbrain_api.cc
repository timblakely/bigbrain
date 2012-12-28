// This needs to be included first to fix some weirdness with Cython

#include <Python.h>

#include <signal.h>
#include <stddef.h>

#include "controller/bigbrain_api.h"

namespace bigbrain {

static struct sigaction new_sa, old_sa;

BigBrain* g_bbapi_singleton = NULL;

BigBrain::BigBrain() {
}

BigBrain::~BigBrain() {
}

void sighandler(int signo) {
  if (g_bbapi_singleton) {
    g_bbapi_singleton->TearDown();
  }
  if (old_sa.sa_handler) {
    (*old_sa.sa_handler)(signo);
  }
}

void BigBrain::Update() {
  if (g_bbapi_singleton) {
    g_bbapi_singleton->XLProcessAgents();
  }
}

void BigBrain::SetSigHandlers() {
  new_sa.sa_handler = sighandler;
  sigaction(SIGINT, &new_sa, &old_sa);
}

} // namespace bigbrain
