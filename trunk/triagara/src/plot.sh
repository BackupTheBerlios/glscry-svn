#!/bin/sh

cat <<EOF | gnuplot
set size 1,1
set terminal png
set output "results.png"
plot "results.data" using 0:1 title "Imm" with lines,\
     "results.data" using 0:2 title "DL"  with lines,\
     "results.data" using 0:3 title "VA"  with lines,\
     "results.data" using 0:4 title "CVA" with lines,\
     "results.data" using 0:5 title "VBO" with lines
EOF
