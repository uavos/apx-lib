#!/usr/bin/python
#
# APX Autopilot project <http://docs.uavos.com>
#
# Copyright (c) 2003-2020, Aliaksei Stratsilatau <sa@uavos.com>
# All rights reserved
#
# This file is part of APX Shared Libraries.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.
#

import argparse
import os

import networkx as nx
import simplejson
from jinja2 import Environment, FileSystemLoader

import yaml

import copy

# import matplotlib.pyplot as plt

# Parse commandline
parser = argparse.ArgumentParser(
    description='Generate source files for APX system.')
parser.add_argument('--data', action='store', required=True,
                    help='JSON/YAML data file name[s] or raw JSON or cmake format')
parser.add_argument('--template', action='store', nargs='*',
                    required=False, help='Template file[s] to use')
parser.add_argument('--dest', action='store', required=True,
                    help='Destination directory or file name')
args = parser.parse_args()
#########################


# filters

def merge_dicts_impl(out, d):
    if type(d) is not dict:
        print("Error dict object: ", d)
        return
    for i in out:
        if i['name'] != d['name']:
            continue
        for key, value in d.items():
            if key not in i:
                i[key] = value
            elif type(value) is list:
                i.update({key: i[key] + value})
            else:
                i.update({key: value})
        return
    out.append(d)


def merge_dicts(list_dicts):
    out = list()
    dout = dict()
    # print("MERGE DICTS: {}".format(list_dicts))
    for d in list(list_dicts):
        assert type(d) is dict, "Error dict object ({})".format(d)
        name = d['name']
        if name in dout:
            base = dout[name]
            # print("UPD<{}: {} {}".format(name, dout, base))
            # print("UPD>{}: {} {}".format(name, dout, d))
            for key, value in d.items():
                if not value:
                    continue
                if key not in base:
                    base.update({key: value})
                elif type(value) is list:
                    if base[key]:
                        base.update({key: merge_dicts(base[key] + value)})
                    else:
                        base.update({key: merge_dicts(value)})
                else:
                    base.update({key: value})
        else:
            base = copy.deepcopy(d)
            out.append(base)
        dout[name] = base
    return out


def merge_arrays(list_dicts):
    out = list()
    for i in list(list_dicts):
        if type(i) is list:
            out = out + i
        else:
            out = out + [i]
    # print("MERGE ARRAYS: {}".format(out))
    return out


def expand_templates_impl(list_dicts, templates):
    if not list_dicts:
        return None
    out = list()
    exp = False
    for d in list(list_dicts):
        if 'template' in d:
            t = d['template']
            if t in templates:
                out = out + templates[t]
                exp = True
            continue
        out.append(d)
        if 'content' in d:
            d['content'] = expand_templates_impl(d['content'], templates)
    if not exp:
        return out
    return expand_templates_impl(out, templates)


def expand_templates(list_dicts):
    out = list()
    templates = dict()
    for i in list(list_dicts):
        if 'template' in i and 'content' in i:
            templates[i['template']] = i['content']
        else:
            out.append(i)
    out = expand_templates_impl(out, templates)
    # print("EXP TEMPLATES: {}".format(out))
    return out


def expand_constants_impl(list_dicts, constants):
    if not list_dicts:
        return None
    out = list()
    for d in list_dicts:
        for i in d:
            if str(d[i]) == d[i] and d[i].startswith('$') and d[i][1:] in constants:
                d[i] = constants[d[i][1:]]
        out.append(d)
        if 'content' in d:
            d['content'] = expand_constants_impl(d['content'], constants)
    return out


def expand_constants(list_dicts):
    out = list()
    constants = dict()
    for i in list(list_dicts):
        if 'constants' in i:
            constants.update(i['constants'])
        else:
            out.append(i)
    out = expand_constants_impl(out, constants)
    # print("EXP CONSTANTS: {}".format(out))
    return out


