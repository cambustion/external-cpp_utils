# Hey Emacs, this is -*- coding: utf-8; mode: bazel -*-

workspace(name = "rh_cpp_utils")

metaPath = __workspace_dir__ + "/.meta"

load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository")
load("//:select_repository.bzl", "select_repository", "default_repository_key")

select_repository(
  name = "compdb",
  repositories = {
    default_repository_key: {
      "rule": git_repository,
      "kwargs": {
        "remote": "https://github.com/ramblehead/bazel-compdb.git",
        "commit": "b21f7fa24b4261d0d8ea8b157dae3d134d997894",
        "shallow_since": "1564584337 +0100"
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
