#ifndef _TEMPMEAS_H
 #define _TEMPMEAS_H
 
struct str_temperatur {
	double a; 	///<aktueller Messwert
	double k; 	///<gefilterter Wert, kurz,  kleine Zeitkonstante
	double l; 	///<gefilterter Wert, lang,  große Zeitkonstante
};	
//void Filterung_kurz_alle();
//void Filterung_lang_alle();
struct str_temperatur T_VL; 				///< Vorlauf
struct str_temperatur T_RL; 				///< Rücklauf
struct str_temperatur T_Aussen; 		///< Aussen
struct str_temperatur T_AussenTag; 	///< Aussen für Tagesmittelwert
struct str_temperatur T_WW; 			///< Warmwaserspeicher
struct str_temperatur T_Kessel; 		///< Ölkessel
struct str_temperatur T_WoZi; 			///< Wohnzimmer
struct str_temperatur T_Raum2; 		///< großer Raum oben
struct str_temperatur T_R1; 				///< Reserve 1
struct str_temperatur T_R2; 				///< Reserve 2
//--- Filter -------------------------------------------
// Filterzeitkonstanten k: sekunden l: minuten
#define zk_xx_k				12.0		///< Filterzeitkonstante für alle Kurz-Filterungen (alle 5 sek.) ehemals 60 alle 1s
#define zk_VL_l				3.0		///< Filterzeitkonstante für Vorlauf (min.)
#define zk_RL_l				8.0		///< Filterzeitkonstante für Rücklauf (min.)
#define zk_Kessel_l			1.0	 	///< Filterzeitkonstante für Kessel (min.) 
#define zk_WoZi_l			20.0		///< Filterzeitkonstante für Wohnzimmer (min.)
#define zk_WW_l				20.0		///< Filterzeitkonstante für Brauchwsserspeicher (min.)
#define zk_Aussen_l			60.0		///< Filterzeitkonstante für Aussen (min.)
#define zk_AussenTag_l	1440.0	///< Filterzeitkonstante für Aussen-Tagesmittel (min.)
void Filter_init(struct str_temperatur* Temp);
void Filterung_kurz(struct str_temperatur* Temp);
void Filterung_lang(struct str_temperatur* Temp, double zk);

// One Wire Befehle
#define 	MATCH_ROM	0x55 		///< One Wire Befehl allg.
#define 	SKIP_ROM		0xCC 	///< One Wire Befehl allg.
#define 	CONVERT_T	0x44		///< One Wire Befehl, DS1820
#define 	READ				0xBE		///< One Wire Befehl, DS1820
// -- One Wire I/O-Pin---------------------
// siehe i_o.h
// -- One Wire Funktionen---------------------
uint8_t ow_start_meas( void ); 	/// Messung starten	
uint8_t ow_read_meas( void ); 	/// Messwerte lesen
#endif // _TEMPMEAS_H
