from libcpp.vector cimport vector
from libcpp.string cimport string

cdef extern from "controller/simulator.h" namespace "bigbrain":
  cdef cppclass Simulator:
    void RunTimestep(double t, double dt) nogil
    void GenerateNetwork(string description) nogil
    void RunUntil(double t, double dt) nogil
    vector[unsigned long int] GetNeuronUUIDs() nogil
    vector[double] GetMembranePotentials() nogil
    vector[double] GetCellPosition(unsigned long int uuid) nogil
    vector[unsigned long int] GetCellConnections(unsigned long int uuid) nogil
    vector[string] GetCellParamNames(unsigned long int uuid) nogil
    vector[string] GetCellParamValues(unsigned long int uuid) nogil
    vector[string] GetCellStateNames(unsigned long int uuid) nogil
    vector[string] GetCellStateValues(unsigned long int uuid) nogil
  
