# conf.py — Sphinx configuration

# -- Project information -----------------------------------------------------
project = "neutros-fc"
author = "Cory McKiel"
copyright = "2026, Cory McKiel"
release = "0.0.1"

# -- General configuration ---------------------------------------------------
extensions = [
    "sphinx_needs",          # requirements / spec objects
    "sphinxcontrib.plantuml",  # UML from .puml / uml:: blocks
    "breathe",               # Doxygen -> Sphinx bridge
]

exclude_patterns = ["_build", "Thumbs.db", ".DS_Store"]

# -- HTML output -------------------------------------------------------------
html_theme = "furo"

# -- PlantUML (sphinxcontrib-plantuml) ---------------------------------------
# The apt 'plantuml' package puts a CLI wrapper on PATH, so just call it.
# (If you pinned a plantuml.jar instead, use: plantuml = "java -jar /path/plantuml.jar")
plantuml = "plantuml"
plantuml_output_format = "svg"

# -- Breathe (Doxygen -> Sphinx bridge) --------------------------------------
# Uncomment and point at your Doxygen XML once Doxygen is emitting it.
# breathe_projects = {"neutros-fc": "_doxygen/xml"}
# breathe_default_project = "neutros-fc"
