//=== main.h ========
#ifndef _MAIN_H_
#define _MAIN_H_
#define F_CPU  14745600UL /// CPU-Takt 
#include <avr/io.h>
#include <stdio.h>
#include <stdlib.h>

struct str_raumregler {
	double Tsoll;  		///< Sollwert
	double PID_esum; 	///< I-Anteil
	double PID_e; 		///< p-Anteil, Fehler
	double PID_d; 		///< D-Anteil
};

struct str_hzg {
   double offs; ///< Verschiebung durch Raumeinfluss usw.
   double soll; ///< Sollwert 
   double hyst; ///< Hysterese
   double m1; ///< Merker für letztn Sollwert
   double m2; ///< Merker für vorletzen Sollwert
}; 


struct str_flag {
	uint8_t uebergang;	///<Übergangszeit erkannt
	uint8_t sommer;		///<Sommer eingeschaltet
	uint8_t frostschutz;	///<Frost erkannt
	uint8_t kamin;			///<Kamin an
	uint8_t tuerauf;		///<Terassentür auf
	uint8_t start;			///<erster Start
}; 

struct str_state {
 uint8_t ww; 		//!< Status für Warmwasser per Kessel
 uint8_t heiz;		//!< Status für Heizbetrieb
 uint8_t bwwp; 	//!<Warmwasser per BW-Wärmepumpe
};


#define standby 		0 ///< standby, warten auf Freigabe
#define aktiv_aus 	1 ///< Freigabe, aber ohne Energie  
#define aktiv_an 		2 ///< Freigabe, aber mit Energie
#define nachlauf 		3 ///< Nachlauf, z. B. Pumpen-Nachlauf bei WW

struct str_raumregler Raumregler; ///< Raumregler
struct str_hzg Hzg; ///< Heizungssollwerte
struct str_flag flag; ///< div. Merker-Flags
struct str_state state; ///< Statusmerker für Statemachine 

// Procedures
double limit(double d, double limitmin, double limitmax);
void Heizkennlinie (void) ;
void Statusmaschine (void);

#endif
