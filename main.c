/* Übersicht; Font=Verdana 10
state.bwwp für Brauchwasserwärmepumpe
UWP: Umwälzpumpe Heizkreis
WWP: Warmwasserladepumpe
BR: Brenner
ZUL: Zuluftventilator Heizraum
*/
#include "main.h"
#include "clock.h"
#include "tempmeas.h"
#include "i_o.h"
#include <util\delay.h>
#include <stdio.h>
#include <avr/eeprom.h>
// =========================================================================
/* Globale Variablen */
double ee_AT_mw_Tag_float EEMEM; //!< EEPROM-Variable für mittlere Tagestemperatur
uint8_t UWP_soll;  					//!< Sollwert für Umwälzpumpe
double tageszeit; 					//!< Dezimalisierte Uhrzeit hh+mm/100, d.h. 12.25 ist 12 Uhr 25 Min
static FILE mystdout = FDEV_SETUP_STREAM( uart_putchar, NULL, _FDEV_SETUP_WRITE );
// =========================================================================
//! Wert d nach oben/unten auf limitmin bzw. limitmax begrenzen
/**
*@param d      			Eingangswert
*@param limitmin      	untere Grenze
*@param limitmax       	obere Grenze
*@retval d 				Ergebnis
*/

double limit(double d, double limitmin, double limitmax) {
	if (d>limitmax)	d=limitmax;
	if (d<limitmin)		d=limitmin;
	return(d);
}	

/// Eine Zeile mit Messwerten ausgeben
void print_it(void){ 
	double sollwert;
	//int debug;
	double debug1, debug2;
	uint16_t debug3,debug4,debug5;
	debug1= Raumregler.PID_esum; 
	debug2= Raumregler.PID_d;
	debug3= UWP_soll;
	debug4= timer_UWP_Pause.ist;
	debug5= timer_UWP_Mindestlauf.ist;
	//if (state.heiz ==aktiv_an) {sollwert=Hzg.soll; } else {sollwert=Hzg.soll-Hzg.hyst; }		// Sollwert für die Datenausgabe umrechnen
	sollwert=  (state.heiz ==aktiv_an) ? (Hzg.soll) :  (Hzg.soll-Hzg.hyst); // Sollwert für die Datenausgabe umrechnen
	if (day==0)  printf("#");	// solange DCF nicht synchr. 
	printf("%02d.%02d.%02d %02d:%02d:%02d" " %d%d%d%d%d"
			" %3d %4.1f %4.1f %4.1f %3.1f %3.1f %3.1f %3.1f %3.1f %3.1f %5.2f"
			"    %5.2f  %5.2f  %3d  %3d   %3d   %3.1f   %3.1f   %d  %3.2f\n", 
			day, mon, year, hh, mm, ss,
			((flag.uebergang||flag.frostschutz)*4+(Schaltuhr.WW*2)+Schaltuhr.Hzg) ,state.heiz,state.ww,state.bwwp, (Brennerstatus()+2*BWWPstatus()),
	        T_Abgas(), sollwert, T_RL.a, T_VL.a, T_Kessel.a, T_WW.k, 
			T_WoZi.k, T_WoZi.l, T_Raum2.a, T_Aussen.l, T_AussenTag.l,
			debug1,debug2,debug3,debug4,debug5, T_R1.a, T_R2.a, flag.kamin, Raumregler.Tsoll);					
}
// =========================================================================
// Berechnung Warmwassersollwerte und Heizsollwerte
// In: 	Aussentemperatur, Raumtemperaturen
// Out: Hzg.soll, 	Hzg.soll_hyst, frostschutz_flag, 
// Parameter: hkp, Hzg.soll_absenk

