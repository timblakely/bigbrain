from google.appengine.ext import webapp
import json

register = webapp.template.create_template_register()

def gql_json_parser(query_obj):
    result = []
    for entry in query_obj:
        result.append(dict([(p, unicode(getattr(entry, p))) for p in entry.properties()]))
    return result

@register.filter(name='make_json')
def make_json(thing):
  try:
    return json.dumps(thing)
  except Exception:
    return json.dumps(gql_json_parser(thing))

@register.filter(name='strip_benchmark')
def strip_benchmark(name):
  return name[10:]
