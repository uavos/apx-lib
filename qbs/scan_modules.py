#!/usr/bin/env python

import argparse
import glob
import os

import simplejson
import yaml

# Parse commandline
parser = argparse.ArgumentParser(description='Scan modules for dependencies using YAML config and print JSON array')
parser.add_argument('--modules', nargs='+', help='list of modules')
parser.add_argument('--paths', nargs='+', help='search paths')
parser.add_argument('--config', help='convert specific config to json')
args = parser.parse_args()

configExt = '.yml'


def expand_wildcards(path, files):
    res = []
    for i in files:
        for f in list(glob.glob(os.path.join(path, i))):
            res.append(os.path.relpath(f, path))
    return res


class Module(dict):
    def __init__(self, name=None, config=None):
        dict.__init__(self, name=name)

        # find configFile
        if not config:
            configFileName = name.split('.')[-1] + configExt
            for path in list(args.paths):
                config = os.path.abspath(os.path.join(path, name.replace('.', '/'), configFileName))
                if os.path.exists(config):
                    break

        if not config or not os.path.exists(config):
            raise Exception('Module config file not found: ' + config)

        with open(config, 'r') as f:
            obj = yaml.load(f.read())
            for i in obj:
                self[i] = obj[i]

        self["config"] = config
        if 'files' in self:
            files = expand_wildcards(os.path.dirname(config), self['files'])
            self['files'] = list(set([f for f in files if f != os.path.basename(config)]))


class Modules(list):
    def __init__(self, names, paths):
        list.__init__(self)
        self.paths = paths
        self.names = list()
        if 'all' in names:
            self.add_all()
        else:
            for name in names:
                self.add_module(name)

    def add_module(self, name, depends=True):
        if name in self.names:
            return
        m = Module(name=name)
        self.append(m)
        self.names.append(name)
        if depends and 'depends' in m and isinstance(m['depends'], list):
            for dep in m['depends']:
                self.add_module(dep)

    def add_all(self):
        for path in list(self.paths):
            for root, dirs, files in os.walk(path):
                for f in files:
                    if os.path.splitext(f)[1] != configExt:
                        continue
                    name = os.path.relpath(os.path.dirname(os.path.join(root, f)), path).replace('/', '.')
                    # print(name)
                    self.add_module(name=name, depends=False)


if args.config:
    modules = Module(config=args.config)
else:
    if not args.modules or not args.paths:
        parser.print_usage()
        exit(1)
    modules = Modules(args.modules, args.paths)

# print args.modules
# print modules.names
print(simplejson.dumps(modules))
