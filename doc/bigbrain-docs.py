import os

from google.appengine.ext.webapp import template
import webapp2

class Home(webapp2.RequestHandler):
  def get(self, reqpath):
    paths = [
        ('Home', 'home'),
        ('Overview', 'overview'),
        ('Set-up', 'setup', [
           ('Localhost', 'setup/localhost'),
           ('Google Compute Engine', 'setup/gce')]),
        ('Tutorial', 'tutorial'),
        ('Download', 'download'),
        # ('API', 'api')
    ]

    firstpath = reqpath.split('/')[0]

    if firstpath not in [x[1] for x in paths]:
      page = 'home'
    else:
      page = firstpath

    downpath = reqpath.split('/')[1:]

    path = os.path.join(os.path.dirname(__file__), 'main.html')
    tvar = {
        'paths': paths,
        'page': page,
        'downpath': downpath
    }
    self.response.out.write(template.render(path, tvar))

app = webapp2.WSGIApplication([
    ('/(.*)', Home)], debug=True)
