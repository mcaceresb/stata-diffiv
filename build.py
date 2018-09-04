#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# ---------------------------------------------------------------------
# Program: build.py
# Author:  Mauricio Caceres Bravo <mauricio.caceres.bravo@gmail.com>
# Created: Fri Aug 31 23:10:09 EDT 2018
# Updated: Fri Aug 31 23:10:09 EDT 2018
# Purpose: Main build file for diffiv (copies contents into ./build and
#          puts a .zip file in ./releases)

from os import makedirs, path, linesep, chdir, system, remove, rename
from shutil import copy2, rmtree
from sys import platform
from tempfile import gettempdir
from zipfile import ZipFile
from re import search
import argparse

# ---------------------------------------------------------------------
# Aux programs

try:
    from shutil import which
except:
    def which(program):
        import os

        def is_exe(fpath):
            return path.isfile(fpath) and os.access(fpath, os.X_OK)

        fpath, fname = path.split(program)
        if fpath:
            if is_exe(program):
                return program
        else:
            for epath in os.environ["PATH"].split(os.pathsep):
                epath = epath.strip('"')
                exe_file = path.join(epath, program)
                if is_exe(exe_file):
                    return exe_file

        return None


def makedirs_safe(directory):
    try:
        makedirs(directory)
        return directory
    except OSError:
        if not path.isdir(directory):
            raise


# ---------------------------------------------------------------------
# Command line parsing

parser = argparse.ArgumentParser()
parser.add_argument('--stata',
                    nargs    = 1,
                    type     = str,
                    metavar  = 'STATA',
                    default  = None,
                    required = False,
                    help     = "Path to stata executable")
parser.add_argument('--stata-args',
                    nargs    = 1,
                    type     = str,
                    metavar  = 'STATA_ARGS',
                    default  = None,
                    required = False,
                    help     = "Arguments to pass to Stata executable")
parser.add_argument('--make-flags',
                    nargs    = 1,
                    type     = str,
                    metavar  = 'MAKE_FLAGS',
                    default  = None,
                    required = False,
                    help     = "Arguments to pass to make")
parser.add_argument('--clean',
                    dest     = 'clean',
                    action   = 'store_true',
                    help     = "Clean build",
                    required = False)
parser.add_argument('--replace',
                    dest     = 'replace',
                    action   = 'store_true',
                    help     = "Replace build",
                    required = False)
parser.add_argument('--test',
                    dest     = 'test',
                    action   = 'store_true',
                    help     = "Run tests",
                    required = False)
parser.add_argument('--windows',
                    dest     = 'windows',
                    action   = 'store_true',
                    help     = "Compile for Windows from Unix environment.",
                    required = False)
args = vars(parser.parse_args())

# ---------------------------------------------------------------------
# Relevant files

diffiv_ssc = [
    "diffiv_mata.ado",
    "diffiv.ado",
    "diffiv.sthlp"
]

diffiv_zip = [
    "changelog.md",
    "diffiv.pkg",
    "stata.toc"
] + diffiv_ssc

diffiv_build = diffiv_zip + [
    "tests.do"
]

# ---------------------------------------------------------------------
# Run the script

# Remove buld
# -----------

if args['clean']:
    print("Removing build files")
    for bfile in diffiv_build:
        try:
            remove(path.join("build", bfile))
            print("\tdeleted " + bfile)
        except:
            try:
                remove(path.join("build", "diffiv", bfile))
                print("\tdeleted " + bfile)
            except:
                print("\t" + bfile + " not found")

    rc = system("make clean")
    exit(0)

makedirs_safe(path.join("build", "diffiv"))
makedirs_safe("releases")

# Stata executable
# ----------------

# I don't have stata on my global path, so to make the script portable
# I make it look for my local executable when Stata is not found.
if args['stata'] is not None:
    statadir = path.abspath(".")
    stataexe = args['stata'][0]
    statargs = "-b do" if args['stata_args'] is None else args['stata_args'][0]
    statado  = '"{0}" {1}'.format(stataexe, statargs)
elif which("stata") is None:
    statadir = path.expanduser("~/.local/stata13")
    stataexe = path.join(statadir, "stata")
    statargs = "-b do" if args['stata_args'] is None else args['stata_args']
    statado  = '"{0}" {1}'.format(stataexe, statargs)
else:
    statadir = path.abspath(".")
    stataexe = 'stata'
    statargs = "-b do" if args['stata_args'] is None else args['stata_args']
    statado  = '"{0}" {1}'.format(stataexe, statargs)