def sort_modules(list_dicts):
    init_order = ['core', 'early', 'main', 'late', 'app']

    g = nx.MultiDiGraph()
    nx.add_path(g, init_order)

    for x in list_dicts:
        xn = x['name']
        g.add_node(xn)
        after, before = None, None

        if 'order' in x:
            init = x['order']
            if ':' in init:
                after, before = init.split(':')
            else:
                before = init
        else:
            before = 'late'

        if before:
            if before in init_order:
                g.add_edge(xn, before)
                i = init_order.index(before)
                if i > 0:
                    g.add_edge(init_order[i-1], xn)
            else:
                g.add_edge(xn, before)

        if after:
            g.add_edge(after, xn)

        if 'depends' in x:
            for dep in x['depends']:
                g.add_edge(dep, xn)

    # nx.draw_kamada_kawai(g, with_labels=True)
    # plt.savefig(os.path.join(args.dest, 'path.png'))
    slist = list(nx.lexicographical_topological_sort(g))
    # print(slist)
    res = list()
    for i in slist:
        for j in list_dicts:
            if j['name'] == i:
                res.append(j)
                break
    return res


def render(template, data):
    loader = FileSystemLoader(os.path.dirname(os.path.abspath(template)))
    env = Environment(loader=loader, trim_blocks=True, lstrip_blocks=False)
    env.filters['merge_arrays'] = merge_arrays
    env.filters['merge_dicts'] = merge_dicts
    env.filters['expand_templates'] = expand_templates
    env.filters['expand_constants'] = expand_constants
    env.filters['sort_modules'] = sort_modules
    template = env.get_template(os.path.basename((template)))
    return template.render(data=data)


def load_data(data):
    ext = os.path.splitext(data)[1][1:].lower()
    with open(data, 'r') as f:
        if ext == 'yml' or ext == 'yaml':
            return yaml.load(f.read(), Loader=yaml.Loader)
        elif ext == 'json':
            return simplejson.loads(str(f.read()))
        else:
            return str(f.read()).strip()
    return {}


def parse_cmake_data(data):
    out = dict()
    index = data.index(';')+1
    while index < len(data):
        next_index = data.find(';', index)
        if '[' in data and next_index > data.find('[', index):
            next_index = data.index(']', index)+1

        if next_index <= 0:
            next_index = len(data)

        token = data[index:next_index]
        index = next_index+1

        name, value = token.split(':')
        if value.startswith('['):
            value = value[1:-1].split(';')
            if os.path.exists(value[0]):
                value_exp = []
                for v in value:
                    value_exp.append(load_data(v))
                value = value_exp
        elif os.path.exists(value):
            value = load_data(value)
        out[name] = value
        # print("{}: {}".format(name, value))
    return out

##################################################################
# main
##################################################################


# parse and load all data
if args.data.startswith('#'):
    data = parse_cmake_data(args.data)
elif ';' in args.data:
    data = []
    for i in args.data.split(';'):
        data.append(load_data(i))
elif os.path.exists(args.data):
    data = load_data(args.data)
else:
    data = simplejson.loads(str(args.data))
# print("GENSRC_DATA: {}".format(data))

# process data

if args.template:
    # render templates mode
    os.makedirs(args.dest, exist_ok=True)

    for template in args.template:
        dest = os.path.splitext(os.path.basename(template))[0]
        if not '.' in dest:
            continue

        # print("Generating {}...".format(dest))

        with open(os.path.join(args.dest, dest), 'w') as f:
            f.write(render(template, data))
            f.close()
    exit()


if os.path.splitext(args.dest)[1][1:].lower() == 'json':
    # output JSON mode
    # print("Generating {}...".format(args.dest))
    outdir = os.path.dirname(args.dest)
    if outdir:
        os.makedirs(outdir, exist_ok=True)
    with open(args.dest, 'w') as f:
        simplejson.dump(data, f, indent=4)
        f.close()
    exit()
