Opt("WinWaitDelay",100)
Opt("WinTitleMatchMode",4)
Opt("WinDetectHiddenText",1)
Opt("MouseCoordMode",0)
#include <IE.au3>
#include <Array.au3>

const $source_path="D:\test\Projekt_HVis\"
const $source_file="testdat.dat"
const $html_template_fname="D:\test\Projekt_HVis\HV1.html"
const $html_output_fname="D:\test\Projekt_HVis\index.html"

global $zeilenteile
dim $texte[20]
Dim $color[20]

$texte[0]="0"
$texte[1]="an" ; Schaltuhr Hzg
$texte[2]="aus" ; Schaltuhr WW
$texte[3]="t03" ; status Hzg
$texte[4]="t04" ; status BWWW
$texte[5]="t05" ; status WW
$texte[6]="t06" ; Kamin
$texte[7]="t07"; frost/übergang
$texte[8]="t08" ; BR
$texte[9]="t09" ; BWWP
$texte[10]="t10" ; WW-LP
$texte[11]="t11" ; UWP
$texte[12]="t12" ; nb
$texte[13]="t13"; nb
$texte[14]="t14"; nb
$texte[15]="t15"; nb
$texte[16]="t16"; nb
$texte[17]="t17"; nb
$texte[18]="t18"; nb
$texte[19]="t19"; nb

$color[0]="col00"
$color[1]="col01"
$color[2]="col02"
$color[3]="col03"
$color[4]="col04"
$color[5]="col05"
$color[6]="col06"
$color[7]="col07"
$color[8]="silver"
$color[9]="silver"
$color[10]="yellow"
$color[11]="yellow"
$color[12]="col12"
$color[13]="col13"
$color[14]="col14"
$color[15]="col15"
$color[16]="col16"
$color[17]="col17"
$color[18]="col18"
$color[19]="col19"
#cs
	debug1= Raumregler.PID_esum;
	debug2= Raumregler.PID_d;
	debug3= UWP_soll;
	debug4= timer_UWP_Pause.ist;
	debug5= timer_UWP_Mindestlauf.ist;
	//if (state.heiz ==aktiv_an) {sollwert=Hzg.soll; } else {sollwert=Hzg.soll-Hzg.hyst; }		// Sollwert fÃ¼r die Datenausgabe umrechnen
	sollwert=  (state.heiz ==aktiv_an) ? (Hzg.soll) :  (Hzg.soll-Hzg.hyst); // Sollwert fÃ¼r die Datenausgabe umrechnen
	if (day==0)  printf("#");	// solange DCF nicht synchr.
	printf("%02d.%02d.%02d %02d:%02d:%02d" " %d%d%d%d%d"
			" %3d %4.1f %4.1f %4.1f %3.1f %3.1f %3.1f %3.1f %3.1f %3.1f %5.2f"
			"    %5.2f  %5.2f  %3d  %3d   %3d   %3.1f   %3.1f   %d  %3.2f  %d  %d\n",
			day, mon, year, hh, mm, ss,
			((flag.uebergang||flag.frostschutz)*4+(Schaltuhr.WW*2)+Schaltuhr.Hzg) ,state.heiz,state.ww,state.bwwp, (Brennerstatus()+2*BWWPstatus()),
	        T_Abgas(), sollwert, T_RL.a, T_VL.a, T_Kessel.a, T_WW.k,
			T_WoZi.k, T_WoZi.l, T_Raum2.a, T_Aussen.l, T_AussenTag.l,
			debug1,debug2,debug3,debug4,debug5, T_R1.a, T_R2.a, flag.kamin, Raumregler.Tsoll, timer_Heizstart.ist, timer_Schnellaufheizen.ist);
#ce

Func fill_textarray()
 $status=$zeilenteile[3] ; Status auswerten
 $s=Stringmid($status,1,1); Status erste Stelle
 $texte[7]="0"
 $texte[1]="aus"
 $texte[2]="aus"
 if $s>4 Then ; Übergang ode frostschutz gesetzt
	 $texte[7]="1"
	 $s=$s-4;
 EndIf
 Switch $s
	Case 1
		$texte[1]="an"
	Case 2
		$texte[2]="an"
	Case 3
		$texte[1]="an"
		$texte[2]="an"
 EndSwitch
 $s=Stringmid($status,2,1); Status zweite Stelle	Hzg-Status
 $texte[3]="-"
 Switch $s
	Case 0
		$texte[3]="standby"
	Case 1
		$texte[3]="aktiv_aus"
	Case 2
		$texte[3]="aktiv_an"
	Case 3
		$texte[3]="nachlauf"
 EndSwitch

 $s=Stringmid($status,3,1); Status  dritte Stelle	WW-Status
 $texte[5]="-" ; status
 $color[5]="silver"
 $texte[10]="aus" ; WW-Ladepumpe
 $color[10]="silver"
 Switch $s
	Case 0
		$texte[5]="standby"
	Case 1
		$texte[5]="aktiv_aus"
	Case 2
		$texte[5]="aktiv_an"
		$texte[10]="an"
		$color[10]="yellow"
	Case 3
		$texte[5]="nachlauf"
		$texte[10]="an"
		$color[10]="yellow"
 EndSwitch
