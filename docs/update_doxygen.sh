#!/bin/bash

doxygen doxygen.cfg
cd html/
cp -r ../../client .
git add *
git commit -m "update docs"
git push origin gh-pages
