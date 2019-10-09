# Hey Emacs, this is -*- coding: utf-8; mode: bazel -*-

workspace(name = "cpp_utils")

metaPath = __workspace_dir__ + "/.meta"

load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository")

git_repository(
  name = "bazel_utils",
  remote = "git@github.com:ramblehead/bazel-utils.git",
  branch = "master",
)

# local_repository(
#   name = "bazel_utils",
#   path = metaPath + "/bazel-utils",
# )

load(
  "@bazel_utils//:index.bzl",
  select_repository = "select_repository_local")

select_repository(
  name = "compdb",
  repositories = {
    "remote": {
      "rule": git_repository,
      "kwargs": {
        "remote": "git@github.com:ramblehead/bazel-compdb.git",
        "branch": "master",
      },
    },
    "local": {
      "rule": local_repository,
      "kwargs": {
        "path": metaPath + "/bazel-compdb",
      },
    },
  },
)

new_local_repository(
  name = "system",
  path = "/usr/lib/x86_64-linux-gnu",
  build_file = "system.BUILD",
)
