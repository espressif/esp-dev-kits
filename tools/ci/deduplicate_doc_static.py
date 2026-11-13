# SPDX-FileCopyrightText: 2026 Espressif Systems (Shanghai) CO LTD
# SPDX-License-Identifier: Apache-2.0
"""Deduplicate _static files across the sites in an esp-docs deployment.

Use --deploy in CI to wrap the pinned esp-docs deployer. Optimization happens
in a private staging tree, with URLs calculated for language/version/target,
not the language/target/html layout of the build artifacts.
"""

import argparse
from collections import defaultdict
from dataclasses import dataclass
import fnmatch
import hashlib
import html
import math
import mimetypes
import os
from pathlib import Path
import posixpath
import re
import shutil
import sys
import tempfile
import threading
import time
from urllib.parse import quote, unquote, urlsplit


TEXT_SUFFIXES = {'.html', '.htm', '.css', '.js', '.mjs', '.json', '.svg', '.xml', '.map', '.txt'}
# Match URL-bearing syntax without serializing/reformatting HTML or JavaScript.
QUOTED = re.compile(r'''(["'`])([^"'`\r\n]*)\1''')
CSS_URL = re.compile(r'''url\(\s*(["']?)([^\s"')]+)\1\s*\)''', re.I)
UNQUOTED_ATTR = re.compile(r'''\b(?:src|href|poster|data|action)\s*=\s*([^\s"'`=<>]+)''', re.I)
SRCSET = re.compile(r'''\bsrcset\s*=\s*(["'])(.*?)\1''', re.I | re.S)
SOURCE_MAP = re.compile(r'[#@]\s*sourceMappingURL=([^\s*]+)')
RAW_TEXT = re.compile(r'<(?:script|style)\b[^>]*>(.*?)</(?:script|style)\s*>', re.I | re.S)


class Progress:
    """Line-oriented, flushed heartbeat; works without a TTY and during gzip."""

    def __init__(self, interval=5.0):
        self.interval = interval
        self.started = time.monotonic()
        self.status = 'Starting'
        self.stop = threading.Event()
        self.lock = threading.Lock()

    def log(self, message):
        with self.lock:
            print('[static-dedup +{:.0f}s] {}'.format(time.monotonic() - self.started, message), flush=True)

    def phase(self, message):
        self.status = message
        self.log(message)

    def update(self, message):
        self.status = message

    def _heartbeat(self):
        while not self.stop.wait(self.interval):
            self.log(self.status)

    def __enter__(self):
        self.thread = threading.Thread(target=self._heartbeat, daemon=True)
        self.thread.start()
        return self

    def __exit__(self, *args):
        self.stop.set()
        self.thread.join()


@dataclass(frozen=True)
class Reference:
    start: int
    end: int
    target: Path
    base: str
    suffix: str
    escape_html: bool


class Layout:
    def __init__(self, build_dir, version, url_base=''):
        if not version or version in ('.', '..') or '/' in version or '\\' in version:
            raise ValueError('Deployment version must be a single nonempty path component')
        self.build_dir = Path(build_dir).resolve()
        self.version = version
        self.url_base = urlsplit(url_base.rstrip('/') + '/') if url_base else None
        self.sites = sorted(self.build_dir.glob('*/*/html'))
        self.sites = [p for p in self.sites if p.is_dir()]
        if not self.sites:
            raise ValueError('No language/target/html sites found in {}'.format(build_dir))
        self.published = {}
        self.by_url = {}
        self.site_root = {}
        for site in self.sites:
            target, language = site.parent.name, site.parent.parent.name
            root = '{}/{}'.format(language, version)
            if target != 'generic':
                root += '/' + target
            for path in sorted(site.rglob('*')):
                if path.is_symlink():
                    raise ValueError('Symlinks are not supported in HTML artifacts: {}'.format(path))
                if not path.is_file() or '_sources' in path.relative_to(site).parts:
                    continue
                published = root + '/' + path.relative_to(site).as_posix()
                if published in self.by_url:
                    raise ValueError('Overlapping deployment paths: {}'.format(published))
                self.published[path] = published
                self.by_url[published] = path
                self.site_root[path] = root

    def resolve(self, source, value):
        value = html.unescape(value).replace('\\/', '/')
        if not value or '${' in value or value.startswith('#'):
            return None
        try:
            url = urlsplit(value)
        except ValueError:
            return None
        if url.scheme or url.netloc or url.path.startswith('/'):
            base = self.url_base
            if not base or (url.scheme and url.scheme != base.scheme) or (url.netloc and url.netloc != base.netloc):
                return None
            path = unquote(url.path)
            prefix = unquote(base.path)
            if not path.startswith(prefix):
                return None
            target = self.by_url.get(path[len(prefix):])
            origin = posixpath.dirname(self.published[source])
        else:
            origin = posixpath.dirname(self.published[source])
            path = unquote(url.path)
            target = self.by_url.get(posixpath.normpath(posixpath.join(origin, path)))
            # Sphinx inline JS also uses URL_ROOT + '_static/...'. The literal
            # in that expression is relative to the site, not to a nested page.
            if target is None and path.startswith('_static/'):
                origin = self.site_root[source]
                target = self.by_url.get(posixpath.join(origin, path))
        if target is None:
            return None
        suffix = ('?' + url.query if url.query else '') + ('#' + url.fragment if url.fragment else '')
        return target, origin, suffix


