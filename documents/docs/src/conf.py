"""Sphinx configuration for the OpenSync documentation."""

import os
from pathlib import Path
import shutil
import sys

# Keep import caches inside the documentation build, including Matplotlib's.
sys.dont_write_bytecode = True
DOCS_DIR = Path(__file__).resolve().parents[1]
# Document this checkout, rather than an older installed copy of OpenSync.
sys.path.insert(0, str(DOCS_DIR.parents[1] / "software"))
os.environ.setdefault("MPLCONFIGDIR", str(DOCS_DIR / "_build" / "matplotlib"))
os.environ.setdefault("MPLBACKEND", "Agg")

# pip's pypandoc-binary supplies Pandoc without a system-wide installation.
if shutil.which("pandoc") is None:
    from pypandoc import get_pandoc_path

    pandoc_directory = str(Path(get_pandoc_path()).parent)
    os.environ["PATH"] = pandoc_directory + os.pathsep + os.environ.get("PATH", "")

project = "OpenSync"
copyright = "2026, OpenPIV"
author = "Erich Zimmer"
release = "0.1.0"
language = "en"
root_doc = "index"

extensions = [
    "sphinx.ext.autodoc",
    "sphinx.ext.autosummary",
    "sphinx.ext.githubpages",
    "sphinx.ext.napoleon",
    "nbsphinx",
    "sphinx_design",
]
exclude_patterns = ["**/.ipynb_checkpoints/**", "Thumbs.db", ".DS_Store"]
napoleon_google_docstring = False
autodoc_typehints = "none"

# Render saved notebook content. Building docs must never operate a USB device.
nbsphinx_execute = "never"

# The same theme used by NumPy's documentation.
html_theme = "pydata_sphinx_theme"
html_title = "OpenSync"
html_baseurl = "https://openpiv.net/OpenSync/"
html_static_path = ["_static"]
html_css_files = ["opensync.css"]
html_logo = "_static/opensync_logo.png"
html_favicon = "_static/opensync_logo.png"
html_context = {"default_mode": "light"}
html_sidebars = {"index": [], "examples/index": [], "user_manual/index": []}
html_theme_options = {
    "logo": {"text": "OpenSync"},
    "navbar_align": "left",
    "show_prev_next": False,
    "navigation_depth": 3,
    "icon_links": [
        {
            "name": "GitHub",
            "url": "https://github.com/ErichZimmer/OpenSync",
            "icon": "fa-brands fa-github",
        },
    ],
}


def add_user_manual(app, docname, source):
    """Offer the manual only when its compiled PDF is present."""
    if docname != "user_manual/index":
        return

    manual = DOCS_DIR / "user_manual" / "opensync_user_manual.pdf"
    app.env.note_dependency(str(manual))
    if manual.is_file():
        source[0] += (
            "\n:download:`Download the OpenSync user manual (PDF) "
            "<../../user_manual/opensync_user_manual.pdf>`\n"
        )
    else:
        source[0] += (
            "\n.. note::\n\n"
            "   **Placeholder:** the standalone user manual is being prepared. Its PDF download will "
            "appear here when it is available.\n"
        )


def format_opensync_docstrings(app, what, name, obj, options, lines):
    """Repair existing docstring markup for rendering without editing the library."""
    if not name.startswith("opensync."):
        return

    for i in range(len(lines) - 1):
        if lines[i + 1] and set(lines[i + 1]) == {"-"}:
            heading = lines[i].rstrip(":")
            if heading == "Example":
                heading = "Examples"
            lines[i] = heading
            lines[i + 1] = "-" * len(heading)

    if name.rsplit(".", 1)[-1] in {"get_clock_params", "get_pulse_params"}:
        formatted = []
        for line in lines:
            if line.startswith("    - "):
                formatted.append("")
            elif line.startswith("        "):
                line = line[2:]
            formatted.append(line)
        lines[:] = formatted


def setup(app):
    app.connect("autodoc-process-docstring", format_opensync_docstrings, priority=100)
    app.connect("source-read", add_user_manual)
