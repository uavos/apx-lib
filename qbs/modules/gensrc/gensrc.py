#!/usr/bin/python

import argparse
import functools
import os

import networkx as nx
import simplejson
from jinja2 import Environment, FileSystemLoader

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
    return expand_templates_impl(out, templates)


def compare_dicts(x, y):
    xn = x['name']
    xdeps = x['depends'] if 'depends' in x else []
    # xbefore = x['before'] if 'before' in x else []
    xinit = x['init'] if 'init' in x else None
    yn = y['name']
    ydeps = y['depends'] if 'depends' in y else []
    # ybefore = y['before'] if 'before' in y else []
    yinit = y['init'] if 'init' in y else None
    res = 0

    init_order = ['core', 'early', 'main', 'late', 'app']
    xgrp = init_order.index(xinit) if xinit else -1
    ygrp = init_order.index(yinit) if yinit else -1

    # if xn in ydeps:
    #     res = -1
    # elif yn in xdeps:
    #     res = 1
    if xgrp >= 0 and ygrp < 0:
        res = -1
    elif ygrp >= 0 and xgrp < 0:
        res = 1
    elif xgrp >= 0 and ygrp >= 0:
        res = xgrp-ygrp

    if res == 0:
        if xn in ydeps:
            res = -1
        elif yn in xdeps:
            res = 1

    # elif xn == yinit:
    #     res = 1
    # elif yn == xinit:
    #     res = -1
    # elif yinit and not xinit:
    #     res = 1
    # elif xinit and not yinit:
    #     res = -1

    # elif xinit and yinit:
    #     res = cmp(x['init'], y['init'])

    # if res == 0:
    #     res = cmp(xn, yn)

    tst = ['node', 'stm32.hrt', 'conf']
    if xn in tst or yn in tst:
        if res < 0:
            print(xn, yn)
        elif res > 0:
            print(yn, xn)
        else:
            print(yn, xn, 'EQUAL')

    return res


def compare_depends(x, y):
    xn = x['name']
    xdeps = x['depends'] if 'depends' in x else []
    yn = y['name']
    ydeps = y['depends'] if 'depends' in y else []
    res = 0
    if xn in ydeps:
        res = -1
    elif yn in xdeps:
        res = 1
    return res


def compare_init(x, y):
    xn = x['name']
    xinit = x['init'] if 'init' in x else None
    yn = y['name']
    yinit = y['init'] if 'init' in y else None
    res = 0
    if xn == yinit:  # or (yinit and not xinit):
        res = 1
    elif yn == xinit or (xinit or yinit):
        res = -1
    print(xn, yn, res)
    return res


def topological_sort(source):
    """perform topo sort on elements.

    :arg source: list of ``(name, [list of dependancies])`` pairs
    :returns: list of names, with dependancies listed first
    """
    pending = [(d['name'], set(d['depends'] if 'depends' in d else [])) for d in source]  # copy deps so we can modify set in-place
    emitted = []
    while pending:
        next_pending = []
        next_emitted = []
        for entry in pending:
            name, deps = entry
            deps.difference_update(emitted)  # remove deps we emitted last pass
            if deps:  # still has deps? recheck during next pass
                next_pending.append(entry)
            else:  # no more deps? time to emit
                yield name
                emitted.append(name)  # <-- not required, but helps preserve original ordering
                next_emitted.append(name)  # remember what we emitted for difference_update() in next pass
        if not next_emitted:  # all entries have unmet deps, one of two things is wrong...
            raise ValueError("cyclic or missing dependancy detected: %r" % (next_pending,))
        pending = next_pending
        emitted = next_emitted


def sort_dicts(list_dicts):

    # a = list_dicts
    # for i in range(len(a)):
    #     x = a[i]
    #     xn = x['name']
    #     xdeps = x['depends'] if 'depends' in x else []
    #     xinit = x['init'] if 'init' in x else None
    #     for j in range(len(a)):
    #         y = a[j]
    #         yn = y['name']
    #         ydeps = y['depends'] if 'depends' in y else []
    #         yinit = y['init'] if 'init' in y else None
    #         if yn in xdeps and j < i:
    #             idx = i+1
    #             continue
    #         if yn == init or (init == 'core' and yinit != 'core'):
    #             idx = i
    #             break
    #     res.insert(idx, x)
    # return res
    # list_dicts.sort(lambda x, y: compare_depends(x, y))
    # list_dicts.sort(lambda x, y: compare_init(x, y))
    # return list_dicts
    # list_dicts.sort(key=functools.cmp_to_key(compare_depends))
    # list_dicts.sort(key=functools.cmp_to_key(compare_dicts))
    # names = topological_sort(list_dicts)
    # print list(names)
    init_order = ['core', 'early', 'main', 'late', 'app']

    g = nx.DiGraph()
    for i in range(len(init_order)-1):
        g.add_edge(init_order[i+1], init_order[i])

    for x in list_dicts:
        xn = x['name']
        g.add_node(xn, d=x)
        if 'init' in x:
            init = x['init']
            if init in init_order:
                g.add_edge(xn, init)
            else:
                g.add_edge(init, xn)

        if 'depends' in x:
            for dep in x['depends']:
                g.add_edge(xn, dep)

    slist = [x for x in reversed(list(nx.topological_sort(g))) if x not in init_order]

    print(slist)
    return list_dicts


# parse template
def render(template, data):
    loader = FileSystemLoader(os.path.dirname(os.path.abspath(template)))
    env = Environment(loader=loader, trim_blocks=True, lstrip_blocks=False)
    env.filters['merge_arrays'] = merge_arrays
    env.filters['merge_dicts'] = merge_dicts
    env.filters['expand_templates'] = expand_templates
    env.filters['sort_dicts'] = sort_dicts
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
