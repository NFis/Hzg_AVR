set term png font arial 10 size 1000,760
#set term win
set output "D:\\test\\plot.png"
#file="D:\\temp\\hzgdaten\\20110111.dat"
set timefmt "%d.%m.%y %H:%M:%S"
set xdata time
set datafile missing "00.00.00"
#set format x "%d.%m.\n%H:%M:%S"
set format x "%d.%m.\n%H:%M"
set autoscale
#set xrange ["28.03.08 15:00:00":]
set y2range [0:180]
set yrange[-10:70]
set y2tics
set grid
#set key left horizontal
set key off
#set mouse format "%5.1" mouseformat 5 clipboardformat 5 labels
set mouse format "%5.1f" mouseformat 3 clipboardformat 3 nolabels
# AGT, sollwert, RLT, VLT, KT, WWT_mw, RT_mw_kurz, RT_mw_lang, RJuT, AT_mw_lang, AT_mw_Tag
plot file using 1: 4 axes x1y2 with lines t "AGT"    	lw 1 ,\
      file using 1: 5 axes x1y1 with lines  t "Soll" 		lw 1 lc rgb "black" ,\
      file using 1: 6 axes x1y1 with lines  t "RLT"               	lw 1 lc rgb "blue" ,\
      file using 1: 7 axes x1y1 with lines  t "VLT"               	lw 1 lc rgb "orange" ,\
      file using 1: 8 axes x1y1 with lines  t "KT"               	lw 1 lc rgb "red"  ,\
      file using 1: 9 axes x1y1 with lines  t "WWT     "	lw 2 lc rgb "dark-blue" ,\
      file using 1:10 axes x1y1 with lines t "RTmwk"	lw 1 lc 18 ,\
      file using 1:11 axes x1y1 with lines t "RTmwlang"	lw 1,\
      file using 1:12 axes x1y1 with lines t "RJuT" 		lw 1 lc rgb "violet" ,\
      file using 1:13 axes x1y1 with lines t "AT_mw_lang"	lw 1 ,\
      file using 1:14 axes x1y1 with lines t "AT-mw_Tag"        lw 1, \
      file using 1:(0.01*$15) axes x1y1 with lines t "debug1"     lw 2 lc rgb "black" , \
      file using 1:(2.5*$16) axes x1y1 with lines t "debug2"     lw 1 lc rgb "green", \
      file using 1:(6.5*(21.0-$11)) axes x1y1 with lines t "P-Anteil"    lw 1 lc rgb "blue", \
      file using 1:(6.5*(21.0-$11) + 0.01*$15 + 1.5*$16) axes x1y1 with lines t "raumaufsch"    lw 2 lc rgb "blue", \
      file using 1:($7-$6) axes x1y1 with lines t "Spreizung"     lw 1 lc rgb "red"
#    file using 1:15 axes x1y1 with lines t "ZT"