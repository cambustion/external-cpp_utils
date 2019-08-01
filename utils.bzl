def select_repository(name, repositories, key = "remote"):
  if(internal.debugging): print('Selecting "' + key + '" repository.')
  rule = repositories[key]["rule"]
  kwargs = repositories[key]["kwargs"]
  rule(name=name, **kwargs)
