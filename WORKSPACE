# Hey Emacs, this is -*- coding: utf-8; mode: bazel -*-

workspace(name = "rh_cpp_utils")

metaPath = __workspace_dir__ + "/.meta"

load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository")

git_repository(
  name = "bazel_utils",
  remote = "git@github.com:ramblehead/bazel-utils.git",
  # branch = "master",
  commit = "456cef0e8af31c0fd98645381c14f2338b748226",
  shallow_since = "1565016199 +0100",
)

# local_repository(
#   name = "rh_bazel_utils",
#   path = metaPath + "/rh-bazel-utils",
# )

load("@rh_bazel_utils//:index.bzl", "select_repository")

select_repository(
  name = "compdb",
  repositories = {
    "default": {
      "rule": git_repository,
      "kwargs": {
        "remote": "git@github.com:ramblehead/bazel-compdb.git",
        # "branch": "master",
        "commit": "b21f7fa24b4261d0d8ea8b157dae3d134d997894",
        "shallow_since": "1564584337 +0100",
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
