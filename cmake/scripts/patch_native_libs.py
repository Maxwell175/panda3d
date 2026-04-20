#!/usr/bin/env python3
"""
patch_native_libs.py — strip version suffixes from shared-library metadata.

Prepares panda3d shared libraries for NuGet packaging: removes version numbers
from ELF/Mach-O metadata and fixes RPATH so consumers need neither
LD_LIBRARY_PATH nor DYLD_LIBRARY_PATH.

Phase 1 (_flatten): collapse each versioned symlink group (libfoo.so /
libfoo.so.1.11 / libfoo.so.1.11.0) into a single real unversioned file,
deleting the versioned variants.

Phase 2 (_patch_*): rewrite any versioned references inside ELF/Mach-O
metadata (DT_NEEDED, SONAME, install IDs, load commands) and set RPATH so
no LD_LIBRARY_PATH / DYLD_LIBRARY_PATH is needed.

Usage:
    python3 patch_native_libs.py <build-dir>
"""

import os
import platform
import re
import shutil
import subprocess
import sys
from pathlib import Path


def _so_base(name: str) -> str:
    return re.sub(r'\.so(\.\d+)+$', '.so', name)


def _dylib_base(name: str) -> str:
    return re.sub(r'\.\d[\d.]*\.dylib$', '.dylib', name)


def _run(*cmd: str) -> subprocess.CompletedProcess:
    return subprocess.run(cmd, capture_output=True, text=True)


def _patch_linux(path: str, known_bases: set[str]) -> None:
    r = _run('patchelf', '--print-needed', path)
    if r.returncode != 0:
        return
    for needed in r.stdout.splitlines():
        needed = needed.strip()
        base = _so_base(needed)
        if base != needed and base in known_bases:
            _run('patchelf', '--replace-needed', needed, base, path)

    r = _run('patchelf', '--print-soname', path)
    if r.returncode == 0 and r.stdout.strip():
        soname = r.stdout.strip()
        base = _so_base(soname)
        if base != soname:
            _run('patchelf', '--set-soname', base, path)

    _run('patchelf', '--set-rpath', '$ORIGIN', path)


def _patch_macos(path: str, known_bases: set[str]) -> None:
    r = _run('otool', '-D', path)
    if r.returncode != 0:
        return
    id_lines = [l.strip() for l in r.stdout.splitlines() if l.strip()]
    if len(id_lines) >= 2:
        cur_id = id_lines[1]
        fname = os.path.basename(cur_id)
        base_fname = _dylib_base(fname)
        if base_fname != fname and base_fname in known_bases:
            new_id = cur_id[: -len(fname)] + base_fname
            _run('install_name_tool', '-id', new_id, path)

    r = _run('otool', '-L', path)
    if r.returncode == 0:
        for line in r.stdout.splitlines()[1:]:
            parts = line.strip().split()
            if not parts:
                continue
            dep = parts[0]
            if not dep.startswith('@rpath/'):
                continue
            fname = dep[len('@rpath/'):]
            base_fname = _dylib_base(fname)
            if base_fname != fname and base_fname in known_bases:
                _run('install_name_tool', '-change', dep,
                     '@rpath/' + base_fname, path)

    # otool -l prints raw load commands; check for existing @loader_path entry
    r = _run('otool', '-l', path)
    if r.returncode == 0 and 'path @loader_path ' not in r.stdout:
        _run('install_name_tool', '-add_rpath', '@loader_path', path)


_SKIP = ('igate', 'supplemental')


def _flatten(root: Path, ver_pat: re.Pattern, base_fn,
             max_depth: int = 4) -> None:
    """Collapse each versioned library group into a single real unversioned file."""
    real_for: dict[Path, Path] = {}
    to_delete: list[Path] = []

    def _walk(d: Path, depth: int) -> None:
        if depth > max_depth:
            return
        try:
            entries = sorted(d.iterdir())
        except PermissionError:
            return
        for entry in entries:
            if not entry.is_symlink() and entry.is_dir():
                _walk(entry, depth + 1)
            elif entry.is_file() and ver_pat.search(entry.name):
                if any(s in entry.name for s in _SKIP):
                    continue
                unver = entry.parent / base_fn(entry.name)
                if not entry.is_symlink():
                    real_for[unver] = entry
                elif unver not in real_for:
                    real_for[unver] = entry.resolve()
                to_delete.append(entry)

    _walk(root, 1)

    for unver, src in real_for.items():
        if unver.is_symlink():
            unver.unlink()
        shutil.copy2(str(src), str(unver))
        print(f'  flattened {unver.name}')

    for path in to_delete:
        try:
            path.unlink()
        except FileNotFoundError:
            pass


def _iter_libs(root: Path, name_pat: re.Pattern,
               max_depth: int = 4) -> list[Path]:
    seen: set[str] = set()
    result: list[Path] = []

    def _walk(d: Path, depth: int) -> None:
        if depth > max_depth:
            return
        try:
            entries = sorted(d.iterdir())
        except PermissionError:
            return
        for entry in entries:
            if not entry.is_symlink() and entry.is_dir():
                _walk(entry, depth + 1)
            elif entry.is_file():  # True for symlinks-to-files too
                if any(s in entry.name for s in _SKIP):
                    continue
                if not name_pat.search(entry.name):
                    continue
                real = str(entry.resolve())
                if real in seen:
                    continue
                seen.add(real)
                result.append(Path(real))

    _walk(root, 1)
    return result


def main() -> None:
    if len(sys.argv) < 2:
        sys.exit(f'usage: {sys.argv[0]} <build-directory>')

    build_dir = Path(sys.argv[1])
    if not build_dir.is_dir():
        sys.exit(f'not a directory: {build_dir}')

    system = platform.system()
    if system == 'Linux':
        ver_pat = re.compile(r'\.so(\.\d+)+$')
        all_pat = re.compile(r'\.so(\.\d+)*$')
        base_fn = _so_base
        patch = _patch_linux
    elif system == 'Darwin':
        ver_pat = re.compile(r'\.\d[\d.]*\.dylib$')
        all_pat = re.compile(r'\.dylib$')
        base_fn = _dylib_base
        patch = _patch_macos
    else:
        sys.exit(f'unsupported platform: {system}')

    _flatten(build_dir, ver_pat, base_fn)

    libs = _iter_libs(build_dir, all_pat)
    known_bases = {lib.name for lib in libs}

    for lib in libs:
        print(f'  patching {lib}')
        patch(str(lib), known_bases)
    print(f'patched {len(libs)} libraries in {build_dir}')


if __name__ == '__main__':
    main()
