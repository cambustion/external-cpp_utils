# Hey Emacs, this is -*- coding: utf-8; mode: bazel -*-

workspace(name = "rh_cpp_utils")

metaPath = __workspace_dir__ + "/.meta"

load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository")

git_repository(
  name = "rh_bazel_utils",
  remote = "https://github.com/ramblehead/bazel-utils.git",
  commit = "9cfd857c9d2af7fe84d3a4b77b87f31107f9882b",
  shallow_since = "1564742532 +0100",
  # branch = "master",
)

# local_repository(
#   name = "rh_bazel_utils",
#   path = metaPath + "/bazel-utils",
# )

load(
  "@rh_bazel_utils//:index.bzl",
  "select_repository",
  "default_repository_key"
)

select_repository(
  name = "compdb",
  repositories = {
    default_repository_key: {
      "rule": git_repository,
      "kwargs": {
        "remote": "https://github.com/ramblehead/bazel-compdb.git",
        "commit": "b21f7fa24b4261d0d8ea8b157dae3d134d997894",
        "shallow_since": "1564584337 +0100",
      },
    },
    "github_master": {
      "rule": git_repository,
      "kwargs": {
        "remote": "https://github.com/ramblehead/bazel-compdb.git",
        "branch": "master",
      },
    },
    "meta": {
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
