#!/usr/bin/env bash
##
## APX Autopilot project <http://docs.uavos.com>
##
## Copyright (c) 2003-2020, Aliaksei Stratsilatau <sa@uavos.com>
## All rights reserved
##
## This file is part of APX Shared Libraries.
##
## This program is free software: you can redistribute it and/or modify
## it under the terms of the GNU Lesser General Public License as published by
## the Free Software Foundation, either version 3 of the License, or
## (at your option) any later version.
##
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU Lesser General Public License for more details.
##
## You should have received a copy of the GNU Lesser General Public License
## along with this program. If not, see <http://www.gnu.org/licenses/>.
##


# This script computes the project version based on git repository tags.
# The tag 'vX.Y' must exist to define the major (X) and minor (Y) versions.
# The version build bumber is based on 'release-X.Y.zzz' tags.

# info=echo
info=:

# find git toplevel repository
GIT="$( git rev-parse --show-toplevel )"
if [ ! -e "$GIT/.git" ]
then
    echo "Not a git repository ${PWD}"
    exit 1
fi

# get closest version tag
version_tag=$(git describe --tags --match="v*.*" --abbrev=0 2> /dev/null)
$info "Version tag: $version_tag"


regex="v([0-9]+).([0-9]+)"
if [[ $version_tag =~ $regex ]]; then
    major="${BASH_REMATCH[1]}"
    minor="${BASH_REMATCH[2]}"
else
    echo "Version tag error"
    exit 1
fi
version=$major.$minor
$info "Version: $version"

# find releases of above version
release_tag=$(git describe --tags --match="release-$version.*" --abbrev=0 2> /dev/null)

if [[ $release_tag ]]; then
    $info "Release tag: $release_tag"
    commits_cnt=$(git rev-list $release_tag.. --count)
    $info "Commits since last release: $commits_cnt"
    # find build number
    regex="release-$version.([0-9]+)"
    if [[ $release_tag =~ $regex ]]; then
        build="${BASH_REMATCH[1]}"
    else
        echo "Release tag error"
        exit 1
    fi
    $info "Last released build: $build"
    if (( $commits_cnt > 0 )); then
        build=$(echo $build + 1 | bc)
    fi
    version=$version.$build

else
    $info "New version from tag: $version"
    version=$version.0
fi


echo $version
