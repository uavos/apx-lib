#!/usr/bin/python

import argparse
import os

import networkx as nx
import simplejson
from jinja2 import Environment, FileSystemLoader

# import matplotlib.pyplot as plt

# Parse commandline
parser = argparse.ArgumentParser(description='Generate source files for APX system.')
parser.add_argument('--data', action='store', required=True, help='JSON encoded data or file name')
parser.add_argument('--template', action='store', required=True, help='Template file to use')
parser.add_argument('--dest', action='store', required=True, help='Destination directory')
args = parser.parse_args()
#########################


# filters

def merge_dicts_impl(out, d):
    if not isinstance(d, dict):
        print("Error dict object: ", d)
        return
    for i in out:
        if i['name'] != d['name']:
            continue
        for key, value in d.items():
            if key not in i:
                i[key] = value
            elif isinstance(value, list):
                i.update({key: i[key] + value})
            else:
                i.update({key: value})
        return
    out.append(d)


def merge_dicts(list_dicts):
    out = list()
    dout = dict()
    for d in list(list_dicts):
        assert isinstance(d, dict), "Error dict object"
        if d['name'] in dout:
            base = dout[d['name']]
            for key, value in d.items():
                if key not in base:
                    base[key] = value
                elif isinstance(value, list):
                    base.update({key: merge_dicts(base[key] + value)})
                else:
                    base.update({key: value})
        else:
            base = d
            out.append(base)
        dout[base['name']] = base
    return out


def merge_arrays(list_dicts):
    out = list()
    for i in list(list_dicts):
        if isinstance(i, list):
            out = out + i
        else:
            out = out + [i]
    return out


def expand_templates_impl(list_dicts, templates):
    out = list()
    for d in list_dicts:
        if 'template' in d:
            t = d['template']
            if t in templates:
                out = out + templates[t]
            continue
        out.append(d)
        if 'content' in d:
            d['content'] = expand_templates_impl(d['content'], templates)
    return out


def expand_templates(list_dicts):
    out = list()
    templates = dict()
    for i in list(list_dicts):
        if 'template' in i and 'content' in i:
            templates[i['template']] = i['content']
        else:
            out.append(i)
    out = expand_templates_impl(out, templates)
    return out


def expand_constants_impl(list_dicts, constants):
    out = list()
    for d in list_dicts:
        for i in d:
            if isinstance(d[i], str) and d[i].startswith('$') and d[i][1:] in constants:
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


# parse template
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


# main
dest = os.path.splitext(os.path.basename(args.template))[0]
print("Generating {}...".format(dest))

if os.path.exists(args.data):
    with open(args.data, 'r') as f:
        data = simplejson.loads(str(f.read()))
else:
    data = simplejson.loads(str(args.data))

with open(os.path.join(args.dest, dest), 'w') as f:
    f.write(render(args.template, data))
    f.close()
