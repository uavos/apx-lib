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
        return dict(), content
    # Parse text with frontmatter, extract metadata and content.
    try:
        _, fm, text = FM_BOUNDARY.split(content, 2)
    except ValueError:
        print('YAML error:', content.splitlines()[0])
        return dict(), content
    fm = yaml.load(fm, Loader=yaml.SafeLoader)
    if not fm:
        fm = dict()
    return fm, text.strip()


def main():
    parser = argparse.ArgumentParser(
        description='Documentation maker for the APX system.')
    parser.add_argument('--dest', action='store', required=True,
                        help='destination directory')
    parser.add_argument('--data', action='store', required=False,
                        help='update YAML data file')
    parser.add_argument('--srcs', nargs='+', required=True,
                        help='Markdown files to copy')
    args = parser.parse_args()

    pages = list()
    for fname in args.srcs:
        with open(fname, 'r') as f:
            content = f.read().strip()
        if len(content) <= 0:
            continue

        fm, text = parse_frontmatter(content)

        if 'label' in fm:
            label = fm['label']
        else:
            label = os.path.splitext(os.path.basename(fname))[0].lower()

        if label == 'readme':
            label = os.path.basename(os.path.dirname(fname)).lower()

        assets = os.path.join(os.path.dirname(fname), 'img')
        if os.path.exists(assets):
            copy_tree(assets, os.path.join(
                args.dest, os.path.basename(assets)))

        if 'page' in fm and len(text) > 0:
            # append text to another page
            page = fm['page']
            page_fname = os.path.join(args.dest, page+'.md')
            if os.path.exists(page_fname):
                text = re.sub(r"^(#+)", r"\1#", text)
                with open(page_fname, 'r') as f:
                    content = f.read().strip()
                content += '\n\n'
                content += text
                content += '\n'
                with open(page_fname, 'w') as f:
                    f.write(content)
                continue

        shutil.copy2(fname, os.path.join(args.dest, label+'.md'))
        pages.append(label)

    if len(pages) > 0 and args.data:
        with open(args.data, "r") as f:
            yml = yaml.load(f, Loader=yaml.SafeLoader)
        yml['pages'] = list(yml['pages'])+pages
        with open(args.data, "w") as f:
            yaml.safe_dump(yml, f, default_flow_style=False,
                           encoding='utf-8', allow_unicode=False)


if __name__ == "__main__":
    sys.exit(main())
