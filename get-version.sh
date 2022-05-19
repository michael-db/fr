#!/usr/bin/env bash
# Copyright (c) 2022 Michael Breen (https://mbreen.com)
# Output a program source-version string derived from the git state.
# If the working directory corresponds to a version tag that has
# been pushed to origin then output "ORIGIN-URL VERSION", e.g.,
# "https://example.com/my-repo v1.2".
set -e

TAG=$( (git describe --tags --match='v[0-9]*' --dirty 2>/dev/null \
    || echo -dirty)|sed s/.*-dirty/devel/)

SOURCE=$( (2>/dev/null git ls-remote --tags origin "$TAG" \
    || >&2 echo "$0: Failed to check origin repo," \
    "using working dir as version source.") |
    grep -q . && git ls-remote --get-url origin || pwd)

echo "$SOURCE $TAG"