#define def_T_Frostschutz 			-5.0  //!< unterhalb dieser Aussentemperatur in °C wird außerhalb der Heizzeit abgesenkt geheizt
#define def_T_Heizgrenze 				12.0	//!< oberhalb dieser Aussentemperatur in °C wird Heizung abgeschaltet
#define def_T_Raumsoll					 21.0 //!< Raumsolltemperatur in °C
#define def_T_Raumsoll_abgesenkt  18.0  //!< Raumsolltemperatur in °C bei Nachtabsenkung
#define def_dT_Hzg_soll_abgesenkt   10.0 //!< Absenkung der Heizkennlinie in K bei Nachtabsenkung*/
/// Sollwertberechnung Tsoll für Heizung
/** Berechnung des Sollwertes für Rücklauf, Ermittlung Übergangszeit, Frostschutz, Kamin
*@param Aussentemperatur
*@param Raumtemperaturen
*@param Poti
*@param Schalter für So/Wi
*@retval Hzg.soll
*@retval Hzg.soll_hyst
*@retval flag.frostschutz
*@retval flag.uebergang
*/
void Heizkennlinie(void){
	const double hkp[7][3] = { /// Array der Stützstellen für AT, Heizkennline und Hysterese
	{-15.0, 	44.5,	5.5 },  // 4.7
	{-10.0, 	43.3,	5.5 },  // 4.8
	{-5.0,	42.1,	4.9 }, // 4.9  läuft mit ca. 40%
	{+0.0,	40.5,	5.5 },
	{+5.0,	38.0,	5.9 },
	{+10.0,	35.2,	6.3},
	{+15.0,	31.0,	7.5}};  
	Hzg.soll =   hkp[0][1]; // Minimalwerte als Default; Maximalwerte ergeben sich explizit aus hkp[6][1]*/
	Hzg.hyst= 	hkp[0][2];
	for (uint8_t i=0; i<6; i++) { /// Interpolation Sollwert
		if ( T_Aussen.l >= hkp[i][0] && T_Aussen.l < hkp[i+1][0] ) { // AT liegt  im Intervall
			double temp1=(hkp[i+1][0]-hkp[i][0]); // (optimiert Codegröße)
			double temp2=(T_Aussen.l-hkp[i][0]);
			Hzg.soll=	hkp[i][1] + temp2 * (hkp[i+1][1] - hkp[i][1]) / temp1;
			Hzg.hyst=  hkp[i][2] + temp2 * (hkp[i+1][2] - hkp[i][2]) / temp1;
		}	
	}
	///Frostschutz, Raumaufschaltung + man. RT-Korrektur
	if (flag.frostschutz==aus ) { // aus; auf Frostschutz prüfen
		Raumregler.Tsoll= def_T_Raumsoll + Poti_dT_Raum_soll(); // default + Poti +/- 1.25V ergibt +/- 2.5 K Vertellbereich;
		if (T_AussenTag.l <= def_T_Frostschutz  && T_Aussen.l < def_T_Frostschutz
			&& Schaltuhr.Hzg==aus && state.ww == standby )  { //Frostschutz ggf. an
			flag.frostschutz = an;
			Timer_Minuten_init(&timer_Schnellaufheizen, 0); 
		}	
	}
	else { // Frostschutz ist an
		Raumregler.Tsoll = def_T_Raumsoll_abgesenkt+ Poti_dT_Raum_soll(); // abgesenkter Sollwert für Raumtemperatur	
		Hzg.soll  = Hzg.soll  - def_dT_Hzg_soll_abgesenkt;  // abgesenkter Sollwert für Heizkennlinie
		//Hzg.hyst = Hzg.hyst + 1.0;	//vergrößerte Hysterese
		if (T_Aussen.l > def_T_Frostschutz + 1.0)  { //Frostschutz ggf. wieder ausschalten 
			flag.frostschutz = aus;
		}
	}
		
	/// Raumeinfluss, PID-Regler
	Raumregler.PID_e = Raumregler.Tsoll - T_WoZi.l;  		// WoZi ist Referenz
	Raumregler.PID_esum = limit( (Raumregler.PID_esum + 0.5 * Raumregler.PID_e), -600.0, 500.0); // begrenzen, falls Strecke nicht reagiert oder aus ist, z. B. Nachtabsenkung, Übergangszeit
	Raumregler.PID_d =   Raumregler.Tsoll - T_WoZi.k; // unkonventioneller D-Anteil über Filterung
		
	/// Schnellaufheizung
	if ( timer_Heizstart.abgelaufen && !(timer_Schnellaufheizen.abgelaufen) && flag.frostschutz==0   ) {
		if (T_Aussen.l < 10.0) {
			Hzg.offs = 12.0 * Raumregler.PID_e + 1.4 * Raumregler.PID_d;
			Hzg.hyst = Hzg.hyst - 2.5;	
		}
		else {	// draussen ist es rel. warm...
		 Hzg.offs = 6.0 * Raumregler.PID_e + 1.3 * Raumregler.PID_d;
		}
		if ( T_WoZi.l < Raumregler.Tsoll ) {
			Raumregler.PID_esum=-500.0; // I-Anteil klemmen
		}
	}
	else { // normaler Regelbetrieb
		if ( flag.kamin == aus && Schaltuhr.Hzg==an) { // Kamin ist aus
			Hzg.offs = limit( (7.0 * Raumregler.PID_e + 0.005 * Raumregler.PID_esum + 0.8 * Raumregler.PID_d), -20.0 , 15.0) ;  // 12 Samples / min.   Ki = Kp / Tn 	
			//if (T_WoZi.l > Raumregler.Tsoll+0.8 && T_WoZi.k > Raumregler.Tsoll+1.1 ) flag.kamin=an; // ggf. ein
			if (T_WoZi.l > Raumregler.Tsoll+0.5 && T_WoZi.k > Raumregler.Tsoll+0.9 ) flag.kamin=an; // ggf. ein
		}
		if (flag.kamin== an) { // Kamin ist an
			//if (Hzg.m2 > 0) Hzg.offs = Hzg.m2; 	// Wert im Ringspeicher ist vorhanden
			//else Hzg.offs = -3.0; 					// leichte Absenkung; default 
			Hzg.offs = -3.0; 					// leichte Absenkung; default 
			Raumregler.PID_esum=0;
			if ( (T_WoZi.l < Raumregler.Tsoll+0.1) || Schaltuhr.Hzg== aus) flag.kamin= aus; // ggf. aus
			//Hzg.hyst = Hzg.hyst + 1.0;	
		}
	} 	
		
	/// Sollwertberechnung, i. a. für Rücklauftemperatur
	Hzg.soll = limit( (Hzg.soll + Hzg.offs), 20.0, 60.0)  ; //+- 2.5K  Potibereich +/-1.25)
	///Übergangszeit
	//Abschaltung in Übergangszeit:
	if  (   T_WoZi.l > Raumregler.Tsoll-0.1 
			&& 	( 		T_Aussen.l > def_T_Heizgrenze + 2.0  // Mitteltemperatur ausreichend
						||	T_AussenTag.l > def_T_Heizgrenze + 1.0
					)	
		)  {  
		flag.uebergang= an; // heizen sperren
		Hzg.soll = 15.0;	
		
	}

    // Übergangsbetrieb ggf.  ausschalten 
	if 	(  T_Aussen.l < def_T_Heizgrenze  // Heizgrenze
			|| (      	T_AussenTag.l < def_T_Heizgrenze - 0.6  
				) 
			|| (         	Schaltuhr.Hzg== an  
					&&   T_WoZi.l < Raumregler.Tsoll-0.3  // Raum kühl
					&&   (			tageszeit < 9.00 // morgens
								|| 	(mon== 9 && tageszeit > 19.30) // abends
								||  	(mon==10 && tageszeit > 19.00)
								||  	(mon== 4 && tageszeit > 19.50)
								||	 	(tageszeit >20.00)
								|| 	T_Aussen.l < T_AussenTag.l // Aussentemperaturabfall
						  )
				)
			|| ( 		T_WoZi.l < Raumregler.Tsoll-0.8 // kalt im Raum
			    )
		) {
		flag.uebergang= aus; 	// heizen freigeben
	}		
	///Schalter Sommer/Winterbetrieb
	if (Schalter_SoWi() == an) flag.uebergang=an; // übersteuert alles
    	
	/// Türauf-Erkennung im WoZi
	if ( flag.tuerauf ==aus) {// Tür zu
		if (T_WoZi.k < T_WoZi.a-1.5 && T_WoZi.k < 18.0 ) { 
			flag.tuerauf = an; // Tür auf
		}	
	}
	else {
		if (T_WoZi.l > 19.0 ) {
			flag.tuerauf = aus; 
			Raumregler.PID_esum = 0.0;
		}
	}
}
// =========================================================================
#define ww_eiskalt	4 ///< WW ist deutlich zu kalt
#define ww_zukalt	3 ///< WW ist zu kalt
#define ww_kalt		2 ///< WW ist unterhalb des Sollwerts
#define ww_mittel	1 ///< WW ist im Range
#define ww_ok	  	0 ///< WW ist ok
#define def_T_WW_Kessel_soll        			48.0	//!< max. WW-Temperatur bei Kessel
#define def_T_WW_Kessel_soll_abgesenkt 	46.5	//!< max. WW-Temperatur bei Kessel außerhalb der WW-Zeit
#define def_T_WW_Kessel_min        			46.0	//!< min. WW-Temperatur bei Kessel
/// Ermittlung WW-Bedarf bei Kesselbetrieb
/** Es werden die Grenzen für Wassertemperatur im Speicher abgefragt 
*@param T_WW.k						gefilterte Speichertemperatur
*@param Schaltuhr.WW				Schaltuhrzustand für WW
*@param _def_T_WW_Kessel_xx	Parameter
*@retval ww_xx							Bedarf
*/
int WW_Bedarf(void){ // 0: kein Bedarf    1: Bedarf  2: sofortiger Bedarf
	int e;
	e=ww_ok;													// 0 default: WWT_mw >= WWHzg.soll	
	if (T_WW.k < def_T_WW_Kessel_soll )  e = ww_mittel;  		// 1
	if (Schaltuhr.WW== an) { // WW tagsüber
		if (T_WW.k < def_T_WW_Kessel_min )  e = ww_kalt; 			// 2 Unterschreitung der Mindestemperatur: WW im Anschluss an Heizvorgang
		if (T_WW.k < def_T_WW_Kessel_min - 1.0 ) e=ww_zukalt; 	// 3 starke Unterschreitung der Mindestemperatur: WW sofort
		if (T_WW.k < def_T_WW_Kessel_min - 2.0)  e=ww_eiskalt; 	// 4 sehr starke Unterschreitung; ggf. BWWP unterstützen
	}
	else {
		if (T_WW.k < def_T_WW_Kessel_soll_abgesenkt )  e = ww_kalt; 
	}
	if (Schalter_WWBR() == aus) e=ww_ok;						// 0; Schalterabfrage: Betrieb mit Wärmepumpe
	return(e);	
}
// ========================================================================
#define def_T_WW_BWWP_soll        			48.0 	//!< max- WW-Temperatur bei BWWP
#define def_T_WW_BWWP_soll_abgesenkt 	46.5 	//!< max- WW-Temperatur bei BWWP außerhalb der WW-Zeit
#define def_T_WW_BWWP_min        			46.0 	//!< min. WW-Temperatur bei BWWP
/// Ermittlung WW-Bedarf bei Wärmepumpenbetrieb
/** Es werden die Grenzen für Wassertemperatur im Speicher abgefragt 
*@param T_WW.l						gefilterte Speichertemperatur
*@param Schaltuhr.WW				Schaltuhrzustand für WW
*@param _def_T_WW_BWWP_xx	Parameter
*@retval ww_xx							Bedarf
*/
int WW_BWWP_Bedarf(void){ // 0: kein Bedarf    1: im Soll 2: Bedarf
	int e;
	e=ww_ok;//	0;  // default: WW.Tk >= Sollwert
	if (Schaltuhr.WW==an) {
		if (T_WW.l < def_T_WW_BWWP_soll) e = ww_mittel; // 1;
	}		
	else  {
		if (T_WW.l < def_T_WW_BWWP_soll_abgesenkt) e =ww_mittel; // 1 abgesenkter Sollwert
	}		
	if (T_WW.l < def_T_WW_BWWP_min )  e = ww_kalt;	// 2; }  // Unterschreitung der Mindesttemperatur
return(e);	
}
// ========================================================================
#define heiz_ok	0		//!< kein Heizbedarf, RL oberhalb des oberen Wertes
#define heiz_mittel 2	//!< RL liegt zwischen unterem und oberen Wert
#define heiz_kalt 1		//!< RL liegt unterhalb des unteren Wertes
/// Ermittlung Heizbedarf
/** Heizbedarf ergibt sich aus Rücklauftemperatur RL.T und der Heizkennlinie HKL
*@param T_AussenTag.l				gefilterte Aussentemperatur
*@param _def_T_Heizgrenze	
*@retval heiz_xx							Bedarf
*/
int Heizbedarf(void){ 
	int e; 
	 e = heiz_mittel; //2 ; // default; zwischen unterer und oberer Grenze
	 if (T_AussenTag.l > def_T_Heizgrenze)  {
		if (T_RL.l < Hzg.soll-Hzg.hyst)  e = heiz_kalt;//1; // lange Filterung, untere Grenze unterschritten
	 }
	 else {  
		if (T_RL.k < Hzg.soll-Hzg.hyst)  e = heiz_kalt; // 1; // kurze Filterung, untere Grenze unterschritten
	 }
	// if (T_RL.a > Hzg.soll) e=heiz_ok;// 0; // obere Grenze überschritten; kein Heizbedarf
	 if (T_VL.a > Hzg.soll+6.0) e=heiz_ok;// 0; // Vorlauf entscheidet auf Abschaltung; obere Grenze überschritten; kein Heizbedarf
	 return (e);
}	

