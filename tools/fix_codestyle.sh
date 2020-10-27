#!/bin/bash
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

GIT="$( git rev-parse --show-toplevel )"

if [ ! -e "$GIT/.git" ]
then
    echo "Not a git repository ${PWD}"
    exit 1
fi

ignore_patterns=$(cat "$GIT/.clang-format-ignore")
patterns=$(cat "$GIT/.clang-format-dirs")

format_file() {
  #return
  file="${1}"
  #echo "Found ${file}..."
  [[ $file != *.c ]] && [[ $file != *.cpp ]] && [[ $file != *.h ]] && return
  [[ -f $file ]] || return 0
  for p in $patterns; do
    [[ $p == "" ]] && continue
    [[ $file =~ "$p" ]] || continue
    for ip in $ignore_patterns; do
      [[ $ip == "" ]] && continue
      [[ $file =~ "$ip" ]] && return 0
    done

    [[ $silent ]] || echo "Reformatting ${file}..."
    /usr/local/bin/clang-format -style=file -fallback-style=none -i ${file}
    git add "${file}"
  done
}


case "${1}" in
  --about )
    echo "Runs clang-format on source files"
    ;;
  --all )
    echo "Fixing codestyle for all files..."
    for file in `git ls-files` ; do
      format_file "${file}"
    done
    ;;
  * )
    echo "Fixing codestyle for modified files..."
    #silent=1
    for file in `git diff-index --cached --name-only HEAD` ; do
      format_file "${file}"
    done
    ;;
esac
