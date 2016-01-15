#!/bin/bash

doxygen doxygen.cfg
cd html/
git add *
git commit -m "update docs"
git push origin gh-pages
