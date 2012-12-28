import bigbrain_server
from libcpp cimport bool
from libcpp.string cimport string
from libcpp.vector cimport vector

cimport cython
cimport interface

# PYTHON -> C  

# Simulator 

cdef class BBClass_Simulator(object):
  cdef interface.Simulator * _sim
  cdef SetObject(self, void *sim):
    self._sim = <interface.Simulator *>sim
  
  ######################################  
  # RunTimestep
  def Params_RunTimestep(self):
    return {
        't': 'float',
        'dt': 'float'
    }
  @cython.always_allow_keywords(True)
  def Call_RunTimestep(self, t, dt):
    self._sim.RunTimestep(t, dt)
    
  ######################################
  # RunUntil
  def Params_RunUntil(self):
    return {
        't': 'float',
        'dt': 'float'
    }
  @cython.always_allow_keywords(True)
  def Call_RunUntil(self, t, dt):
    pt = <double>t
    pdt = <double>dt
    with nogil:
      self._sim.RunUntil(pt, pdt)
    
  ######################################
  # GenerateNetwork
  def Params_GenerateNetwork(self):
    return {
        'description': 'string'
    }
  @cython.always_allow_keywords(True)
  def Call_GenerateNetwork(self, description):
    self._sim.GenerateNetwork(description)
    
  ######################################
  # GetNeuronUUIDs
  def Params_GetNeuronUUIDs(self):
    return {
    }
  @cython.always_allow_keywords(True)
  def Call_GetNeuronUUIDs(self):
    return self._sim.GetNeuronUUIDs()
  
  ######################################
  # GetMembranePotentials
  def Params_GetMembranePotentials(self):
    return {
    }
  @cython.always_allow_keywords(True)
  def Call_GetMembranePotentials(self):
    return self._sim.GetMembranePotentials()

  ######################################
  # GetCellPosition
  def Params_GetCellPosition(self):
    return {
        'uuid':'int'
    }
  @cython.always_allow_keywords(True)
  def Call_GetCellPosition(self, uuid):
    return self._sim.GetCellPosition(uuid)
  
  ######################################
  # GetCellConnections
  def Params_GetCellConnections(self):
    return {
       'uuid':'int'
    }
  @cython.always_allow_keywords(True)
  def Call_GetCellConnections(self, uuid):
    return self._sim.GetCellConnections(uuid) 
  
  ######################################
  # GetCellConnections
  def Params_GetCellConnections(self):
    return {
       'uuid':'int'
    }
  @cython.always_allow_keywords(True)
  def Call_GetCellConnections(self, uuid):
    return self._sim.GetCellConnections(uuid) 
  
  ######################################
  # GetCellParamNames
  def Params_GetCellParamNames(self):
    return {
       'uuid':'int'
    }
  @cython.always_allow_keywords(True)
  def Call_GetCellParamNames(self, uuid):
    return self._sim.GetCellParamNames(uuid) 
  
  ######################################
  # GetCellParamValues
  def Params_GetCellParamValues(self):
    return {
       'uuid':'int'
    }
  @cython.always_allow_keywords(True)
  def Call_GetCellParamValues(self, uuid):
    return self._sim.GetCellParamValues(uuid) 
  
  ######################################
  # GetCellParamNames
  def Params_GetCellStateNames(self):
    return {
       'uuid':'int'
    }
  @cython.always_allow_keywords(True)
  def Call_GetCellStateNames(self, uuid):
    return self._sim.GetCellStateNames(uuid) 
  
  ######################################
  # GetCellParamValues
  def Params_GetCellStateValues(self):
    return {
       'uuid':'int'
    }
  @cython.always_allow_keywords(True)
  def Call_GetCellStateValues(self, uuid):
    return self._sim.GetCellStateValues(uuid) 




# C -> PYTHON



quickhack = None
client = None
client = bigbrain_server.BigBrainClientSideServer()
client.Initialize()
bbclass = client.GetBBClass()


from libcpp.string cimport string

cdef public void StartWebserver(string name, string port, string srv_host, 
                                string srv_port, bool blocking):
  global client
  client.SetInstanceName(name)
  client.SetInstancePort(port)
  client.SetSrvHost(srv_host)
  client.SetSrvPort(srv_port)
  if not blocking:
    client.start()
  else:
    client.run()
  
cdef public void StopWebserver() with gil:
  client.Shutdown()

cdef public int WebserverRunning() with gil:
  return client.IsRunning()

cdef public void BBProcessAgents() with gil:
  client.ProcessAgents()
  
cdef public void SetRPCObject(void *sim):
  BBClass_Simulator.SetObject(bbclass, sim)
  
