#!/bin/sh

cat <<EOF | gnuplot
set size 1,1
set terminal png
set output "results.png"
plot "results.data" using 0:1 smooth bezier title "Imm" with lines
EOF


#     "results.data" using 0:2 smooth bezier title "DL"  with lines,\
#     "results.data" using 0:3 smooth bezier title "VA"  with lines,\
#     "results.data" using 0:4 smooth bezier title "CVA" with lines,\
#     "results.data" using 0:5 smooth bezier title "VBO" with lines
