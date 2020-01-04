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


# construct dict based on file [config] or data
class Mandala(dict):
    # static members
    configExt = '.yml'
    config = None
    inheritance = list()

    def __init__(self, root_config=None, data=None, parent=None):
        dict.__init__(self)

        if root_config:
            assert(data is None and parent is None)
            data = self.read_config(root_config)
            Mandala.config = root_config
            Mandala.inheritance = data['inheritance']

        # inherit parent props
        if parent:
            for key in parent:
                if key in Mandala.inheritance:
                    self[key] = parent[key]

        for key in data:
            if key != 'content':
                self[key] = data[key]

        for key in data:
            if key != 'content':
                continue
            obj = data[key]
            self[key] = list()
            if not isinstance(obj, list):
                conf = os.path.join(os.path.dirname(Mandala.config), obj+Mandala.configExt)
                obj = self.read_config(conf)
            for i in list(obj):
                self[key].append(Mandala(data=i, parent=self))

    def read_config(self, config):
        # print('Reading {}...'.format(config))
        with open(config, 'r') as f:
            return yaml.load(f.read())


mandala = Mandala(root_config=args.dict)

print(simplejson.dumps(mandala))
