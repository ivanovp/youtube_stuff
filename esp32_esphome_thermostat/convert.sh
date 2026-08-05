#!/bin/sh
pandoc README.md -o README.pdf --pdf-engine=pdflatex -V geometry:margin=1cm
