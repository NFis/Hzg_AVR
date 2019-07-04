#include "clock.h"
// Die globale Uhrzeit seht in folgenden Variablen:
volatile uint8_t 		ss		= 0;	///<Globale Variable für die Sekunden
volatile uint8_t 		mm	= 0;	///<Globale Variable für die Minuten
volatile uint8_t 		hh		= 0;	///<Globale Variable für die Stunden
volatile uint8_t 		day	= 0;	///<Globale Variable für den Tag
volatile uint8_t		wday	= 0;	///<Globale Variable für den Wochentag 1=Mo...7=So
volatile uint8_t 		mon	= 0;	///<Globale Variable für den Monat
volatile uint8_t 		year	= 0;	///<Globale Variable für das Jahr; nur zweistellig!!!
volatile uint8_t 		rx_bit_counter = 0; 	///< Bitzähler für RX Bit
volatile uint64_t 	dcf_rx_buffer 	= 0; 	///< 64 Bit für DCF77; benötigt werden 59 Bits unsigned long long 
//volatile struct DCF77_Bits dcf_rx_buffer;
volatile uint16_t	h_ss = 0; 	///< Hilfs Sekunden Counter
// Deklarationen
void Add_one_Second(void); 
#define TCNT1_init	(UINT16_MAX-(F_CPU/1024U)) // = 51135 / 0xC7BF
///############################################################################
/// Interrupt-Routine, Overflow Timer1
/**Overflow Interrupt wird ausgelöst in der 59. Sekunde oder bei fehlendem DCF77 Signal */
ISR(TIMER1_OVF_vect) {
	struct  DCF77_Bits *rx_buffer;
	rx_buffer = (struct DCF77_Bits*)(char*)&dcf_rx_buffer;
	//rx_buffer = (struct DCF77_Bits*)&dcf_rx_buffer;
	
	TCNT1 = TCNT1_init; ///Zurücksetzen des Timers
	/// wurden alle 59 bit empfangen und sind die Paritys richtig?
	if (rx_bit_counter == 59 && 
		flags.parity_P1 == rx_buffer->P1 && 
		flags.parity_P2 == rx_buffer->P2 &&
		flags.parity_P3 == rx_buffer->P3) { /// alle 59 bit empfangen: stellen der Uhr nach DCF77 Buffer
			mm = rx_buffer->Min-((rx_buffer->Min/16)*6); 	//Berechnung der Minuten BCD to HEX
			if (mm > 0) {
				mm--;
			}
			else mm = 59;
			if (mm != 59) {
				hh = rx_buffer->Hour-((rx_buffer->Hour/16)*6);	 		//Berechnung der Stunden BCD to HEX
				day= rx_buffer->Day-((rx_buffer->Day/16)*6);  		//Berechnung des Tages BCD to HEX
				mon= rx_buffer->Month-((rx_buffer->Month/16)*6); 		//Berechnung des Monats BCD to HEX
				year= /*2000 +*/ rx_buffer->Year-((rx_buffer->Year/16)*6); 		//Berechnung des Jahres BCD to HEX
			}	
			ss = 59; 	///Sekunden werden auf 0 zurückgesetzt
			flags.dcf_sync = 1;
	}
	else {  /// nicht alle 59 bit empfangen bzw. DCF77 Signal fehlt: Uhr läuft timergesteuert weiter
		Add_one_Second();
		flags.dcf_sync = 0;
	}
	rx_bit_counter = 0; /// Zurücksetzen des RX Bit Counters
	dcf_rx_buffer = 0; /// Löschen des Rx Buffers
}
//############################################################################
//DCF77 Modul empfängt Träger 
/// Interrupt-Routine für DCF-Empfang
/** DCF77 Modul löst am INT-Pin Interrupts aus; Umschaltung steigende/fallende Flanke*/
ISR (DCF77_INT){
	uint16_t pulse_wide;
	if (INT0_CONTROL == INT0_RISING_EDGE) 	{ ///Auswertung der Pulseweite 
		flags.dcf_rx ^= 1;
		h_ss = h_ss + TCNT1 - TCNT1_init; /// Secunden Hilfs Counter berechnen, F_CPU defined in main.h		
		TCNT1 = TCNT1_init; /// Zurücksetzen des Timers1
		//ist eine Secunde verstrichen // F_CPU defined in USART.H
		if ((h_ss > F_CPU / 1024 / 100 * 90)) { //90% von 1 Sekunde (14400) = 12960 =0x32A0
			Add_one_Second(); /// Addiere +1 zu Sekunden
			h_ss = 0; //Zurücksetzen des Hilfs Counters
		};
		INT0_CONTROL = INT0_FALLING_EDGE; /// nächster Interrupt soll bei abfallender Flanke ausgelöst werden
	}
	else { /// Auslesen der Pulsweite von ansteigender Flanke zu abfallender Flanke
		pulse_wide = TCNT1;
		TCNT1 = TCNT1_init; /// Zurücksetzen des Timers1
		h_ss = h_ss + pulse_wide - TCNT1_init; // Secunden Hilfs Counter berechnen
		///Parity speichern
		if (rx_bit_counter ==  21 || rx_bit_counter ==  29 || rx_bit_counter ==  36)  { //beginn von Bereich P1/P2/P3
			flags.parity_err = 0;
		};
		if (rx_bit_counter ==  28) {flags.parity_P1 = flags.parity_err;}; //Speichern von P1
		if (rx_bit_counter ==  35) {flags.parity_P2 = flags.parity_err;}; //Speichern von P2
		if (rx_bit_counter ==  58) {flags.parity_P3 = flags.parity_err;}; //Speichern von P3
		///Überprüfen, ob eine 0 (100ms) oder eine 1 (200ms) empfangen wurde
		if (pulse_wide > (UINT16_MAX - (F_CPU / 1024)/100*85)) {//Abfrage größer als 150ms (15% von 1Sekunde also 150ms)	
			((uint8_t *)&dcf_rx_buffer)[rx_bit_counter >> 3]  |= (uint8_t)(1 <<(rx_bit_counter % 8)); //andere version
			//dcf_rx_buffer = dcf_rx_buffer | ((unsigned long long) 1 << rx_bit_counter); //Schreiben einer 1 im dcf_rx_buffer an der Bitstelle rx_bit_counter
			flags.parity_err = flags.parity_err ^ 1; //Toggel Hilfs Parity
		}
		INT0_CONTROL = INT0_RISING_EDGE; /// nächster Interrupt soll bei ansteigender Flanke ausgelöst werden
		rx_bit_counter++; /// RX Bit Counter wird um 1 erhöht
	}
}
//############################################################################
///Addiert 1 Sekunde
/**ss wird erhöht und bei Überlauf die anderen Variablen weitergezählt.
*/
void Add_one_Second (void) {
	uint8_t monatswechsel;
	ss++; // Globale Sekunde
	if (ss == 60) {
		ss = 0;
		mm++;
		//Min_Timer_Updates(); // alle Timer hochzählen
		if (mm == 60) {
			mm = 0;
			hh++;
			if (hh == 24) {
				hh = 0;
				wday++;
				if(wday > So) wday = Mo; 
				day++; // immer erst weiterzählen; ist aber ggf. zu weit und wird dann korrigiert!
				monatswechsel=0;
				if (day==32 && (mon==1||mon==3||mon==5||mon==7||mon==8||mon==10||mon==12) ) monatswechsel=1;
				if (mon == 2  ) { // Februar
						if (year%4 == 0)	{ // vereinfachte Schaltjahrerkennung; gespart:  && (year%100 !=0)) || (year%400 == 0) )
							if (day==30) monatswechsel=1;
						}	
						else { // kein Schaltjahr
							if (day==29) monatswechsel=1; 
						}	
				}
				if ( day==31 && (mon==4||mon==6||mon==9||mon==11)  ) monatswechsel=1;
				if (monatswechsel) {
					mon++; 
					day=1;
				}	
			}
		}	
	}
}
//############################################################################
/// Startet DCF77 und initialisiert den Timer
void Start_Clock (void) {
	DCF77_INT_ENABLE(); 	///Interrupt DCF77 einschalten
	INT0_CONTROL = INT0_RISING_EDGE; /// auf ansteigende Flanke einstellen
	TIMSK1 |= (1 << TOIE1); 	/// Interrupt Overflow enable
	TCCR1B |= (1<<CS10 | 0<<CS11 | 1<<CS12); 	///Setzen des Prescaler auf 1024 
	TCNT1 = TCNT1_init;    ///Timer befüllen; 
/*
	// OW- Timer0, 14400 counts/s
	TCCR0 	|= (0 << WGM00 | 0 << WGM01 | 1 << CS02 | 0<< CS01 | 1 << CS00); 	/// normal mode, Prescaler /1024
	TIMSK 	|= (1 << TOIE0); 	/// Interrupt Overflow enable
	TCNT0 =0; //init
*/
}

