#Region ;**** Directives created by AutoIt3Wrapper_GUI ****
#AutoIt3Wrapper_outfile=Hzg_plot_remote.exe
#EndRegion ;**** Directives created by AutoIt3Wrapper_GUI ****
#include <GUIConstants.au3>
#include <IE.au3>
#include <ButtonConstants.au3>
Opt("WinWaitDelay",100)
Opt("WinTitleMatchMode",1)
Opt("WinDetectHiddenText",1)
Opt("MouseCoordMode",0)
Opt("SendKeyDelay",10)
Opt("GUIOnEventMode", 1)

;ini einlesen
const $inifile="Hzg_plot.ini"
$www_source_path=	Iniread($inifile,"daten",	"www_source_path",	"http://fnet.selfhost.me/Hzg_Daten/") ;
$local_source_path=	Iniread($inifile,"daten",	"local_source_path","C:\temp\hzgdaten\") ;
$dest_path=			Iniread($inifile,"daten", 	"dest_path",		"D:\temp\hzgdaten\")
$ext=				Iniread($inifile,"daten", 	"ext",				".dat")
$tempfile= 			Iniread($inifile,"daten", 	"tempfile", 		"temp") & $ext
$logfile=			Iniread($inifile,"daten", 	"logfile",	 		"log.txt")
$gnuplot=			Iniread($inifile,"Gnuplot",	"program",			"d:\programme\gnuplot\binary\gnuplot.exe")
;$gnuplot_script =  	IniRead($inifile,"Gnuplot",	"script",			"hzg_png.plt") ; doppelte Trenner!
$gnuplot_script =  	IniRead($inifile,"Gnuplot",	"script",			"hzg_png.plt") ; doppelte Trenner!
$local=1; "local";				IniRead($inifile,"config",	"local","0")
;----------
$Form = GUICreate("Datumwahl", 242, 357, 197, 120)
GUISetOnEvent($GUI_EVENT_CLOSE, "FormClose")
$Label1 = GUICtrlCreateLabel("Heizdatenplot", 40, 8, 165, 33)
	GUICtrlSetFont(-1, 18, 800, 0, "MS Sans Serif")
	GUICtrlSetColor(-1, 0xFF0000)
$Button_local = GUICtrlCreateButton($local, 16, 38, 60, 20)
;	GUICtrlSetOnEvent(-1, "Button_localClick")
$heute = @YEAR & "/" & @MON & "/" & @MDAY ;$heute=@YEAR & "/" & @MON & "/" & @MDAY
$MonthCal = GUICtrlCreateMonthCal($heute, 16, 96, 193, 177)
	GUICtrlSetOnEvent(-1, "Kalender_Click")

$Label_Ueberschrift = GUICtrlCreateLabel("Datum wählen:", 16, 64, 123, 24)
	GUICtrlSetFont(-1, 12, 800, 0, "MS Sans Serif")
$Button_ok = GUICtrlCreateButton("OK!", 16, 288, 89, 41, 0)
	GUICtrlSetOnEvent(-1, "Ok_Click")
;GUICtrlSetFont(-1, 14, 800, 0, "MS Sans Serif")
;$Button_Cancel = GUICtrlCreateButton("Cancel", 120, 288, 89, 41, 0)
GUISetState(@SW_SHOW)

;
if $local="1"Then ; Button setzen
	Set_local();
Else
	Set_remote();
EndIf

While 1
	sleep(500)
WEnd

Func Set_local()
	GUICtrlSetData($Button_local, "local")
	GUICtrlSetBkColor($Button_local,0x00ff00)
EndFunc

Func Set_remote()
	GUICtrlSetData($Button_local, "remote")
	GUICtrlSetBkColor($Button_local,0xff0000)
Endfunc

;--------------------------
Func Warte($a,$b)
	$x=WinWait($a,$b,5)
	 If $x<>0 Then; Fenster existiert
		If Not WinActive($a,$b) Then WinActivate($a,$b)
		WinWaitActive($a,$b)
	EndIf
 return($x)
EndFunc
;--------------------------
Func write_log($t)
 local $file=Fileopen($logfile,1) ; write/append
 FileWriteLine($file,@YEAR & "." & @MON &"."& @MDAY &"  "& @HOUR &":" & @MIN & ":" & @SEC &"  "& $t)
 FileClose($file)
EndFunc
;--------------------------
; kopiert Datei $src (lokal oder aus dem Internet) in lokale Datei $dst
func Download($src,$dst)
	if GUICtrlRead($Button_local)="remote" Or StringInStr($src,"http") Then ; aus Internet laden
		$oIE = _IECreate( $src,0,0,0)
		Send("{ENTER}"); Proxy bestätigen
		$x=Warte("Verbindung herstellen mit fnet.selfhost.me","")
		If $x<>0 THEN ; window exisitert: server login
			Send("eb0fihr{TAB}4224{TAB}{TAB}{ENTER}")
		EndIf
		;_IELoadWait($oIE)
		;sleep(2000)
		; Speichern unter
		Warte("Dateidownload","Möchten Sie diese Da")
		sleep(500)
		Send("{ALTDOWN}s{ALTUP}") ; speichern auswählen
		;Speicherort
		Warte("Speichern unter","Speichern in:")
		Send($dst)
		Send("{Altdown}s{Altup}") ; speichern
		;Frage nach Überschreiben
		Warte("Speichern unter", $dst)
		Send("{Altdown}j{Altup}")
		;fertig
		Warte("Download beendet","Download abgeschloss")
		Send("{ENTER}")
	Else ; lokal
		FileCopy($src,$dst,	9)
	EndIf
EndFunc

;$oIE = _IECreate($source_path)
;$oIE = _IECreate("google.de")
;Send("{ENTER}"); Proxy bestätigen
;$oIE = _IECreate($source_path)
;Warte("Verbindung herstellen mit fnet.selfhost.me","Der Server")
;Send("eb0fihr{TAB}4224{TAB}{TAB}{ENTER}")
;----------
; umkopieren und unvollständige Zeile am Schluss weglassen
Func Clear($src,$dst)
	local $file_in=FileOpen ( $src, 0 ) ; 0: read  2: write
	; Prüfen, ob Datei zum Lesen geöffnet wurde
	If $file_in = -1 Then
		MsgBox(0, "Fehler", "Die Datei " & $src & " konnte nicht geöffnet werden.")
		Exit
	EndIf

	$file_out= FileOpen ( $dst, 2 ) ; 0: read  2: rewrite
	If $file_out = -1 Then
		MsgBox(0, "Fehler", "Die Datei " &$dst & " konnte nicht geöffnet werden.")
		Exit
	EndIf
	While 1
		$line = FileReadLine($file_in)
		If @error = -1 Then ExitLoop
		$line_length=StringLen($line)
		if ( (StringLeft($line,1) = "#") Or ($line_length >40) ) Then ; Zeileninhalt i.O.
		 FileWriteLine($file_out, $line)
		Else
			;MsgBox(0, "Defekte Zeile gelesen:", $line & " >>" & $line_length)
		EndIf

		;if ($line_length < 95) Then MsgBox(0, "Zeile gelesen:", $line & " >>" & $line_length)
	Wend
	FileWriteLine($file_out, "#----Ende---")
	FileClose($file_out)
	FileClose($file_in)
EndFunc
;---------------
Func Do_it()
	$gew_Datum=GUICtrlRead ($MonthCal) ; yyyy/mm/dd
	$Datum=stringleft($gew_Datum,4) & StringMid($gew_Datum,6,2)& Stringmid($gew_Datum,9,2)
	$file= $Datum & $ext
	$datfile=$dest_path & $file;

	if GUICtrlRead($Button_local)="remote" Then
		Download($www_source_path & $file, $dest_path & $tempfile)
		Clear($dest_path & $tempfile, $datfile)
	Else
		Download($local_source_path & $file, $dest_path & $tempfile)
		Clear($dest_path & $tempfile, $datfile)
	EndIf

	$datfile=StringReplace($datfile,"\","\\");
	$gnuplot_script=StringReplace($gnuplot_script,"\","\\");
	;$command=" -e "& """file='" & $datfile & "';outfile='" & $gnuplot_png & "'"" " & $gnuplot_plt
	$command=" -persist -e "& """file='" & $datfile & "'"" " & $gnuplot_script
	;ConsoleWrite('@@ Debug(' & @ScriptLineNumber & ') : $command = ' & $command & @crlf & '>Error code: ' & @error & @crlf) ;### Debug Console
	;$returncode=RunWait($command, "", @SW_HIDE) ; Gnuplot völlig unauffällig im Hintergrund ausführen; es wird die Datei $gnuplot.png erzeugt
	Run($gnuplot&$command,"", @SW_MINIMIZE) ; Gnuplot ausführen
	Exit
endfunc

;------------
Func FormClose()
	Beenden()
EndFunc
;------------
Func Kalender_Click()
	Do_it();
	Beenden();
EndFunc
;------------
Func Ok_Click()
	Beenden()
EndFunc
;------------
Func Button_localClick()
	If GUICtrlRead($Button_local)="local" THEN  ; toggeln
		Set_remote();
		Iniwrite($inifile,"config","local","0")
	Else
		Set_local();
		Iniwrite($inifile,"config","local","1")
	EndIf
EndFunc
;------------
Func Beenden()
  exit;
EndFunc
;------------