//=================================================
#define def_T_Kessel_min						30.0 	//!< Kesselschutz, minimale Temperatur in °C
#define def_T_Kessel_sockel 					38.0 	//!< Kesselschutz, Sockeltemperatur für UWP-Zuschaltung in °C
#define def_T_Kessel_sockel_Anhebung 	 3.0  	//!< Kesselschutz, angehobener Sockel in K
#define def_T_Kessel_kondens 				49.0 	//!< Kesselschutz, Kondensationstemperatur in °C
#define def_T_Kessel_WW_max				58.0 	//!< max. Kesseltemperatur bei Warmwasserberitung in °C
#define def_T_Kessel_max 					65.0 	//!< max. Kesseltemperatur in°C
#define def_Zeit_UWP_Pause					12 	//!< Pause für UWP bei geringer Spreizung in Minuten
#define def_Zeit_UWP_Mindestlauf 			5		//!< Mindestlaufzeit für UWP in Minuten
#define def_Zeit_BR_Sperre 					20		//!< Brennersperre in Minuten
#define def_Zeit_Schnellaufheizen			120	//!< Schnellheizzeit nach NAchtabschaltung in Minuten
#define def_dT_spreizung_min 				1.4	//!< minimale Spreizung in K; unterhalb wird UWP abgeschaltet
/// Statemachine
/** Statusvariablen
*@param state.heiz	für Heizung
*@param state.ww für Warmwasser per Kessel	
*@param state.bwwp für Warmwasser per Wärmepumpe
*/
void Statusmaschine (void){
	double dT_spreizung, dT_spreizung_soll;
	static double T_Kessel_sockel= def_T_Kessel_sockel; ; // Mindesttemperatur
	static int delay_min=0;
// state.heiz ---------
	switch (state.heiz) { /// state.heiz auswerten
		case standby: // keine Heizungsfreigabe
			flag.kamin =0;
			Hzg.m1=0;
			Hzg.m2=0; 
			// Frostschutzüberwachung
			if (flag.frostschutz==an) { // Frostschutz ausserhalb der Heizzeit oder in der Wartezeit 
			 	Timer_Minuten_init(&timer_Schnellaufheizen, 0); 
				Timer_Minuten_init(&timer_Heizstart, 180); //egal
				state.heiz= aktiv_aus; // <<<<<<<<<
				flag.start=an;
			}
			// warten auf Schaltuhr
			if ( flag.start == aus && Schaltuhr.Hzg==an ){ // Schaltuhr muss 3h früher als Nutzungsbeginn einschalten
				flag.start = an;
				// Verzögerung bis zum Einschalten in Minuten berechnen; Rest bis 3h /180min. heizt!
				delay_min= (int)(180.0- 2.5 * (20.0 - T_Aussen.l) - 8.0 * (Raumregler.Tsoll -T_WoZi.l)) ;  // Minuten
				if ( delay_min < 0) delay_min = 0;
				Timer_Minuten_init(&timer_Heizstart,(uint16_t)delay_min);
				delay_min=delay_min-30;
				if (delay_min < 0 ) delay_min = 0;
				Timer_Minuten_init(&timer_WWstart, (uint16_t)delay_min);
			}
			// Freigabe Heizung nach Ablauf Heizstart-delay
			if ( (flag.start ==an && timer_Heizstart.abgelaufen)  ) { 
				Timer_Minuten_init(&timer_Schnellaufheizen, def_Zeit_Schnellaufheizen);
				//sprintf(s, "# Startzeit-delay WW (min): %5d", delay_min);
				printf("# Startzeit-delay WW (min): %5d\n", delay_min);
				//uputsnl(s);
				state.heiz= aktiv_aus; // <<<<<<<<<
			}
			Timer_Minuten_init(&timer_UWP_Pause,1);
			Raumregler.PID_esum=0.0;
			T_Kessel_sockel =	def_T_Kessel_sockel + def_T_Kessel_sockel_Anhebung; //für ersten Anheizvorgang anheben 
			break;	//--------------------------------------------------------	
		case aktiv_aus:  // hzg // Heizungsfreigabe ohne Brennerlauf
			// wenn im Frostschutz die Schaltuhr angeht, 3h weiter abgesenkt heizen
			if (flag.start ==an && flag.frostschutz== an && Schaltuhr.Hzg == an ) { // Wiedereinschalten des Normalbetriebes nach
					Timer_Minuten_init(&timer_Heizstart, 180); // 3h einamlig setzen
					flag.start = aus;
			}	
			if (timer_Heizstart.abgelaufen) flag.frostschutz=aus; // Absenkung zurücknehmen; da Schaltuhr an ist, wird flag.frostschutz nicht wieder aktiviert
			// >>> exit nachlauf	
			if ( Schaltuhr.Hzg==aus && flag.frostschutz==aus &&state.ww==standby) { // Heizfreigabe aufgehoben; Kessel nachlauf/leerfahren
					state.heiz = nachlauf;	// <<<<<<<<<
			}
			// UWP-Steuerung
			dT_spreizung = (T_VL.k > T_RL.k) ?  (T_VL.k - T_RL.k) : 0.0; 
		    if (timer_BR_Sperre.abgelaufen) dT_spreizung_soll= def_dT_spreizung_min;
			else dT_spreizung_soll= 2.0 * def_dT_spreizung_min;
			UWP_soll = an;
			if ( dT_spreizung < dT_spreizung_soll && timer_Schnellaufheizen.abgelaufen && T_Aussen.l>0) { // Spreizung klein
				UWP_soll = aus;
				if (timer_UWP_Pause.abgelaufen) { 
					if (timer_UWP_Mindestlauf.abgelaufen) Timer_Minuten_init(&timer_UWP_Pause, def_Zeit_UWP_Pause); // Zyklus neu starten; Spreizung ist nicht gestiegen
					else UWP_soll = an; // UWP laufen lassen, weil Mindesttimer noch nicht abgelaufen ist
				}			
				else Timer_Minuten_init(&timer_UWP_Mindestlauf,def_Zeit_UWP_Mindestlauf); // Mindesttimer aktiviert halten, solange UWP_Timer noch nicht abgelaufen ist
			} 
			else { // Spreizung groß bzw. darf nicht
				Timer_Minuten_init(&timer_UWP_Pause, def_Zeit_UWP_Pause); // Timer aktiviert halten
			}	
			if (UWP_soll == an) { // nach AT differenzieren
				UWP_soll = 210;
				if (T_Aussen.l > 0.0 ) { UWP_soll=190; }
				if (T_Aussen.l > 5.0 ) { UWP_soll=170; }
				if (T_Aussen.l > 10.0) { UWP_soll=160;  }	
				if (flag.kamin==an ||flag.frostschutz==an) UWP_soll=190;
				if (T_Aussen.l < 0 && dT_spreizung< dT_spreizung_soll) {UWP_soll=80;}
			}
				// >>> exit aktiv_an
			if ( Heizbedarf() ==heiz_kalt  && timer_BR_Sperre.abgelaufen) {  // Brenner anschalten	
				UWP_soll=80;
				if (T_RL.k < 30.0 ) {
					T_Kessel_sockel = def_T_Kessel_sockel + def_T_Kessel_sockel_Anhebung;
					UWP_soll  = aus;
				}	
				if  (T_Kessel.a < T_Kessel_sockel ) UWP_soll = aus;
				if ( flag.kamin ==aus && timer_Schnellaufheizen.abgelaufen) {
					Hzg.m2= Hzg.m1; // Ringspeicher :-) für Raumaufschaltung im Kaminbetrieb
					Hzg.m1= Hzg.offs;
				}	
				state.heiz = aktiv_an;  // <<<<<
			}
			break;//--------------------------------------------------------
		case aktiv_an:  // hzg // Heizungsfreigabe mit Brennerlauf
            T_RL.l	= T_RL.a; // Filter zurücksetzen.....
            T_RL.k 	= T_RL.a;
            if ( (T_Kessel.a > T_Kessel_sockel)  ) {
			if (T_Kessel.a < def_T_Kessel_kondens-4.0) 	{ 
					UWP_soll = 80; 
				} 
				else { 
					UWP_soll = 220; 
				} 
			}	
			if (T_Kessel.a < def_T_Kessel_sockel - 5.0) UWP_soll=aus;
			// exit aktiv_aus
			//if ( Heizbedarf()==heiz_ok || (T_Kessel.a>50.0 &&  timer_Schnellaufheizen.abgelaufen )) {// Abschaltgrenze des Kessels erreicht
			if ( Heizbedarf()==heiz_ok) {// Abschaltgrenze des Kessels erreicht
				Timer_Minuten_init(&timer_BR_Sperre, def_Zeit_BR_Sperre);
				T_Kessel_sockel=	def_T_Kessel_sockel;  // nach 1. Anheizen zurücksetzen
				state.heiz= aktiv_aus;
				if ( state.ww== aktiv_aus && WW_Bedarf()  >=ww_kalt) state.ww=aktiv_an; // ggf.  WW im Anschluss         
            }
            break;  	//--------------------------------------------------------
		case nachlauf: // hzg // Kessel leerfahren
			flag.start = aus;
			delay_min=0;
			if ( T_Kessel.a < 33.0 || Schaltuhr.Hzg==an || flag.frostschutz==an) { // Kessel kalt, Uhr geht wieder an oder Frostschutz
			    UWP_soll = aus;
				state.heiz= standby; 
			}	
			else { // leerfahren
				UWP_soll =170;
			}	
			break;
	} // switch state.heiz
	
// state.ww ---------
	switch (state.ww) { /// state.ww auswerten
		case standby: // ww // keine WW-Freigabe durch Schaltuhr
	        LPWW(aus); 
			if ( Schaltuhr.WW==an && flag.start==an && timer_WWstart.abgelaufen ) { // WW-Freigabe durch Schaltuhr >> w1
				if (WW_Bedarf() >=ww_kalt ) { state.ww = aktiv_an; }// Zwangsstart
				else { state.ww = aktiv_aus; }
			}
			if ( flag.uebergang == aus && Schaltuhr.Hzg==an && BWWPstatus()==1 && WW_Bedarf() == ww_eiskalt ) { // Unterstützung BWWP	
			 state.ww=aktiv_an;
			}
			break;
		case aktiv_aus: // WW-Freigabe, WW warm genug
            if ( WW_Bedarf() >=ww_zukalt ) { // Ermittlung, ob WW gemacht werden muss.
				state.ww= aktiv_an;
			}
			if  (Schaltuhr.WW== aus) { // aus
				state.ww= standby;
			}
			break;
			
		case aktiv_an: // ww // WW-Aufheizbedarf
			if ( T_Kessel.a > T_Kessel_sockel +def_T_Kessel_sockel_Anhebung ) {  // WWP an
				LPWW(an);
			}	
			else {
			    if ( T_Kessel.a <  T_Kessel_sockel ){
	     			LPWW(aus);
			    }
			}
            if ( ( T_Kessel.a > def_T_Kessel_WW_max) || ( T_Kessel.a > def_T_Kessel_kondens &&  WW_Bedarf() < ww_kalt )  )   { // Ende Kessel-Aufheizvorgang >> w3	
					LPWW(an);  
					//BR_Sperrzeit_Timer=0;
				    state.ww = nachlauf; 
			}
			else { // Abschaltgrenze erreicht 
			 if (WW_Bedarf() == ww_ok) {
			   LPWW(aus);
			   state.ww = aktiv_aus;
			 }
			}
			break;
			
		case nachlauf: // ww // WW-Nachheizphase
			if ( T_Kessel.a - T_WW.k < ( (flag.uebergang)?(2.0):(4.0)) ) { 
				LPWW(aus);
				state.ww= aktiv_aus;
			}
			else {
			  LPWW(an);
			}
			break;
	} // switch state.ww	
	
// state.bwwp ---------
	switch (state.bwwp) { /// state.bwwp auswerten
		case standby: // ww // keine WW-Freigabe durch Schaltuhr
	        BWWP(aus); //
			if ( Schaltuhr.WW==an || WW_BWWP_Bedarf()>=ww_kalt ) state.bwwp = aktiv_an;  // WW-Freigabe durch Schaltuhr oder Bedarf
			break;
			
		case aktiv_aus: // BWWP-Freigabe, WW warm genug
		    BWWP(aus);
            if ( WW_BWWP_Bedarf() >= ww_kalt ) state.bwwp= aktiv_an;  
			if  ( Schaltuhr.WW==aus && WW_BWWP_Bedarf() <= ww_kalt ) state.bwwp= standby; // Schaltuhr aus
			break;
			
		case aktiv_an: // BWWP läuft
			BWWP(an);
			if (WW_BWWP_Bedarf() == ww_ok )  state.bwwp = aktiv_aus; // Ende Aufheizvorgang
			break;
	} // switch state.bwwp	
//-------	
	///Umwälzpumpensteuerung
    if ( 		state.ww 	== aktiv_an 
			|| (state.ww	== nachlauf  && Schalter_WWBR()==an)
			|| flag.uebergang== an
			|| T_Kessel.a < def_T_Kessel_min)  { 
		UWP_soll= aus; 
	}
	  // bei Tür auf
	if ((flag.tuerauf== an) && state.heiz==aktiv_aus && T_Kessel.a<55.0) {
		UWP_soll= aus;	
	}	
	UWP(UWP_soll);  
//-------		
	/// Brennersteuerung
	if ( state.heiz== aktiv_an  ||  state.ww== aktiv_an) {
	  BR(an);
	}	
	else {
	  BR(aus);
	}
	
}	
// =========================================================================
/// Initialiserung
void maininit (void){
	uart_init(); /// RS232
	stdout = &mystdout;
	ioinit(); /// I/O
	Start_Clock(); ///DCF77 und Hw-Timer initialisieren
	/// Timer initialisieren
	Timer_Minuten_init(&timer_BR_Sperre, 2);
	Timer_Minuten_init(&timer_Schnellaufheizen, 2);
	Timer_Minuten_init(&timer_Heizstart, 2);
	Timer_Minuten_init(&timer_WWstart,2);
	Timer_Minuten_init(&timer_EEPROM,30);
	Timer_Minuten_init(&timer_UWP_Pause,0);
	Timer_Minuten_init(&timer_UWP_Mindestlauf,5);
	/// One Wire: erstes Mal Temperaturwerte einlesen; xyz.T werden durch read_meas befüllt
	ow_start_meas();
	_delay_ms(800); // Wandlung abwarten; max. 750ms
	ow_read_meas();  
	/// Filter mit Messwerten initialisieren
	Filter_init(&T_VL); // alle gefilterten Werte auf Messwert setzen
	Filter_init(&T_RL);
	Filter_init(&T_WoZi);
	Filter_init(&T_WW);
	Filter_init(&T_Aussen);
	Filter_init(&T_AussenTag);
	Filter_init(&T_Kessel);
	Filter_init(&T_Raum2);
	/// allg. flag
	flag.tuerauf=aus;
	flag.kamin=aus;
	flag.uebergang=aus;
	flag.frostschutz=aus;
	flag.start=aus;
	/// Raumregler
	Raumregler.PID_esum = 0.0;
	Raumregler.Tsoll = def_T_Raumsoll;
	/// Hzg
	Hzg.m1=0;
	Hzg.m2=0; 
	Hzg.offs=0;
	// Tagesmittelwert initialisieren; normalerweise aus EEPROM
	//Init bitte manuell...
	#if 0 // wird nur bei Bedarf compiliert...
		T_AussenTag.l= (float)11.67; // 19.04. 18:50   
		eeprom_write_block(&T_AussenTag.l, &ee_AT_mw_Tag_float, sizeof(ee_AT_mw_Tag_float)); // inital
	#endif
	/// Tagesmitteltemperatur aus EEPROM lesen
	eeprom_read_block(&T_AussenTag.l, &ee_AT_mw_Tag_float, sizeof(ee_AT_mw_Tag_float));  // Ü!
	UWP_soll=aus;
	/// Status Brenner und BWWP abfragen und ggf. weiterlaufen lassen
	if (BWWPstatus()==an) state.bwwp=aktiv_an; // BWWP läuft gerade
	else state.bwwp=aktiv_aus;
	if (Brennerstatus()==an) state.heiz = aktiv_an;  // Brenner läuft gerade
	else state.heiz=aktiv_aus;
	/// Versionsinfo ausgeben
	printf("#Norbert's Heizungssteuerung - Version: "__DATE__" / "__TIME__"\n"); //bei Neustart
	//uputsnl("#Norbert's Heizungssteuerung - Version: "__DATE__" / "__TIME__); //bei Neustart
//	ow_Timer=0;
	sei(); /// Interrupts freigeben
}
// =========================================================================
void Do_it(void) {
	static uint8_t oldsekunde=99;
	uint8_t sek, yes;
	/// One-Wire einlesen; genutzt wird TCNT1 von Timer1 von DCF
	yes= 0;
	sek=ss; //Kopie, damit der Rest hier konsistent abgearbeitet wird
	/// Protokollausgabe steuern
	if  (sek != oldsekunde) {
		Schaltuhr_einlesen(); 
		if (sek%5==3) { ///< in der 3. Sekunde im Fünfsekundenblock
			LED(an);
			ow_start_meas();  
			/// etwas Routine-Rechenzeit hierher legen
			
			LED(aus);
		}	
		if (sek%5==4) {///< in der 4. Sekunde im Fünfsekundenblock 
			LED(an);
			ow_read_meas();  
			LED(aus);
		}
		if (sek % 5==0)  { ///< in der 5. Sekunde im Fünfsekundenblock
			//Schaltuhr_einlesen(); 
			LED(an);
			/// filtern der Temperatursensoren
			Filterung_kurz(&T_VL);
			Filterung_kurz(&T_RL);
			Filterung_kurz(&T_WW);
			//Filterung_kurz(&T_Kessel); verzichtbar
			Filterung_kurz(&T_WoZi);
			Filterung_kurz(&T_Aussen);
			Filterung_kurz(&T_AussenTag);	
			Heizkennlinie(); /// Sollwerte ermitteln
			Statusmaschine(); /// abarbeiten der Statemachines
			if ( ( Brennerstatus() == an || timer_BR_Sperre.ist>15 || day==0) ) yes= 1; // alle 5 Sek., Brennerlauf, DCF noch nicht synchr.
			LED(aus);	
		}
		if (sek== 0) {// bei vollem Minutenanfang
			if ( day>0 ) {
				tageszeit=  hh+mm/100.0; // Uhrzeit berechnen
			} 
			else {
				tageszeit=0;  
			}
			/// Update der Minutentimer
			Timer_Minuten_update_all();
			/// filtern lang
			//Filterung_lang(&T_VL, zk_VL_l); 
			Filterung_lang(&T_RL, zk_RL_l); 
			Filterung_lang(&T_WW, zk_WW_l);
			Filterung_lang(&T_Aussen, zk_Aussen_l); 
			Filterung_lang(&T_WoZi, zk_WoZi_l); 
			Filterung_lang(&T_AussenTag, zk_AussenTag_l);
			/// EEPROM ggf. schreiben
			if (timer_EEPROM.abgelaufen)  { // alle 30 Minuten Wert Tagesmittelwert speichern
				 eeprom_write_block(&T_AussenTag.l, &ee_AT_mw_Tag_float, sizeof(ee_AT_mw_Tag_float));
				 Timer_Minuten_init(&timer_EEPROM,30);
			}
			yes=1; // Protokoll immer zum Minutenanfang
		}	
		if (sek==30 && ( Schaltuhr.Hzg== an || Schaltuhr.WW==an ) ) yes=1; // im normalen Betrieb Protokoll auch bei halben Minuten
		if (yes==1)  print_it(); ///< Daten ausgeben

	//ow_start_meas();  	
	}	
	oldsekunde=sek;
	//LED(aus);
}
// =========================================================================
/// Hauptprogramm
int main( void ) {
	maininit(); 				/// Initialisierung aufrufen
	while(1) {				/// endlose Hauptschleife
		Do_it();	
	} 
};	