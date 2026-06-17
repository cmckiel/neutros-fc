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

# -- make sphinx-needs adapt to Furo dark mode ---------------------------
html_static_path = ["_static"]
html_css_files = ["needs_dark.css"]

# -- PlantUML (sphinxcontrib-plantuml) ---------------------------------------
# The apt 'plantuml' package puts a CLI wrapper on PATH, so just call it.
# (If you pinned a plantuml.jar instead, use: plantuml = "java -jar /path/plantuml.jar")
plantuml = "plantuml"
plantuml_output_format = "svg"

# -- Breathe (Doxygen -> Sphinx bridge) --------------------------------------
# Uncomment and point at your Doxygen XML once Doxygen is emitting it.
breathe_projects = {"neutros-fc": "doxygen/xml"}
breathe_domain_by_extension = {"h": "c", "c": "c"}
breathe_default_project = "neutros-fc"

# -- sphinx-needs: need types --------------------------------------------
needs_types = [
    dict(directive="conops", title="ConOps",          prefix="CO_",  color="#BFD8D2", style="node"),
    dict(directive="sysreq", title="System Req",      prefix="SYS_", color="#FEDCD2", style="node"),
    dict(directive="hlr",    title="High-Level Req",  prefix="HLR_", color="#DF744A", style="node"),
    dict(directive="llr",    title="Low-Level Req",   prefix="LLR_", color="#DCB239", style="node"),
    dict(directive="design", title="Design",          prefix="DSN_", color="#9CC2E5", style="node"),
    dict(directive="impl",   title="Implementation",  prefix="IMP_", color="#A5D6A7", style="node"),
    dict(directive="test",   title="Test",            prefix="TST_", color="#CE93D8", style="node"),
]

# -- sphinx-needs: link (trace) types ------------------------------------
# NOTE: 8.x dict form. Do NOT use needs_extra_links (deprecated list form).
needs_links = {
    "satisfies":  dict(incoming="is satisfied by",   outgoing="satisfies"),
    "refines":    dict(incoming="is refined by",     outgoing="refines"),
    "implements": dict(incoming="is implemented by", outgoing="implements"),
    "verifies":   dict(incoming="is verified by",    outgoing="verifies"),
}

needs_statuses = [
    dict(name="draft",    description="Being written"),
    dict(name="open",     description="Approved, not yet implemented"),
    dict(name="done",     description="Implemented and verified"),
]
