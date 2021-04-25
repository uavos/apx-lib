import argparse
import sys
import os
import shutil
import re
import yaml

from distutils.dir_util import copy_tree

FM_BOUNDARY = re.compile(r'^-{3,}$', re.MULTILINE)


def parse_frontmatter(content):
    content = content.strip()
    if not content.startswith('---\n'):
        return None, content
    # Parse text with frontmatter, extract metadata and content.
    try:
        _, fm, text = FM_BOUNDARY.split(content, 2)
    except ValueError:
        print('Front matter error:', content.splitlines()[0])
        return None, content
    fm = yaml.load(fm, Loader=yaml.SafeLoader)
    if not fm:
        fm = dict()
    return fm, text.strip()


def update_pages_data(path, pages):
    if len(pages) > 0 and path:
        with open(path, "r") as f:
            yml = yaml.load(f, Loader=yaml.SafeLoader)
        if not yml:
            yml = dict()
        if type(yml['pages']) is list:
            yml['pages'] = yml['pages'] + pages
        else:
            yml['pages'] = pages

        with open(path, "w") as f:
            yaml.safe_dump(yml, f, default_flow_style=False,
                           encoding='utf-8', allow_unicode=False)


def copy_assets(src, dest):
    assets = list()
    for asset in ['img', 'assets']:
        path = os.path.join(src, asset)
        if os.path.exists(path):
            copy_tree(path, os.path.join(dest, asset))
            assets.extend([os.path.join(asset, i) for i in os.listdir(path)])
    return assets


def make_docs(dest, data, headers, srcs):
    pages = list()
    for fname in srcs:
        with open(fname, 'r') as f:
            content = f.read().strip()
        if len(content) <= 0:
            continue

        fm, text = parse_frontmatter(content)

        # skip files with no front matter
        if fm == None:
            continue

        if 'label' in fm:
            label = fm['label']
        else:
            label = os.path.splitext(os.path.basename(fname))[0].lower()

        if label == 'readme':
            label = os.path.basename(os.path.dirname(fname)).lower()

        copy_assets(os.path.dirname(fname), dest)

        if 'page' in fm and len(text) > 0:
            # append text to another page
            page = fm['page']
            page_fname = os.path.join(dest, page + '.md')
            if os.path.exists(page_fname):
                h = headers
                while h > 0:
                    text = re.sub(r"^(#+)", r"\1#", text)
                    h = h - 1
                with open(page_fname, 'r') as f:
                    content = f.read().strip()
                content += '\n\n'
                content += text
                content += '\n'
                with open(page_fname, 'w') as f:
                    f.write(content)
                continue

        shutil.copy2(fname, os.path.join(dest, label + '.md'))
        pages.append(label)

    update_pages_data(data, pages)


def main():
    parser = argparse.ArgumentParser(
        description='Documentation maker for the APX system')
    parser.add_argument('--dest', action='store', required=True,
                        help='destination directory')
    parser.add_argument('--data', action='store', required=False,
                        help='update YAML data file')
    parser.add_argument('--headers', action='store', required=False, default=1,
                        help='Markdown headers level increment for nested pages')
    parser.add_argument('--srcs', nargs='+', required=True,
                        help='Markdown files to copy')
    args = parser.parse_args()
    make_docs(args.dest, args.data, args.headers, args.srcs)


if __name__ == "__main__":
    sys.exit(main())
