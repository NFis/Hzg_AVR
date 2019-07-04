// === i_o.h ===
// Atmel ATMEGA32 
/* Pin-Belegung:
(PA0) ADC0			: Abgas-NTC
(PA1) ADC1			: Poti HKL_offset	 
PA2 ADC2			: -
PA3 (ADC3) 		: Schalter WWsoll per Brenner
PA4 (ADC4)			: Schalter Sommer/Winter
PA5 (ADC5)			: BW-WP Status; an/aus
PA6 (ADC6)			: ext. Schaltuhr, Heizungsfreigabe
PA7 (ADC7)			: ext. Schaltuhr, Warmwasserfreigabe

PB0 (XCK/T0)		: LED out
PB1 (T1)			: One Wire DS18S20-Sensoren i/o
PB2 (INT2/AIN0)	: BR-Relais
PB3 (OC0/AIN1)	: UWP-Relais
PB4 (SS)			: WWP-Relais
PB5 MOSI			: ser. Prog ISP
PB6 MISO			: ser. Prog ISP
PB7 SCK				: ser. Prog ISP

PC0 (SCL)			: Zuluftrelais 
PC1 (SDA)			: Brennerzustand input; an/aus
(PC2) TCK			: JTAG
(PC3) TMS			: JTAG
(PC4) TDO			: JTAG
(PC5) TFI			: JTAG
PC6 TOSC1			: -
PC7 TOSC2			: -

(PD0) RXD			: UART
(PD1) TXD			: UART
(PD2) INT0			: DCF77 - in, Interrupt
PD3 INT1			: -
PD4 OC1B			: -	
PD5 OC1A			: -
PD6 ICP1			: -
(PD7) OC2			: UWP_v (Hw-PWM über Timer 2, Opto-out)
//--------------------------------
Timer:
Timer0 (8 bit)		: ---
Timer1 (16 bit)	: OVFL für DCF77
Timer2 (8 bit)		: PWM über Pin OC2
*/
#ifndef _I_O_H
 #define _I_O_H
// ==== PORT A ===========================================
// ----- PORT A  Analogeingänge
#define T_Abgas_pin 					PA0 // 
#define Poti_dT_Raum_pin 			PA1 // 
// ----- PORT A  Digitaleingänge
#define Umschalter_WWBR_input	PINA
#define Umschalter_WWBR_port	PORTA
#define Umschalter_WWBR_pin 	PA3 // input WW per Brenner_an_input

#define Umschalter_SoWi_input	PINA
#define Umschalter_SoWi_port	PORTA
#define Umschalter_SoWi_pin 	PA4 // input Sommer/Winter

#define Status_BWWP_input		PINA
#define Status_BWWP_port 		PORTA
#define Status_BWWP_pin 		PA5 // input BWWP-Status an/aus

#define Uhr_Hzg_input				PINA
#define Uhr_Hzg_port				PORTA
#define Uhr_Hzg_pin 				PA6 // input Uhrfreigabe Brenner

#define Uhr_WW_input				PINA
#define Uhr_WW_port 				PORTA
#define Uhr_WW_pin 				PA7 // input Uhrfreigabe WW
// === PORT B ===========================================
// ---- PORT B  Eingänge/Ausgänge
// PB1: One Wire i/o
// -- One Wire I/O-Pin---------------------
#define OW_PIN						PB1
#define OW_IN						PINB
#define OW_OUT						PORTB
#define OW_DDR						DDRB

// ---- PORT B  Digitalausgänge
// LED
#define LED_port 					PORTB 		 
#define LED_pin 						PB0 				

// Relais
#define BR_port 						PORTB 		
#define BR_pin  						PB2 	// Brenner schalten

#define UWP_port 					PORTB 		
#define UWP_pin 						PB3 // Umwälzpumpe

#define WWP_port 					PORTB 	
#define WWP_pin 					PB4 // Warmwasserladepumpe
// === PORT C ===========================================
// ---- PORT C  Digitalausgänge 
//#define ZUL_port 					PORTC
//#define ZUL_pin 					PC0 // Zuluftventilator

#define BWWP_port 					PORTC
#define BWWP_pin 					PC0 // BWWP - Relais

// ---- PORT C  Digitaleingänge 
#define Status_Brenner_port 		PORTC // Pullup
#define Status_Brenner_input 	PINC
#define Status_Brenner_pin 		PC1      // input Brennerlauf

// === PORT D ===========================================
// ---- PORT D Digitalausgänge
#define UWP_v_port 				PORTD 
#define UWP_v_pin  				PD7 //  output UWP Drehzahl PWM, Optokoppler

// Allgemein ===========================================
#define aus 							0 // für Pumpen, Brenner, LED
#define an								1 // für Pumpen, Brenner, LED
// UART ===========================================
#define	BAUD							19200 ///< Datenrate für ser. Schnittstelle 19200, 8N1, no flow control
//#define 	bauddivider 				(unsigned int)(1.0 * F_CPU / BAUD / 16 - 0.5)

// Schaltuhreingänge
struct {
			uint8_t Hzg ; 	///< Status Schaltuhr für Heizung
			uint8_t WW ;	///< Status Schaltuhr für Warmwasser
} Schaltuhr;

///Initialisierung
void ioinit( void );

///LED ein und Ausschalten
void LED(uint8_t mode);

/// Relais
void UWP(unsigned int speed) ;
void LPWW(uint8_t mode);
void BR(uint8_t mode);
//void ZUL(uint8_t mode);
void BWWP(uint8_t mode);

// Digitaleingang Brennerlauf
uint8_t Brennerstatus(void);

// Digitaleingang Sommer/Winter-Schalter
uint8_t Schalter_SoWi(void);

// Digitaleingang WW per Brenner-Schalter
uint8_t Schalter_WWBR(void);

// Digitaleingang BWWP an/aus
uint8_t BWWPstatus(void);

// Digitaleingang Freigaben durch Schaltuhr
void Schaltuhr_einlesen(void);

//Analogeingänge
int T_Abgas(void);
double Poti_dT_Raum_soll(void);

void uart_init( void );
//void uputchar( char c );
//void uputs( char *s );
//void uputsnl( char *s );
int uart_putchar(char c, FILE *stream); /// uart ist auf stout umgeleitet

#endif