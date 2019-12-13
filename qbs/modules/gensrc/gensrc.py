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


def render(template, data):
    loader = FileSystemLoader(os.path.dirname(os.path.abspath(template)))
    env = Environment(loader=loader, trim_blocks=True, lstrip_blocks=False)
    template = env.get_template(os.path.basename((template)))
    return template.render(data=data)


dest = os.path.splitext(os.path.basename(args.template))[0]
print("Generating {}...".format(dest))

data = simplejson.loads(str(args.data))

with open(os.path.join(args.dest, dest), 'w') as f:
    f.write(render(args.template, data))
    f.close()
