#!/usr/bin/env python

import argparse
import glob
import os

import simplejson
import yaml

# Parse commandline
parser = argparse.ArgumentParser(description='Parse Mandala dict YAML files and print JSON')
parser.add_argument('--dict', required=False, help='path to dict YAML file')
args = parser.parse_args()

args.dict = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'dict/mandala.yml')

configExt = '.yml'


# construct dict based on file [config] or data
class Mandala(dict):
    def __init__(self, config=None, data=None, parent=None):
        dict.__init__(self)

        if not data:
            data = self.read_config(config)

        # inherit parent props
        if parent:
            for p in parent:
                if isinstance(parent[p], list):
                    continue
                if p in ['name', 'title', 'desc']:
                    continue
                self[p] = parent[p]

        if config:
            self['config'] = config

        for key in data:
            if key == 'content':
                continue
            self[key] = data[key]

        for key in data:
            if key != 'content':
                continue
            obj = data[key]
            self[key] = list()
            if not isinstance(obj, list):
                obj = self.read_config(os.path.join(os.path.dirname(self['config']), obj+configExt))
            for i in list(obj):
                self[key].append(Mandala(data=i, parent=self))

        # post process some fields
        if 'suffix' in self:
            print parent
            assert(parent)
            self['title'] = parent['title']+' '+self['suffix']
            del self['suffix']

    def read_config(self, config):
        print('Reading {}...'.format(config))
        with open(config, 'r') as f:
            return yaml.load(f.read())


mandala = Mandala(config=args.dict)

print(simplejson.dumps(mandala))
