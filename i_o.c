#include <avr/io.h>
#include "main.h"
#include "i_o.h"
#include <util/setbaud.h>
/// Setzen der I/O-Ports und des PWM-Timers
void ioinit(void) {
	//----- Port A --------
	ADCSRA = (1<<ADEN) | (1<<ADPS1) | (1<<ADPS0);    /// Frequenzvorteiler für AD-Wandlung

	DDRA &=~(	(1<<Uhr_Hzg_pin)|
				(1<<Status_BWWP_pin) | 
				(1<<Uhr_WW_pin)|
				(1<<Umschalter_WWBR_pin) | 
				(1<<Umschalter_SoWi_pin)); ///  DDRA als Eingänge 
				
	PORTA|=(1<<Uhr_Hzg_pin) | 
				(1<< Status_BWWP_pin) | 
				(1<<Uhr_WW_pin) | 
				(1<<Umschalter_WWBR_pin) |
				(1<<Umschalter_SoWi_pin);  /// Port A Pullups ein
	//----- Port B --------
	DDRB |= 	(1<<LED_pin)|
				(1<<UWP_pin)|
				(1<<WWP_pin)|
				(1<<BR_pin); /// DDRB als Ausgaenge
	//----- Port C --------
	DDRC |= (1<<BWWP_pin); /// DDRC als Ausgaenge
	DDRC &=~(1<<Status_Brenner_pin); ///  DDRC als Eingänge 
	PORTC |= (1<<Status_Brenner_pin); /// PORTC Pullups EIN		

	//----- Port D --------
	DDRD |= (1<<UWP_v_pin); // DDRD als als Ausgänge	
	// Timer, PWM
	TCCR2= (1<< WGM20 ) | (1<< COM21) | (1<< CS22) | (1<< CS20); /// PWM an OC2 / PD7 ca. 226 Hz
}
//################
/// LED schalten
void LED(uint8_t mode) { 
	if (mode) 	LED_port |= (1 << LED_pin);  // setzt Bit
	else LED_port &= ~(1 << LED_pin);   // löscht Bit
}
//################
/// Umwälzpumpe schalten
/**
@param speed  Pumpenansteuerung 1-255: PWM; 0: aus
*/
void UWP(uint16_t speed) {
// OCR2 = PWM-Duty-cycle; inverse Spannung an der Pumpe
	if (speed > 255) speed=255;
	if (speed < 0) speed=0;
	if ( speed == 0 ) {
		OCR2=128;
		UWP_port &= ~(1 << UWP_pin);    // Pumpenrelais aus
	}
	else {
		OCR2=(256-speed);
		UWP_port 	|=	(1 << UWP_pin);    // Pumpenrelais an
	}
}  		

//################
/// Ladepumpe für WW-Speicher schalten
void LPWW(uint8_t mode) {
 if (mode) WWP_port |= (1 << WWP_pin);   // setzt Bit
 else WWP_port &= ~(1 << WWP_pin);   // löscht Bit
 }
//################
/// Brenner schalten
void BR(uint8_t mode) {
  if (mode) BR_port |= (1 << BR_pin);    // setzt Bit
  else BR_port &= ~(1 << BR_pin);   // löscht Bit
}
/*################
/// Zuluftventilator schalten
void ZUL(uint8_t mode) {
 if (mode) ZUL_port |= (1 << ZUL_pin);    // setzt Bit
 else ZUL_port &= ~(1 << ZUL_pin);    // löscht Bit
}
*/
//################
/// Brauchwasser_Wärmepumpe schalten
void BWWP(uint8_t mode) {
	if (mode) BWWP_port |= (1 << BWWP_pin);    // setzt Bit
	else BWWP_port &= ~(1 << BWWP_pin);    // löscht Bit
}
//################
/// Status des Brenners lesen
uint8_t Brennerstatus(void) { //Brenner_an_input
if ( Status_Brenner_input & (1<<Status_Brenner_pin)) return(aus) ;
else return(an);
}
//##################
/// Status der Brauchwasserwärmepumpe lesen
uint8_t BWWPstatus(void) {
 if ( Status_BWWP_input & (1<<Status_BWWP_pin)) return(aus);
 else return(an);
}
//################
/// Schalter für WW per Brenner lesen
/**@retval an WW per Brenner machen
*/
uint8_t Schalter_WWBR(void)
{ //Schalter für WW per Brenner abfragen
if ( Umschalter_WWBR_input & (1<<Umschalter_WWBR_pin)) return(aus); 
else return(an);
}

