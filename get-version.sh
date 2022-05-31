#!/usr/bin/env bash
# Copyright (c) 2022 Michael Breen (https://mbreen.com)
# Output a "SOURCE VERSION" string derived from the git state.
# SOURCE:
# If the working directory corresponds to a commit that has been
# pushed to origin then SOURCE is the origin URL, otherwise
# SOURCE is the local directory.
# VERSION:
# If there are uncommitted changes then VERSION is "devel".
# Otherwise, if the working directory corresponds to a commit
# with a version tag in SOURCE (i.e., the tag has been pushed
# if SOURCE is origin) then it is the tag.
# Otherwise, VERSION is the currently checked-out commit ID.
set -e

COMMIT=$(git describe --dirty --long --always |
    sed 's/.*-g//;s/.*-dirty/devel/')

SOURCE=$(git rev-list --glob remotes/origin | grep -q ^$COMMIT &&
    git ls-remote --get-url origin || true)

TAG=$( (git describe --tags --match='v[0-9]*' --exact-match \
    --dirty 2>/dev/null || echo -dirty) | sed 's/.*dirty.*//' )

if [ -n "$TAG" ]; then
    if [ -z "$SOURCE" ] ||
            ( (2>/dev/null git ls-remote --tags origin "$TAG" ||
            >&2 echo "$0: failed to check origin repo for tag $TAG") |
            grep -q . ) then
        COMMIT="$TAG"
    fi
fi

# Normalize URL.
SOURCE=$(echo $SOURCE|sed 's,^git@github.com:,https://github.com/,')

[ -n "$SOURCE" ] || SOURCE=$(pwd)

echo "$SOURCE $COMMIT"