def references(layout, path, content):
    """Find literal local URLs, including srcset, inline CSS/JS and SVG hrefs."""
    spans = []
    for pattern, group in ((QUOTED, 2), (CSS_URL, 2), (UNQUOTED_ATTR, 1), (SOURCE_MAP, 1)):
        spans.extend((m.start(group), m.end(group)) for m in pattern.finditer(content))
    for match in SRCSET.finditer(content):
        value = match.group(2)
        cursor = 0
        while cursor < len(value):
            while cursor < len(value) and (value[cursor].isspace() or value[cursor] == ','):
                cursor += 1
            start = cursor
            while cursor < len(value) and not value[cursor].isspace():
                cursor += 1
            stop = cursor
            while stop > start and value[stop - 1] == ',':
                stop -= 1
            spans.append((match.start(2) + start, match.start(2) + stop))
            if stop == cursor:
                # Skip density/width descriptors, not commas inside data URLs.
                while cursor < len(value) and value[cursor] != ',':
                    cursor += 1
    raw_text = [m.span(1) for m in RAW_TEXT.finditer(content)] if path.suffix.lower() in {'.html', '.htm'} else []
    found = []
    end = -1
    for start, stop in sorted(set(spans)):
        resolved = layout.resolve(path, content[start:stop])
        if resolved is None or start < end:
            continue
        target, base, suffix = resolved
        escape_html = path.suffix.lower() in {'.html', '.htm', '.svg', '.xml'}
        if any(a <= start < b for a, b in raw_text):
            escape_html = False
        found.append(Reference(start, stop, target, base, suffix, escape_html))
        end = stop
    return found


def digest(path):
    result = hashlib.sha256()
    with path.open('rb') as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b''):
            result.update(chunk)
    return result.digest()


def atomic_write(path, content):
    # Staging files may be hardlinks to build artifacts. Never truncate them.
    with tempfile.NamedTemporaryFile(dir=path.parent, delete=False) as stream:
        temp = Path(stream.name)
        try:
            stream.write(content.encode('utf-8'))
            stream.flush()
            shutil.copymode(path, temp)
            os.replace(str(temp), str(path))
        finally:
            if temp.exists():
                temp.unlink()


def skip_empty_build(build_dir, allow_empty, progress):
    """Skip intentional incremental no-ops before creating an archive or deploying."""
    root = Path(build_dir)
    if root.exists() and not root.is_dir():
        raise ValueError('Build output root is not a directory: {}'.format(root))
    sites = sorted(path for path in root.glob('*/*/html') if path.is_dir())
    if sites:
        for site in sites:
            if not (site / 'index.html').is_file():
                raise ValueError('Incomplete HTML site (missing index.html): {}'.format(site))
        return False
    if any(path.is_file() for path in root.glob('*/*/latex/build/*.pdf')):
        raise ValueError('Found PDFs but no language/target/html sites in {}'.format(root))
    if not allow_empty:
        raise ValueError('No language/target/html sites found in {}'.format(root))
    progress.phase('[document deployment skipped] No HTML or PDF build outputs in {}; '
                   'nothing to optimize or deploy'.format(root))
    return True


