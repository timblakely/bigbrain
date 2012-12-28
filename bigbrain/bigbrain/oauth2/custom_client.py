import urllib2
from urllib import urlencode
import json
import webbrowser
from time import sleep
import re

class OAuth2Client:
  web_service = False
  cache_file = ""
  oauth_params = {}

  def __init__(self, client_secrets):
    secrets_file = open(client_secrets,'r')
    self.oauth_params = json.load(secrets_file)
    secrets_file.close()

  def NewQueryCode(self):
    oauth_params = self.oauth_params
    
    #TODO(blakely): Add ability to set up as web server
    oauth_params['response_type'] = 'code'
    
    
    queryString = "https://accounts.google.com/o/oauth2/auth?scope=%s&redirect_uri=%s&response_type=%s&client_id=%s" % ( \
      oauth_params['scope'], \
      oauth_params['redirect_uri'], \
      oauth_params['response_type'], \
      oauth_params['client_id'] \
    )
    
    webbrowser.open_new_tab(queryString)
    sleep(2)
    
    self.oauth_params['code'] = raw_input("Please enter the code here: ")
    
    if self.cache_file != "":
      self.CacheAuthValue()
    
    return True
  
  def CacheAuthValue(self):
    if self.cache_file == "":
      return
    
    cache_file = open(self.cache_file,'w')
    json.dump(self.oauth_params, cache_file)
    cache_file.close()
  
  def LoadCachedAuthValue(self):
    if self.cache_file == "":
      return
    
    try:
      cache_file = open(self.cache_file,'r')
      self.oauth_params = json.load(cache_file)
      cache_file.close()
    except:
      print "No cache file detected!"
  
  def SetCacheFile(self, cache_file):
    self.cache_file = cache_file
    self.LoadCachedAuthValue()
  
  def GetNewAccessToken(self):
    oauth_params = self.oauth_params
    if 'code' not in oauth_params:
      self.NewQueryCode()
    
    if 'code' not in oauth_params:
      print "Error getting code!"
      return
    
    # TODO: Web service
    oauth_params['grant_type'] = 'authorization_code'
      
    token_values = {
      'code' : oauth_params['code'],
      'client_id' : oauth_params['client_id'],
      'client_secret' : oauth_params['client_secret'],
      'redirect_uri' : oauth_params['redirect_uri'],
      'grant_type' : oauth_params['grant_type']
      }
    
    encoded_token_values = urlencode(token_values)
    
    print encoded_token_values
    
    request = urllib2.Request(url="https://accounts.google.com/o/oauth2/token", data = encoded_token_values )
  
    try:
      reply =  urllib2.urlopen(request)
    except urllib2.HTTPError as err:
      print err.read()
      if err.code == 400:
        if self.NewQueryCode():
          self.GetNewAccessToken()
      print "Failed to get New Query Code"
      return
    
    reply = json.loads(reply.read())

    self.oauth_params['access_token'] = reply['access_token']
    self.oauth_params['refresh_token'] = reply['refresh_token']
    if self.cache_file != "":
      self.CacheAuthValue()
  
  def RefreshAccessToken(self):
    if 'code' not in self.oauth_params:
      self.NewQueryCode()
    
    if 'code' not in self.oauth_params:
      print "Error getting new application code"
      return False
    
    #TODO Web server-ize 
    self.oauth_params['grant_type'] = 'refresh_token'
    
    token_values = {
      'refresh_token' : self.oauth_params['refresh_token'],
      'client_id' : self.oauth_params['client_id'],
      'client_secret' : self.oauth_params['client_secret'],
      'grant_type' : self.oauth_params['grant_type']
      }
    
    encoded_token_values = urlencode(token_values)
    
    request = urllib2.Request(url="https://accounts.google.com/o/oauth2/token", data = encoded_token_values )
    
    jsonReply =  urllib2.urlopen(request).read()
    reply = json.loads(jsonReply)
    self.oauth_params['access_token'] = reply['access_token']
    if self.cache_file != "":
      self.CacheAuthValue()
    return True
        
  # Helper functions
  def MakeCall(self, url, request):
    if 'access_token' not in self.oauth_params:
      self.GetNewAccessToken()
    
    auth_request = request
    auth_request.add_header("Authorization", "Bearer "+self.oauth_params['access_token'])
    try:
      connection = urllib2.urlopen(auth_request)
    except urllib2.HTTPError as err:
      if err.code == 401:
        #print err.read()
        print "Google said we are unauthorized, need to request new token"
        self.oauth_params['access_token'] = "" #invalidating access_token
        if(self.RefreshAccessToken()):
          return self.MakeCall(url, request)
        return
      else:
        print "Google responded with an error: " + str(err.code)
        return
    
    jsonReply = connection.read()
    #print jsonReply
    return json.loads(jsonReply)
    
  
  # API CALLS
  def APIGET(self, url, data = {}):
    if data:
      url += "?"+urlencode(data)
    request = urllib2.Request(url)
    return self.MakeCall(url, request)
  
  def APIPOST(self, url, data = {}):
    if data:
      query = json.dumps(data)
      request = urllib2.Request(url = url, data = query, headers={"Content-type":"application/json"})
    else:
      request = urllib2.Request(url = url)
    return self.MakeCall(url, request)
  
  def APIDELETE(self, url, data = {}):
    if data:
      query = json.dumps(data)
      request = urllib2.Request(url = url, data = query, headers={"Content-type":"application/json"})
    else:
      request = urllib2.Request(url = url)
    request.get_method = lambda: 'DELETE'    
    return self.MakeCall(url, request)