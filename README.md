![build](https://github.com/mochi-hpc/mochi-doc/actions/workflows/build.yml/badge.svg)

# Mochi documentation

This repository contains a Sphinx-based documentation
for the Mochi libraries: Margo, Thallium, Argobots, Mercury,
ABT-IO, and SSG, as well as corresponding code examples.

## Building the documentation

To build and/or contribute to this documentation, you must have a Sphinx and
a few related extensions installed.  These can be installed as follows using
Python's `pip`.

```
pip install sphinx
pip install sphinx_rtd_theme
pip install sphinx_copybutton
pip install recommonmark
pip install breathe
```

You must also install the `doxygen` documentation system.  This is likely
available in your platform's primary package manager.  For example on Ubuntu:

```
sudo apt install doxygen
```

Once you have these dependencies installed, clone this
repository and cd into it. You can change the documentation
by editing the files in the source subdirectory (these files
use the .rst format). You can build the documentation
using the following command.

```
cd docs
make html
```

And check the result by opening the `build/index.html` page
that has been created in the docs directory.

## Building the code examples

To build the code, you will need spack and the
[mochi repo](https://github.com/mochi-hpc/mochi-spack-packages) setup.

```
cd code
spack env create mochi-doc-env spack.yaml
spack env activate mochi-doc-env
spack install
mkdir build
cd build
cmake .. -DCMAKE_CXX_COMPILER=mpicxx -DCMAKE_C_COMPILER=mpicc
make
```