def optimize(build_dir, version, progress, url_base='', keep=(), dry_run=False, allow_empty=False):
    if skip_empty_build(build_dir, allow_empty, progress):
        return {'duplicates': 0, 'saved_bytes': 0, 'changed_urls': 0}
    progress.phase('Discovering HTML sites and static resources')
    layout = Layout(build_dir, version, url_base)
    static_files = [p for p in layout.published if '_static' in p.relative_to(layout.build_dir).parts]
    groups = defaultdict(list)
    total_bytes = sum(p.stat().st_size for p in static_files)
    progress.phase('Hashing {} static files across {} sites ({:.1f} MiB)'.format(
        len(static_files), len(layout.sites), total_bytes / 2**20))
    processed_bytes = 0
    for index, path in enumerate(static_files, 1):
        relative = path.relative_to(layout.build_dir).as_posix()
        if any(fnmatch.fnmatchcase(relative, pattern) for pattern in keep):
            key = ('keep', relative)
        else:
            # Retain the HTTP content type (e.g. empty .js and .css differ).
            key = (mimetypes.guess_type(path.name)[0] or path.suffix.lower(), digest(path))
        groups[key].append(path)
        processed_bytes += path.stat().st_size
        progress.update('Hashing {}/{} files, {:.1f}/{:.1f} MiB'.format(
            index, len(static_files), processed_bytes / 2**20, total_bytes / 2**20))

    progress.phase('Reading HTML, CSS, JavaScript, SVG and other text references')
    texts, refs = {}, {}
    for index, path in enumerate(layout.published, 1):
        if path.suffix.lower() in TEXT_SUFFIXES:
            try:
                content = path.read_bytes().decode('utf-8')
            except UnicodeDecodeError:
                # Unknown encodings must not be moved to another URL context.
                if path in static_files:
                    raise ValueError('Non-UTF-8 text resource: {}'.format(path))
                continue
            texts[path] = content
            refs[path] = references(layout, path, content)
        progress.update('Reading references: {}/{} files'.format(index, len(layout.published)))

    # Identical CSS/SVG/etc. can have different meanings when their relative
    # dependencies differ. Refine content groups until those contexts agree.
    partitions = list(groups.values())
    progress.phase('Checking relative dependencies of duplicate text resources')
    while True:
        canonical = {p: group[0] for group in partitions for p in group}
        refined = []
        for group in partitions:
            contexts = defaultdict(list)
            for path in group:
                signature = tuple(canonical.get(ref.target, ref.target) for ref in refs.get(path, ()))
                contexts[signature].append(path)
            refined.extend(contexts.values())
        if len(refined) == len(partitions):
            break
        partitions = refined
        progress.update('Checking dependency contexts: {} unique resource groups'.format(len(partitions)))

    duplicates = {p: first for p, first in canonical.items() if p != first}
    saved = sum(p.stat().st_size for p in duplicates)
    progress.phase('{}: {} duplicate files, {:.1f} MiB removable'.format(
        'Dry run' if dry_run else 'Rewriting resource URLs', len(duplicates), saved / 2**20))
    changed_files, changed_urls = 0, 0
    for index, (path, content) in enumerate(texts.items(), 1):
        if path in duplicates:
            continue
        replacements = []
        for ref in refs[path]:
            destination = duplicates.get(ref.target)
            if destination is None:
                continue
            new_path = posixpath.relpath(layout.published[destination], ref.base)
            value = quote(new_path, safe='/.-_~') + ref.suffix
            if ref.escape_html:
                value = html.escape(value, quote=True)
            # Check the replacement in the deployed layout before writing.
            resolved = layout.by_url.get(posixpath.normpath(posixpath.join(ref.base, unquote(new_path))))
            if resolved != destination:
                raise ValueError('Cannot safely rewrite {} in {}'.format(content[ref.start:ref.end], path))
            replacements.append((ref.start, ref.end, value))
        if replacements:
            for start, end, value in reversed(replacements):
                content = content[:start] + value + content[end:]
            if not dry_run:
                atomic_write(path, content)
            changed_files += 1
            changed_urls += len(replacements)
        progress.update('Rewriting references: {}/{} text files, {} URLs changed'.format(index, len(texts), changed_urls))

    progress.phase('Removing duplicate static files' if not dry_run else 'Finishing dry run (no files changed)')
    if not dry_run:
        for index, path in enumerate(duplicates, 1):
            path.unlink()
            progress.update('Removing duplicates: {}/{} files'.format(index, len(duplicates)))
    progress.log('Summary: {} sites; {} -> {} static files; {:.1f} MiB saved; {} URLs in {} files {}'.format(
        len(layout.sites), len(static_files), len(static_files) - len(duplicates), saved / 2**20,
        changed_urls, changed_files, 'would be rewritten' if dry_run else 'rewritten'))
    return {'duplicates': len(duplicates), 'saved_bytes': saved, 'changed_urls': changed_urls}


