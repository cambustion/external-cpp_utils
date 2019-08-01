# Hey Emacs, this is -*- coding: utf-8; mode: bazel -*-

def select_repository(name, repositories, key = "remote"):
  print('Selecting "' + key + '" repository.')
  rule = repositories[key]["rule"]
  kwargs = repositories[key]["kwargs"]
  rule(name=name, **kwargs)