/// Interrupt-Routine, Overflow Timer0
/**Overflow Interrupt wird ausgelöst, wenn Timer0 überläuft, d.h. TCNT0 > 255 ist und wieder 0 wird.
14400 counts/s  / 255 = ISR wird alle 56ms aufgerufen
Timer-Wert steht in TCNT0
*/
/*
ISR(TIMER0_OVF_vect) {
  ow_Timer++; // jeweils alle ca. 56ms erhöhen.
}
*/
//---------------------------------------------------------------
/// Initialisierung eines Timers
/** Timer, der im Minutentakt läuft, initialisieren
@param str_timer* Pointer auf Timer
@param soll Dauer 
*/
void Timer_Minuten_init(struct str_timer* ptimer, uint16_t soll) {
 ptimer->ist= soll;
 ptimer->abgelaufen = 0;
}
//---------------------------------------------------------------
/// Update der Minutentimer
/** Timer herunterzählen und bei Null auf abgelaufen setzen
@param str_timer* Pointer auf Timer
@retval abgelaufen ist eins, wenn Timer abgelaufen ist  
*/
void Timer_Minuten_update(struct str_timer* ptimer) {
	if (ptimer->ist > 0) (ptimer->ist)--;
	if (ptimer->ist > 0) ptimer->abgelaufen = 0;
	else ptimer->abgelaufen = 1;
}
/// Update aller timer, die im Minutentakt laufen
void Timer_Minuten_update_all(void) {
	Timer_Minuten_update(&timer_EEPROM);
	Timer_Minuten_update(&timer_BR_Sperre);
	Timer_Minuten_update(&timer_Heizstart);
	Timer_Minuten_update(&timer_Schnellaufheizen);
	Timer_Minuten_update(&timer_UWP_Pause);
	Timer_Minuten_update(&timer_UWP_Mindestlauf);
	Timer_Minuten_update(&timer_WWstart);
}