# Temporary files
# ---------------

maindir   = path.dirname(path.realpath(__file__))
tmpdir    = gettempdir()
tmpupdate = path.join(tmpdir, ".update_diffiv.do")

# Compile plugin files
# --------------------

if platform in ["linux", "linux2", "win32", "cygwin", "darwin"]:
    print("Trying to compile plugins for -diffiv-")
    print("(note: this assumes you copied libssl.a and libcrypto.a to lib/windows)")
    make_flags = args['make_flags'][0] if args['make_flags'] is not None else ""
    rc = system("make all {0}".format(make_flags))
    print("Success!" if rc == 0 else "Failed.")
else:
    print("Don't know platform '{0}'; compile manually.".format(platform))
    exit(198)

print("")

# Copy files to ./build
# ---------------------

gdir = path.join("build", "diffiv")
copy2("changelog.md", gdir)

copy2(path.join("src",  "diffiv.pkg"),      gdir)
copy2(path.join("src",  "stata.toc"),       gdir)
copy2(path.join("src",  "tests.do"),        gdir)
copy2(path.join("src",  "tests.do"),        "build")
copy2(path.join("src",  "diffiv_mata.ado"), gdir)
copy2(path.join("src",  "diffiv.ado"),      gdir)
copy2(path.join("docs", "diffiv.sthlp"),    gdir)

# Copy files to .zip folder in ./releases
# ---------------------------------------

# Get stata version
with open(path.join("src", "diffiv.ado"), 'r') as f:
    line    = f.readline()
    version = search('(\d+\.?)+', line).group(0)

plugins = ["diffiv_unix.plugin",
           "diffiv_windows.plugin",
           "diffiv_macosx.plugin"]
plugbak = plugins[:]
for plug in plugbak:
    if not path.isfile(path.join("build", plug)):
        alt = path.join("lib", "plugin", plug)
        if path.isfile(alt):
            copy2(alt, "build")
        else:
            print("Could not find '{0}'".format(plug))

chdir("build")
print("Compressing build files for diffiv-{0}".format(version))
if rc == 0:
    diffiv_anyplug = False
    for plug in plugbak:
        if path.isfile(plug):
            diffiv_anyplug = True
            rename(path.join(plug), path.join("diffiv", plug))
        else:
            plugins.remove(plug)
            print("\t'{0}' not found; skipping.".format(plug))

    if not diffiv_anyplug:
        print("WARNING: Could not find plugins despite build exit with 0 status.")
        exit(-1)

    diffiv_zip += plugins
else:
    print("WARNING: Failed to build plugins. Will exit.")
    exit(-1)

outzip = path.join(maindir, "releases", "diffiv-latest.zip".format(version))
with ZipFile(outzip, 'w') as zf:
    for zfile in diffiv_zip:
        zf.write(path.join("diffiv", zfile))
        print("\t" + path.join("diffiv", zfile))
        rename(path.join("diffiv", zfile), zfile)

chdir(maindir)
rmtree(path.join("build", "diffiv"))

# Copy files to send to SSC
# -------------------------

print("")
print("Compressing build files for diffiv-ssc.zip")
if rc == 0:
    diffiv_ssc += plugins
else:
    print("WARNING: Failed to build plugins. Will exit.")
    exit(-1)

chdir("build")
outzip = path.join(maindir, "releases", "diffiv-ssc.zip")
with ZipFile(outzip, 'w') as zf:
    for zfile in diffiv_ssc:
        zf.write(zfile)
        print("\t" + zfile)

# Replace package in ~/ado/plus
# -----------------------------

chdir(maindir)
if args["replace"]:
    if which(stataexe):
        with open(tmpupdate, 'w') as f:
            f.write("global builddir {0}".format(path.join(maindir, "build")))
            f.write(linesep)
            f.write("cap net uninstall diffiv")
            f.write(linesep)
            f.write("net install diffiv, from($builddir) replace")
            f.write(linesep)

        chdir(statadir)
        system(statado + " " + tmpupdate)
        remove(tmpupdate)
        # print(linesep + "Replaced diffiv in ~/ado/plus")
        chdir(maindir)
    else:
        print("Could not find Stata executable '{0}'.".format(stataexe))
        exit(-1)

# Run tests
# ---------

if args['test']:
    print("Running tests (see build/tests.log for output)")
    chdir("build")
    system(statado + " tests.do")
    chdir(maindir)
