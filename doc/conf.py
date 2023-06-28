# Configuration file for the Sphinx documentation builder.
#
# For the full list of built-in configuration values, see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html
import subprocess, os
# -- Project information -----------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#project-information

project = 'SystemC Components Library'
copyright = '2023, MINRES Technologies GmbH'
author = 'Eyck Jentzsch'
release = '2022.08'

# -- General configuration ---------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#general-configuration

extensions = [ "breathe" ]

templates_path = ['_templates']
exclude_patterns = ['_build', 'Thumbs.db', '.DS_Store']



# -- Options for HTML output -------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#options-for-html-output

#html_theme = 'alabaster'
html_theme = 'classic'
html_static_path = ['_static']
html_theme_options = {
    "leftsidebar": "true",
    "relbarbgcolor": "black"
}
# Breathe Configuration
breathe_default_project = "SCC"
breathe_domain_by_extension = {"h" : "cpp"}
highlight_language = 'c++'
primary_domain = 'cpp'

