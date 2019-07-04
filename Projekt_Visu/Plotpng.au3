; Gnuplot starten und aus dem heutigen .dat-File eine "plot.html" machen
const $inifile="Plotpng.ini"
$datfilepath =	iniread($inifile,"Daten","datfilepath","d:\\temp\\hzgdaten\\");doppelte Trenner!
$gnuplot =		iniread($inifile,"Gnuplot",	"Programm","d:\programme\gnuplot\binary\gnuplot.exe")
$gnuplot_plt =	iniread($inifile,"Gnuplot",	"Script","d:\\test\\hzg_png.plt") ; doppelte Trenner!
$gnuplot_png =	iniread($inifile,"Gnuplot",	"png","d:\\test\\plot.png"); doppelte Trenner!
$webfile =		iniread($inifile,"WWW",		"htmlfile","d:\test\plot.html") ; 
$webzurueck=	iniread($inifile,"WWW",		"zurueck","d:\test\index.html") ; für Rücksprungadresse aus Plotfenster

$datfile=$datfilepath & @YEAR & @MON & @MDAY  & ".dat" ; heute
;D:\programme\gnuplot\binary\gnuplot.exe -e "file='D:\\temp\\hzgdaten\\20110111.dat'" D:\\test\\hzg_png.plt
$command=$gnuplot& " -e "& """file='" & $datfile & "';outfile='" & $gnuplot_png & "'"" " & $gnuplot_plt  
ConsoleWrite('@@ Debug(' & @ScriptLineNumber & ') : $command = ' & $command & @crlf & '>Error code: ' & @error & @crlf) ;### Debug Console
RunWait(@ComSpec & " /c " & $command, "", @SW_HIDE) ; Gnuplot völlig unauffällig im Hintergrund ausführen; es wird die Datei $gnuplot.png erzeugt
; html erzeugen
$file=FileOpen($webfile,2) ; 2: Datei schreiben
FileWriteLine($file,"<!DOCTYPE html PUBLIC><head></head><body><p><a href=""" & $webzurueck & """" & ">zur&uuml;ck</a></p>")
FileWriteLine($file,"<img src=""" & $gnuplot_png & """ width=""1000"" height=""760"" style=""border: 0px;""/></body></html>")
FileClose($file)
exit;