$s=Stringmid($status,4,1); Status vierte Stelle	BWWP-Status
$texte[4]="-"
 Switch $s
	Case 0
		$texte[4]="standby"
	Case 1
		$texte[4]="aktiv_aus"
	Case 2
		$texte[4]="aktiv_an"
	Case 3
		$texte[4]="nachlauf"
 EndSwitch

$s=Stringmid($status,5,1); Status  fünfte Stelle	Aktor-Status
$texte[8]="aus" ; BR
$color[8]="silver"
$texte[9]="aus" ; WP
$color[9]="silver"
 Switch $s
	Case 1
		$texte[8]="an"
		$color[8]="red"
	Case 2
		$texte[9]="an"
		$color[9]="red"
	Case 3
		$texte[8]="an"
		$color[8]="red"
		$texte[9]="an"
		$color[9]="red"
 EndSwitch
; UWP auswerten --------
$uwp=$zeilenteile[17]
$texte[11]=$uwp
$color[11]="yellow"
If $uwp=0 Then
	$texte[11]="aus"
	$color[11]="silver"
EndIf

; Kamin auswerten --------
$uwp=$zeilenteile[17]
$texte[6]="aus"
If $zeilenteile[22]=1 Then
	$texte[6]="an"
EndIf

EndFunc

;----------------------------------------
; sucht im template Merker für Variablen
;%nn: direkte Ersetzung, d.h. aus gelesener Zeile 1:1 mit Index nn
;#zz: berechneter Wert aus zweitem Array
Func scan_html()
	const $varmarker="~"; Marker für Variablen im template
	const $varlaenge=2 ; Länge der Zahl nach dem Marker
	const $textmarker="§"; Merker für Texte im template
	const $textlaenge=2 ; Länge der Zahl nach dem Marker
	const $colormarker="§c"; Merker für Texte im template
	const $colorlaenge=2 ; Länge der Zahl nach dem Marker
	local $file, $line, $lc, $index
	$html_out=FileOpen($html_output_fname,2); immer überschreiben
	$file = FileOpen($html_template_fname,0); lesen
	If $file = -1 Then
		MsgBox(0, "Fehler", "Die Datei konnte nicht geöffnet werden.")
		Exit
	Else
		$lc=0
		While 1
			$line = FileReadLine($file) ; template zeilenweise einlesen
			If @error = -1 Then ExitLoop
			$lc=$lc+1
			;nach Variablen scannen
			$treffer = StringRegExp($line, "("& $varmarker & "\d{"&$varlaenge&"})+", 3); findet alle Übereinstimmungen und speichert sie in array
			if @Error =0 Then ; Ausdruck gefunden
				;_ArrayDisplay($treffer); debug
				For $i=0 to UBound($treffer)-1 ; für alle Ausdrücke in einer Zeile
					$index= StringMid($treffer[$i],Stringlen($varmarker)+1,$varlaenge); Zahl separieren
					$line=StringReplace($line,$varmarker&$index, $zeilenteile[$index]) ; Marker durch Wert ersetzen
					;ConsoleWrite('@@ Debug(' & @ScriptLineNumber & ') : $line = '&$lc & $line & '      Index: '&$index & '   ' & $zeilenteile[$index] & @crlf) ;### Debug Console
				Next
			EndIf
			;nach Texten scannen
			$treffer = StringRegExp($line, "("& $textmarker & "\d{"&$textlaenge&"})+", 3); findet alle Übereinstimmungen und speichert sie in array
			if @Error =0 Then ; Ausdruck gefunden
				;_ArrayDisplay($treffer); debug
				For $i=0 to UBound($treffer)-1 ; für alle Ausdrücke in einer Zeile
					$index= StringMid($treffer[$i],StringLen($textlaenge)+1, $textlaenge); Zahl separieren
					$line=StringReplace($line,$textmarker&$index, $texte[$index]) ; Marker durch Wert ersetzen
					;ConsoleWrite('@@ Debug(' & @ScriptLineNumber & ') : $line = '&$lc & $line & '      Index: '&$index & '   ' & $zeilenteile[$index] & @crlf) ;### Debug Console
				Next
				;ConsoleWrite('@@ Debug(' & @ScriptLineNumber & ') : $line = '& $line & @crlf) ;### Debug Console
			endif
			;nach Farben scannen
			$treffer = StringRegExp($line, "("& $colormarker & "\d{"&$colorlaenge&"})+", 3); findet alle Übereinstimmungen und speichert sie in array
			if @Error =0 Then ; Ausdruck gefunden
				;_ArrayDisplay($treffer); debug
				For $i=0 to UBound($treffer)-1 ; für alle Ausdrücke in einer Zeile
					$index= StringMid($treffer[$i],StringLen($colormarker)+1,$colorlaenge); Zahl separieren
					$line=StringReplace($line,$colormarker&$index, $color[$index]) ; Marker durch Wert ersetzen
					;ConsoleWrite('@@ Debug(' & @ScriptLineNumber & ') : $line = '&$lc & $line & '      Index: '&$index & '   ' & $color[$index] & @crlf) ;### Debug Console
				Next
				;ConsoleWrite('@@ Debug(' & @ScriptLineNumber & ') : $line = '& $line & @crlf) ;### Debug Console
			endif



			FileWriteLine($html_out,$line)
		WEnd
	EndIf
	FileClose($file);
	FileClose($html_out);
