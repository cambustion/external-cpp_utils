# Hey Emacs, this is -*- coding: utf-8; mode: bazel -*-

debug = False
default_repository_key = "github"

def select_repository(name, repositories, key = default_repository_key):
  if(debug): print('Selecting "' + key + '" repository.')
  rule = repositories[key]["rule"]
  kwargs = repositories[key]["kwargs"]
  rule(name=name, **kwargs)
