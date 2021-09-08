import setuptools 

with open("README.md", "r", encoding="utf-8") as fh:
    long_description = fh.read()

setuptools.setup(
    name="PySysC-SCC",
    version="0.0.1",
    author="MINRES Technologies GmbH",
    author_email="info@minres.com",
    description="SCC python modules for intergration in PySysC",
    long_description=long_description,
    long_description_content_type="text/markdown",
    url="https://github.com/Minres/SystemC-Components.git",
    project_urls={
        "Bug Tracker": "https://github.com/Minres/SystemC-Components.git/issues",
    },
    classifiers=[
        "Development Status :: 3 - Alpha",
        "Programming Language :: Python :: 3.6",
        "License :: OSI Approved :: Apache Software License",
        "Operating System :: OS Independent",
        "Topic :: Scientific/Engineering :: Electronic Design Automation (EDA)",
    ],
    package_dir={"": "src"},
    packages=setuptools.find_namespace_packages(where="src"),
    python_requires=">=3.6",
    install_requires=[
        'PySysC'
        ],
)