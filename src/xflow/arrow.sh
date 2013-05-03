#! /bin/bash

for a in `seq 0 6` ; do
    cat > tmp.fig <<EOF
#FIG 3.2  Produced by xfig version 3.2.5
Landscape
Center
Metric
A4      
100.00
Single
-2
1200 2
2 1 0 1 0 7 50 -1 -1 0.000 0 0 -1 0 1 2
	$a 0 1.00 60.00 120.00
	 1260 3870 1395 4050
EOF
    fig2dev -L png -m 1.5 tmp.fig > arrow$a.png 
done
rm -f tmp.fig
