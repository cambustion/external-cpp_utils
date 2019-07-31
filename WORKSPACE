workspace(name = "rh_cpp_utils")

workspace(name = "s600_host")

load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository")

git_repository(
  name = "bazel_skylib",
  remote = "https://github.com/bazelbuild/bazel-skylib.git",
  tag = "0.9.0",
)

load("@bazel_skylib//lib:paths.bzl", "paths")

metaPath = paths.normalize(paths.join(__workspace_dir__, ".meta"))
metaUrl = "file://" + metaPath

# git_repository(
#   name = "compdb",
#   # remote = "https://github.com/ramblehead/bazel-compdb.git",
#   remote = paths.join(metaUrl, "bazel-compdb"),
#   branch = "master",
# )

local_repository(
  name = "compdb",
  path = paths.join(metaPath, "bazel-compdb"),
)

new_local_repository(
  name = "system",
  path = "/usr/lib/x86_64-linux-gnu",
  build_file = "system.BUILD",
)
