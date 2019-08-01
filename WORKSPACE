workspace(name = "rh_cpp_utils")

metaPath = __workspace_dir__ + "/.meta"
metaUrl = "file://" + metaPath

load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository")
load("//:utils.bzl", "select_repository")

select_repository(
  name = "compdb",
  # key = "local",
  repositories = {
    "local": {
      "rule": local_repository,
      "kwargs": {
        "path": metaPath + "/bazel-compdb",
      },
    },
    "remote": {
      "rule": git_repository,
      "kwargs": {
        "remote": "https://github.com/ramblehead/bazel-compdb.git",
        "branch": "master",
      },
    },
    "local_git": {
      "rule": git_repository,
      "kwargs": {
        "remote": metaUrl + "/bazel-compdb",
        "branch": "master",
      },
    },
  },
)

new_local_repository(
  name = "system",
  path = "/usr/lib/x86_64-linux-gnu",
  build_file = "system.BUILD",
)
