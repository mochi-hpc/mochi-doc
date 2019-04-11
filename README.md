Mochi documentation
===================

This repository contains a Sphinx-based documentation
for the Mochi libraries: Margo, Thallium, Argobots, Mercury.

To contribute to this documentation, make sure that you
have Sphinx installed as well as the ReadTheDoc theme.
These can be installed as follows using `pip`.

```
pip install sphinx
pip install sphinx_rtd_theme
```

Once you have these dependencies installed, clone this
repository and cd into it. You can change the documentation
by editing the files in the source subdirectory (these files
use the .rst format). You can build the documentation
using the following command.

```
make html
```

And check the result by opening the `build/index.html` page.
