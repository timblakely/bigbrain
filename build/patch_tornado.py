import re
import string
import sys
import tornado.httpclient

filename = tornado.httpclient.__file__
if filename[-1] == 'c':
  filename = filename[:-1]

try:
  targetfile = open(filename,'r')
except IOError:
  print 'ERR: can\'t read from target file!  Check permissions (try sudo?)'
  sys.exit(-1)
  
content = targetfile.read()
targetfile.close()

if content is None:
  print 'ERR: Target file empty?'
  sys.exit(-1)

if (re.search('binary_body=False', content, re.MULTILINE)) and (
    re.search('self.body = body if binary_body else utf8\(body\)', content, 
    re.MULTILINE)):
    print 'File already patched!'
    sys.exit(0)
  

print '''

This will patch the file:'

 >>  %s

to allow tornado's httpclient to send a binary body in addition to a unicode
encoded body.  This will add the following patches:

1) HTTPRequest will have a new __init__ param called binary_body that defaults
   to False.

2) In the HTTPRequest __init__ function, the line

   >>  self.body = utf8(body)

   will be replaced with

   >>  self.body = body if binary_body else utf8(body)
   
''' % filename

accept = raw_input('Is this acceptable? [y]/n - ')
accept = string.lower(accept)
if not accept:
  accept = 'y'

print accept
if accept != 'y' and accept != 'yes':
  print '**Patch not applied**'
  print ''
  print 'Note: patch must be applied for BigBrain to work successfully.'
  sys.exit(0)
  
print 'Patching...'
  
content, num = re.subn(
    'auth_username=None, auth_password=None,',
    'binary_body=False, auth_username=None, auth_password=None,',
    content)
print num

content, num = re.subn(
    'self.body = utf8\(body\)',
    'self.body = body if binary_body else utf8(body)',
    content)
print num

print 'Checking result...'
if not (re.search('binary_body=False', content, re.MULTILINE)) or not (
    re.search('self.body = body if binary_body else utf8\(body\)', content, 
    re.MULTILINE)):
    print 'ERR: String patching failed!'
    sys.exit(0)

print 'Writing file...'
try:
  targetfile = open(filename,'w')
except IOError:
  print 'ERR: can\'t write to target file!  Check permissions (try sudo?)'
  sys.exit(-1)
targetfile.write(content)
targetfile.close()

print 'Checking file...'
try:
  targetfile = open(filename,'r')
except IOError:
  print 'ERR: can\'t read from target file!  Check permissions (try sudo?)'
  sys.exit(-1)
content = targetfile.read()
targetfile.close()
if content is None:
  print 'ERR: target file empty?'
  sys.exit(-1)
if not (re.search('binary_body=False', content, re.MULTILINE)) or not (
    re.search('self.body = body if binary_body else utf8\(body\)', content, 
    re.MULTILINE)):
    print 'ERR: stored file appears unchanged?'
    sys.exit(-1)

print ''
print 'Patch successfully applied!'
