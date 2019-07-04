/*#######################################################################################
AVR DCF77 Clock   Copyright (C) 2005 Ulrich Radig
#######################################################################################*/
#ifndef _CLOCK_H
 #define _CLOCK_H
#include <avr/interrupt.h>
#include "main.h"
// wday-Numerierung:
#define Mo	1	///< wday-Nr für Montag
#define Di	2	///< wday-Nr für Dienstag
#define Mi	3	///< wday-Nr für Mittwoch
#define Do	4	///< wday-Nr für Donnerstag
#define Fr	5	///< wday-Nr für Freitag
#define Sa	6	///< wday-Nr für Samstag
#define So	7	///< wday-Nr für Sonntag

volatile uint8_t		ss;		///<Globale Variable für Sekunden
volatile uint8_t		mm;		///<Globale Variable für Minuten
volatile uint8_t		hh;		///<Globale Variable für Stunden
volatile uint8_t		day;		///<Globale Variable für den Tag
volatile uint8_t		wday;	///<Globale Variable für den Wochentag
volatile uint8_t		mon;		///<Globale Variable für den Monat
volatile uint8_t		year;		///<Globale Variable für den Jahr; zweistellig!

extern void Start_Clock (void); 

struct str_timer {
 uint16_t ist; 				///< ist-Wert des Timers
 uint8_t abgelaufen; 	///< ist 1, wenn Timer abgelaufen ist,  d.h. den Wert 0 hat
};
void Timer_Minuten_init(struct str_timer* ptimer, uint16_t soll) ;
void Timer_Minuten_update(struct str_timer* ptimer);
void Timer_Minuten_update_all(void);

struct str_timer 	timer_EEPROM; ///< Abspeichern in EEPROM
struct str_timer	timer_BR_Sperre; ///< Brennersperrzeit
struct str_timer	timer_Heizstart; ///< Verzögerung (nach Schaltuhr-Freigabe) bis zum Heizstart
struct str_timer	timer_WWstart;  ///< Verzögerung (nach Schaltuhr-Freigabe) bis zum WW-Start
struct str_timer	timer_Schnellaufheizen; ///< Dauer des Schnellaufheizens nach Nachtabschaltung
struct str_timer	timer_UWP_Pause; ///< UWP Pause, spreizungsabhängig 
struct str_timer	timer_UWP_Mindestlauf; ///< UWP Mindestlaufzeit nach Pause

//volatile uint64_t dcf_rx_buffer; ///< 64 Bit für DCF77; benötigt werden 59 Bits
volatile extern uint8_t rx_bit_counter;	///< RX Pointer (Counter)
volatile uint16_t h_ss; ///< Hilfs Sekunden Counter

//Structur des dcf_rx_buffer
struct  DCF77_Bits {
	uint8_t M			:1	;
	uint8_t O1			:1	;
	uint8_t O2			:1	;
	uint8_t O3			:1	;
	uint8_t O4			:1	;
	uint8_t O5			:1	;
	uint8_t O6			:1	;
	uint8_t O7			:1	;
	uint8_t O8			:1	;
	uint8_t O9			:1	;
	uint8_t O10		:1	;
	uint8_t O11		:1	;
	uint8_t O12		:1	;
	uint8_t O13		:1	;
	uint8_t O14		:1	;
	uint8_t R			:1	;
	uint8_t A1			:1	;
	uint8_t Z1			:1	;
	uint8_t Z2			:1	;
	uint8_t A2			:1	;
	uint8_t S			:1	;
	uint8_t Min		:7	; ///< 7 Bits für die Minuten
	uint8_t P1			:1	; ///< Parity Minuten
	uint8_t Hour		:6	; ///< 6 Bits für die Stunden
	uint8_t P2			:1	; ///< Parity Stunden
	uint8_t Day		:6	; ///< 6 Bits für den Tag
	uint8_t Weekday	:3	; ///< 3 Bits für den Wochentag 
	uint8_t Month	:5	; ///< 3 Bits für den Monat
	uint8_t Year		:8	; ///< 8 Bits für das Jahr **eine 5 für das Jahr 2005**
	uint8_t P3			:1	; ///< Parity von P2
	};
	
//volatile struct DCF77_Bits dcf_rx_buffer; ///< 64 Bit für DCF77; benötigt werden 59 Bits	
struct {
	volatile int8_t 	parity_err	:1	; ///< Hilfs Parity
	volatile int8_t	parity_P1	:1	; ///< Berechnetes Parity P1
	volatile int8_t 	parity_P2	:1	; ///< Berechnetes Parity P2
	volatile int8_t 	parity_P3	:1	; ///< Berechnetes Parity P3
	volatile int8_t 	dcf_rx		:1	; ///< Es wurde ein Impuls empfangen
	volatile int8_t 	dcf_sync	:1	; ///< In der letzten Minute wurde die Uhr syncronisiert
} flags;
	


#if defined (__AVR_ATmega128__)
	//Interrupt an dem das DCF77 Modul hängt hier INT0
	#define DCF77_INT_ENABLE()	EIMSK |= (1<<INT0);
	#define DCF77_INT			INT0_vect
	#define INT0_CONTROL		EICRA
	#define INT0_FALLING_EDGE	0x02
	#define INT0_RISING_EDGE	0x03
	#define TIMSK1 				TIMSK
#endif

#if defined (__AVR_ATmega32__)
	//Interrupt an dem das DCF77 Modul hängt hier INT0
	#define DCF77_INT_ENABLE()	GICR |= (1<<INT0);
	#define DCF77_INT			INT0_vect
	#define INT0_CONTROL		MCUCR
	#define INT0_FALLING_EDGE	0x02
	#define INT0_RISING_EDGE	0x03
	#define TIMSK1 				TIMSK
#endif

#if defined (__AVR_ATmega16__)
	//Interrupt an dem das DCF77 Modul hängt hier INT0
	#define DCF77_INT_ENABLE()	GICR |= (1<<INT0);
	#define DCF77_INT			INT0_vect
	#define INT0_CONTROL		MCUCR
	#define INT0_FALLING_EDGE	0x02
	#define INT0_RISING_EDGE	0x03
	#define TIMSK1 				TIMSK
#endif

#if defined (__AVR_ATmega8__)
	//Interrupt an dem das DCF77 Modul hängt hier INT0
	#define DCF77_INT_ENABLE()	GICR |= (1<<INT0);
	#define DCF77_INT			INT0_vect
	#define INT0_CONTROL		MCUCR
	#define INT0_FALLING_EDGE	0x02
	#define INT0_RISING_EDGE	0x03
    #define TIMSK1              TIMSK
#endif

#if defined (__AVR_ATmega88__)
	//Interrupt an dem das DCF77 Modul hängt hier INT0
	#define DCF77_INT_ENABLE()	EIMSK |= (1<<INT0);
	#define DCF77_INT			INT0_vect
	#define INT0_CONTROL		EICRA
	#define INT0_FALLING_EDGE	0x02
	#define INT0_RISING_EDGE	0x03
#endif
 
#endif //_CLOCK_H
