#!/usr/bin/python

import argparse
import os

import simplejson
from jinja2 import Environment, FileSystemLoader

# Parse commandline
parser = argparse.ArgumentParser(description='Generate source files for APX system.')
parser.add_argument('--data', action='store', required=True, help='JSON encoded data')
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


# parse template
def render(template, data):
    loader = FileSystemLoader(os.path.dirname(os.path.abspath(template)))
    env = Environment(loader=loader, trim_blocks=True, lstrip_blocks=False)
    env.filters['merge_arrays'] = merge_arrays
    env.filters['merge_dicts'] = merge_dicts
    env.filters['expand_templates'] = expand_templates
    template = env.get_template(os.path.basename((template)))
    return template.render(data=data)


# main
dest = os.path.splitext(os.path.basename(args.template))[0]
print("Generating {}...".format(dest))

data = simplejson.loads(str(args.data))

with open(os.path.join(args.dest, dest), 'w') as f:
    f.write(render(args.template, data))
    f.close()