EndFunc
;----------------------------------
Func read_last_line($source)
	local $file, $line, $lastline, $line_length
	$file = FileOpen($source,0)
	If $file = -1 Then
		MsgBox(0, "Fehler", "Die Datei konnte nicht geöffnet werden.")
		Exit
	Else
		While 1
			$line = FileReadLine($file) ; zeilenweise einlesen
			If @error = -1 Then ExitLoop
				$line_length=StringLen($line)
				if ( (StringLeft($line,1) <> "#") And ($line_length >90) ) Then ; Zeileninhalt i.O.
				$lastline=$line
			Else
				;MsgBox(0, "Defekte Zeile gelesen:", $line & " >>" & $line_length)
			EndIf
		WEnd
	EndIf
	FileClose($file); Handle  schliessen
	return($lastline)
EndFunc
;----------------------------------
; main
;----------------------------------
#cs
Enum $z_Datum=1, $z_Zeit, $z_Status, $z_T_Abgas, $z_T_RL_Soll, $z_T_RL_a, $z_T_VL_k, $z_T_KT_k, $z_T_WW_k,
	$z_WoZi_k, $z_Wozi_l, $z_T_oben_a, $z_T_Aussen_k, $z_T_Aussen_l, $z_T_AussenTag_l,
	$z_debug1 ,$z_debug2 ,$z_UWP_Soll, $z_debug3 ,$z_debug4 , $z_T_Keller1, $z_T_Keller2,
	$z_debug4, $z_T_Raum_Soll, $z_debug5, $z_debug6, $z_ENDE
#ce
dim $varname[30] ; enthält Variablennamen aus html-Vorlage
dim $value[30]; enthält aufbereitete Werte aus gelesener Zeile

$lastline= read_last_line($source_path & $source_file)
;MsgBox(0, "Letzte Zeile:", $lastline)
$lastline= StringStripWS ( $lastline,4 )
$zeilenteile=StringSplit ($lastline," ",1); Zeile aufteilen in array von Teilstrings;
if UBound($zeilenteile)<25 Then
	exit;
EndIf
fill_textarray()
scan_html() ; html->Template durchsuchen und ersetzen

#cs
Aufbau Zeilenarray
[0]|25 			Anzahl
[1]|06.01.11	Datum
[2]|17:32:35	Zeit
[3]|32123		Statuswort
[4]|123			Abgas
[5]|43.5		RL_soll
[6]|34.6		T_RL
[7]|40.0		T_VL
[8]|42.7		T_Kessel
[9]|45.6		T_WW
[10]|20.5		T_R_Wozi.k
[11]|20.7		T_R_Wozi.l
[12]|20.5		T_R_oben
[13]|1.1		T_Aussen.l
[14]|-1.23		T_Aussentag.l
[15]|323.44		PID.P
[16]|0.45		PID.D
[17]|80			UWP
[18]|0
[19]|0
[20]|12.6		T_Keller1
[21]|10.4		T_Keller2
[22]|0
[23]|20.94		T_R_Soll
[24]|0			debug
[25]|0			debug
#ce

Exit;