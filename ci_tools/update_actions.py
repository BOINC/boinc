#!/usr/bin/env python3

# This file is part of BOINC.
# https://boinc.berkeley.edu
# Copyright (C) 2025 University of California
#
# BOINC is free software; you can redistribute it and/or modify it
# under the terms of the GNU Lesser General Public License
# as published by the Free Software Foundation,
# either version 3 of the License, or (at your option) any later version.
#
# BOINC is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with BOINC.  If not, see <http://www.gnu.org/licenses/>.

import os
import requests
import sys

def get_all_yml_files():
    yml_files = []
    for root, _, files in os.walk(".github/workflows", topdown=True):
        for filename in files:
            if filename.endswith(".yml"):
                yml_files.append(os.path.join(root, filename))
    return yml_files

def get_all_actions(yml_files):
    actions = []
    for yml_file in yml_files:
        with open(yml_file, "r") as f:
            lines = f.readlines()
        for line in lines:
            if "uses: " in line:
                line = line.strip()
                line = line.split("@")[0]
                line = line.split("uses: ")[1]
                res = line.split(" ")
                if len(res) > 1 and res[1] == "# ignore-check":
                    continue
                line = res[0]
                if line not in actions:
                    actions.append(line)
    return actions

def get_action_with_sha(action, token):
    if token is not None:
        headers = {'Authorization': f'Bearer {token}'}
    else:
        headers = None
    print("Getting latest release for: " + action)
    res = requests.get(f'https://api.github.com/repos/{action}/releases/latest', headers=headers).json()
    tag_name = res['tag_name']
    res = requests.get(f'https://api.github.com/repos/{action}/git/ref/tags/{tag_name}', headers=headers).json()
    object_type = res['object']['type']
    tag_sha = res['object']['sha']
    if object_type != 'commit':
        res = requests.get(f'https://api.github.com/repos/{action}/git/tags/{tag_sha}', headers=headers).json()
        tag_sha = res['object']['sha']
    return action, tag_sha

def get_all_actions_with_sha(actions, token):
    actions_with_sha = []
    for action in actions:
        actions_with_sha.append(get_action_with_sha(action, token))
    return actions_with_sha

def get_action_sha(actions_with_sha, action):
    for action_with_sha in actions_with_sha:
        if action_with_sha[0] == action:
            return action_with_sha[1]

def process_yml_file(yml_file, actions_with_sha):
    print("Processing file: " + yml_file)
    with open(yml_file, "r") as f:
        lines = f.readlines()
    with open(yml_file, "w") as f:
        for line in lines:
            new_line = line
            if "uses: " in new_line:
                new_line = new_line.strip()
                res = new_line.split("@")
                version = res[1]
                new_line = res[0]
                new_line = new_line.split("uses: ")[1]
                res = version.split(" ")
                if len(res) == 1:
                    version = res[0]
                    new_sha = get_action_sha(actions_with_sha, new_line)
                    if new_sha != version:
                        line = line.replace(version, new_sha)
            f.write(line)

def process_all_yml_files(yml_files, actions_with_sha):
    for yml_file in yml_files:
        process_yml_file(yml_file, actions_with_sha)


if (len(sys.argv) > 1):
    token = sys.argv[1]
else:
    token = None

yml_files = get_all_yml_files()

latest_actions = get_all_actions_with_sha(get_all_actions(yml_files), token)

process_all_yml_files(yml_files, latest_actions)