def link_or_copy(source, destination):
    try:
        os.link(source, destination)
    except OSError:
        shutil.copy2(source, destination)
    return destination


def deploy(progress, keep=(), allow_empty=False):
    build_dir = os.path.expandvars(os.environ.get('DOCS_BUILD_DIR', ''))
    if not build_dir.strip():
        raise ValueError('DOCS_BUILD_DIR must be set for --deploy')
    if skip_empty_build(build_dir, allow_empty, progress):
        return

    from esp_docs import deploy_docs

    original_pack = deploy_docs.build_doc_tarball

    def optimized_pack(version, git_ver, build_dir):
        root = Path(build_dir).resolve()
        layout = Layout(root, version)
        # Keep the original artifacts intact, including when esp-docs deploys
        # the same build a second time under the 'stable' version.
        with tempfile.TemporaryDirectory(prefix='.static-dedup-', dir=root) as temp:
            staging = Path(temp) / 'build'
            progress.phase('Staging {} HTML sites (copy on write)'.format(len(layout.sites)))
            for index, site in enumerate(layout.sites, 1):
                shutil.copytree(str(site), str(staging / site.relative_to(root)),
                                copy_function=link_or_copy, ignore=shutil.ignore_patterns('_sources'))
                progress.update('Staging HTML sites: {}/{}'.format(index, len(layout.sites)))
            for pdf in sorted(root.glob('*/*/latex/build/*.pdf')):
                destination = staging / pdf.relative_to(root)
                destination.parent.mkdir(parents=True, exist_ok=True)
                link_or_copy(str(pdf), str(destination))
            optimize(staging, version, progress, os.environ.get('DOCS_DEPLOY_URL_BASE', ''), keep)
            progress.phase('Compressing optimized documentation for {}'.format(version))
            archive, urls = original_pack(version, git_ver, str(staging))
            destination = root / Path(archive).name
            os.replace(archive, str(destination))
            progress.log('Archive: {} ({:.1f} MiB)'.format(destination.name, destination.stat().st_size / 2**20))
        progress.phase('Uploading/extracting documentation with esp-docs')
        return str(destination), urls

    # esp-docs 2.2.0 has no pre-archive hook. Replace only its packaging
    # callback; keep version naming, SSH/SCP deployment and preview output.
    deploy_docs.build_doc_tarball = optimized_pack
    try:
        deploy_docs.main()
    finally:
        deploy_docs.build_doc_tarball = original_pack


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('build_dir', nargs='?', help='Build output root; modified in place without --deploy')
    parser.add_argument('--version', help='Deployment directory name (required without --deploy)')
    parser.add_argument('--url-base', default=os.environ.get('DOCS_DEPLOY_URL_BASE', ''), help='Site deployment URL prefix')
    parser.add_argument('--deploy', action='store_true', help='Stage, optimize and run the installed esp-docs deployer')
    parser.add_argument('--dry-run', action='store_true', help='Report duplicates without changing files')
    parser.add_argument('--allow-empty', action='store_true',
                        help='Succeed without packaging or deploying if incremental builds produced no HTML or PDFs')
    parser.add_argument('--keep', action='append', default=[], metavar='GLOB',
                        help='Keep matching files at their own URLs; relative to build_dir, repeatable')
    parser.add_argument('--progress-interval', type=float, default=5.0, metavar='SECONDS')
    args = parser.parse_args()
    if not math.isfinite(args.progress_interval) or args.progress_interval <= 0:
        parser.error('--progress-interval must be positive')
    if args.deploy and (args.build_dir or args.version or args.dry_run):
        parser.error('--deploy uses esp-docs environment variables; do not pass build_dir, --version or --dry-run')
    if not args.deploy and (not args.build_dir or not args.version):
        parser.error('build_dir and --version are required without --deploy')
    with Progress(args.progress_interval) as progress:
        try:
            if args.deploy:
                deploy(progress, args.keep, args.allow_empty)
            else:
                optimize(args.build_dir, args.version, progress, args.url_base, args.keep, args.dry_run, args.allow_empty)
        except (OSError, ValueError) as error:
            progress.log('ERROR: {}'.format(error))
            return 1
    return 0


if __name__ == '__main__':
    sys.exit(main())
