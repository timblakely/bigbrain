import time
from network.cluster_managers import google_compute_cluster_manager as gccm

cm = gccm.GoogleComputeClusterManager()
cm.Initialize('bats','12345','32123')
cm.Heartbeat()

cm.ListInstanceNames()

last = ''

print 'NumInstances: %i' % cm.NumInstances()
for instance in cm.ListInstanceNames():
  print '  > %s' % instance
  last = instance
  
print 'Started:'
for a,b in cm.IsStarted().iteritems():
  print '  > %s -> %r' % (a,b)
  
print 'Started - specific:'
for a,b in cm.IsStarted([last]).iteritems():
  print '  + %s -> %r' % (a,b)

print 'Failed:'
for a,b in cm.IsFailed().iteritems():
  print '  > %s -> %r' % (a,b)

print 'Failed - specific:'
for a,b in cm.IsFailed([last]).iteritems():
  print '  + %s -> %r' % (a,b)
  
print 'Running:'
for a,b in cm.IsRunning().iteritems():
  print '  > %s -> %r' % (a,b)

print 'Running - specific:'
for a,b in cm.IsRunning([last]).iteritems():
  print '  + %s -> %r' % (a,b)
  
print 'Status:'
for a,b in cm.GetStatus().iteritems():
  print '  > %s -> %s' % (a,b)

print 'Status - specific:'
for a,b in cm.GetStatus([last]).iteritems():
  print '  + %s -> %s' % (a,b)
  
name = cm.StartNewInstance()
waited = 0
while not cm.IsRunning(name) and not cm.IsFailed(name):
  print """Time %3i:
  %s
    > started: %r
    > running: %r
    > failed:  %r
    > status:  %s""" % (waited, name, cm.IsStarted(name), cm.IsRunning(name), cm.IsFailed(name), cm.GetStatus(name))
  time.sleep(1.5)
  cm.Heartbeat()
  time.sleep(1.5)
  waited += 3
  
print """Time %3i:
  %s
    > started: %r
    > running: %r
    > failed:  %r
    > status:  %s""" % (waited, name, cm.IsStarted(name), cm.IsRunning(name), cm.IsFailed(name), cm.GetStatus(name))
