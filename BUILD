# Hey Emacs, this is -*- coding: utf-8; mode: bazel -*-

load("@compdb//:compdb.bzl", "compilation_database")

compilation_database(
  name = "compdb",
  targets = [
    "//debug",
    "//inline",
    "//reflection",
  ],
  module_exts = ["cpp"],
)
