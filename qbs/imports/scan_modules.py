#!/usr/bin/env python

import simplejson
import yaml
import os
import glob

import argparse

# Parse commandline
parser = argparse.ArgumentParser(description='Scan modules for dependencies using YAML config and print JSON array')
parser.add_argument('--modules', nargs='+', required=True, help='list of modules')
parser.add_argument('--paths', nargs='+', required=True, help='search paths')
args = parser.parse_args()

configExt = '.yml'


def expand_wildcards(path, files):
    res = []
    for i in files:
        for f in list(glob.glob(os.path.join(path, i))):
            res.append(os.path.relpath(f, path))
    return res


class Module(dict):
    def __init__(self, name):
        dict.__init__(self, name=name)

        # find configFile
        config = ''
        fname = os.path.split(name)[-1]
        for path in args.paths:
            for f in [
                os.path.join(path, name, fname + configExt),
                os.path.join(path, os.path.split(name)[0], fname + configExt),
                os.path.join(path, name, 'module' + configExt),
                os.path.join(path, fname + configExt)
            ]:
                if os.path.exists(f):
                    config = f
                    break

            if os.path.exists(config):
                break

        if not os.path.exists(config):
            raise Exception('Module config file not found: ' + name + ' in ' + ','.join(args.paths))

        with open(config, 'r') as f:
            obj = yaml.load(f.read())
            for i in obj:
                self[i] = obj[i]

        self["config"] = config
        if 'files' in self:
            self['files'] = expand_wildcards(os.path.dirname(config), self['files'])


class Modules(list):
    def __init__(self, names):
        list.__init__(self)
        self.names = list()
        for name in names:
            self.add_module(name)

    def add_module(self, name):
        if name in self.names:
            return
        m = Module(name)
        self.append(m)
        self.names.append(name)
        if 'depends' in m and isinstance(m['depends'], list):
            for dep in m['depends']:
                self.add_module(dep)


modules = Modules(args.modules)

# print args.modules
# print modules.names
print(simplejson.dumps(modules))
