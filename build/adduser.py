import getpass
import sys

import pymongo

dbconn = pymongo.connection.Connection()
database = dbconn.bigbrain

user = raw_input('Username to add: ')

password = getpass.getpass('Password : ')
password_again = getpass.getpass('Enter again: ')

if password != password_again:
  print 'Passwords do not match! User not added'
  sys.exit(-1)

database.users.save({'user': user, 'password': password})

print 'User %s successfully added' % user
