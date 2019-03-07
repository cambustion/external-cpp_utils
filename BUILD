load("//.project/compdb:compdb.bzl", "compilation_database")

compilation_database(
  name = "compdb",
  targets = [
    "//debug",
    "//reflection",
  ],
  module_exts = ["cpp"],
  exclude_dirs = ["external/"],
)