//################
/// Schalter für Sommerabschaltung lesen
uint8_t Schalter_SoWi(void)
{ //Sommer-/Winter Schalter
if ( Umschalter_SoWi_input & (1<<Umschalter_SoWi_pin)) return(aus); 
else return(an);
}
//################
/// Schaltuhrzustand lesen
/**@retval struct str_schaltuhr wird direkt befüllt */
void Schaltuhr_einlesen(void) { 
	if ( Uhr_Hzg_input & (1<<Uhr_Hzg_pin)) Schaltuhr.Hzg = an;
	else Schaltuhr.Hzg=aus;
	if ( Uhr_WW_input & (1<<Uhr_WW_pin)) Schaltuhr.WW=an;
	else Schaltuhr.WW = aus;
}

/// ein AD-Kanal lesen
int16_t lese_adc(uint8_t channel) {
	uint16_t result;
	uint8_t i;
	ADMUX = (channel);     //  Gain+Kanal waehlen
	ADMUX |= (1<<REFS1) | (1<<REFS0); // interne Referenzspannung nutzen 
	/* nach Aktivieren des ADC wird ein "Dummy-Readout" empfohlen, man liest also einen Wert und verwirft diesen, um den ADC "warmlaufen zu lassen" */
	ADCSRA |= (1<<ADSC);              // eine ADC-Wandlung 
	while ( ADCSRA & (1<<ADSC) ) {    
		;     // auf Abschluss der Konvertierung warten 
	}
	result = ADCW;  // ADCW muss einmal gelesen werden, sonst wird Ergebnis der nächsten Wandlung nicht übernommen.
	// Eigentliche Messung - Mittelwert aus 4 aufeinanderfolgenden Wandlungen
	result = 0; 
	for( i=0; i<4; i++ )  {
		ADCSRA |= (1<<ADSC);            // eine Wandlung "single conversion"
		while ( ADCSRA & (1<<ADSC) ) {
			;   // auf Abschluss der Konvertierung warten
		}
		result += ADCW;		    // Wandlungsergebnisse aufaddieren
	}
  return(result/4);   // Summe durch vier teilen = arithm. Mittelwert
}// lese_adc
//--------------------------------------------------------------
/// Abgastemperatur KTY einlesen
int16_t T_Abgas(void) {
int16_t result;
  result=lese_adc(T_Abgas_pin);
  // Linearisierung bei 5,16V Vcc, 2.56V Vref, 1700 Ohm 
  if (result<888) return( (int16_t)(0.3426*result-158.33) ); 	// <888
  else 
	if (result<941)  return( (int16_t)(0.3762*result-187.45) ); // 888-940
	else  return( (int16_t)(0.3924*result-202.97)); 								// > 940
}
//################ 
/// Poti für Raumtemperaturoffset einlesen
double Poti_dT_Raum_soll(void) {
	uint16_t result;
	result= lese_adc(Poti_dT_Raum_pin);
	return(2.0*result*2.56/1024-1.25);  // Linearisierung bei 5Vcc 
}
//################ 

void uart_init( void ){
   UBRRH = UBRRH_VALUE; // aus makro setbaud.h
   UBRRL = UBRRL_VALUE;
   /*
   #if USE_2X
   UCSRA |= (1 << U2X);
   #else
   UCSRA &= ~(1 << U2X);
   #endif
   */
  UCSRA = 0; 
  UCSRC = 1<<URSEL^1<<UCSZ1^1<<UCSZ0;	//8 Bit
  UCSRB = 1<<RXEN^1<<TXEN;		//enable RX, TX
}  
   
/// UART initialisieren
/*
void uart_init( void ){
  UBRRL = bauddivider;			//set baud rate
  UBRRH = bauddivider >> 8;
  UCSRA = 0;				//no U2X, MPCM
  UCSRC = 1<<URSEL^1<<UCSZ1^1<<UCSZ0;	//8 Bit
  UCSRB = 1<<RXEN^1<<TXEN;		//enable RX, TX
}
*/
/// ein Zeichen an UART senden
/*
void uputchar( char c ) {
  while( (UCSRA & 1<<UDRE) == 0 );
  UDR = c;
}
/// eine Zeichenkette an UART senden
void uputs( char *s ) {
  while( *s ) 
     uputchar( *s++ );
}

/// eine Zeichenkette mit CR/LF an UART senden
void uputsnl( char *s ) {
  uputs( s );
  uputchar( 0x0D );
  uputchar( 0x0A);
}
*/
int uart_putchar(char c, FILE *stream) {
	if( c == '\n' )  {
		uart_putchar( '\r', stream );
	}	
	while( (UCSRA & 1<<UDRE) == 0 )
	; // warten
	UDR = c;
	return 1;
}

//--- Ende 