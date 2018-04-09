#!/bin/bash

doxygen doxygen.cfg
cp -r ../client/* .
git add *
cd html/
git commit -m "update docs"
git push origin gh-pages
