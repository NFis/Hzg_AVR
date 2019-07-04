//#include <stdio.h>
#include <avr/interrupt.h> //wg. cli/sei
#include "main.h" //wg. FCPU
#include <util/delay.h>
#include "i_o.h"
#include "tempmeas.h"
//#include <avr/pgmspace.h>

void Filterung_kurz_alle(void);
/// IDs für One - Wire - Sensoren
/** Sensor IDs: Länge 8 Bytes, Konstante Stellen:  [0]= 0x10 [4]= 0x01 [5]= 0x08 [6]= 0x00 */
//const uint8_t sensor_id[10][4] PROGMEM =  {
const uint8_t sensor_id[10][4]  =  {
  { 0x2E, 0x27, 0x6D, 0x27 },
  { 0x9F, 0xF5, 0x6C, 0x45 },
  { 0xAF, 0x1E, 0x6D, 0x8D },
  { 0xB8, 0x00, 0x6D, 0x8D }, 
  { 0x13, 0x1C, 0x6D, 0x74 }, 
  { 0x80, 0x36, 0x6D, 0xD9 },  
  { 0x4A, 0x11, 0x6D, 0x20 },  
  { 0xD5, 0xF0, 0x6C, 0x2D },
  { 0x13, 0x24, 0x6D, 0xCE },
  { 0x94, 0x0D, 0x6D, 0xAA }
};
//-------------------------------------------
/// One-Wire - Reset
uint8_t ow_reset(void) {
  unsigned char err;
  OW_OUT &= ~(1<<OW_PIN);
  OW_DDR |= 1<<OW_PIN;
  _delay_us( 480 );			// 480 us
  cli();
  OW_DDR &= ~(1<<OW_PIN);
  _delay_us( 66); // 66
  err = OW_IN & (1<<OW_PIN);			// no presence detect
  sei();
  _delay_us( 480-66);
  if( (OW_IN & (1<<OW_PIN)) == 0 )   err = 1;		// short circuit
  return err;
}
//-------------------------------------------
/// One-Wire - Bit-I/O
uint8_t ow_bit_io( uint8_t b ) {
  cli();
  OW_DDR |= 1<<OW_PIN;
  _delay_us(1);
  if( b ) {
	OW_DDR &= ~(1<<OW_PIN);
	_delay_us(10-1); // beim LESEN kurzer delay
  }
  else {
    _delay_us(15-1); //15-1
  }
  if( (OW_IN & (1<<OW_PIN)) == 0 )  b = 0;
  _delay_us(60-15); 
  OW_DDR &= ~(1<<OW_PIN);
  sei();
  return b;
}
//-------------------------------------------
/// One-Wire - Byte-Write 
uint8_t ow_byte_write( uint8_t b ) {
  unsigned char i,  j;
  for (i=8; i>0; i--) {
     j = ow_bit_io( b & 1 );
    b >>= 1;
    if ( j ) b |= 0x80;
  }
  return b;
}
//-------------------------------------------
/// One-Wire - Byte-Read 
uint8_t ow_byte_read( void ) {
  return ow_byte_write( 0xFF );
}
//-------------------------------------------
/// One-Wire - Befehl senden 
/**@param command One-Wire Befehl
*@retval Error*/
void ow_command(uint8_t command) {
  ow_reset();
  ow_byte_write( SKIP_ROM );
  ow_byte_write( command );
}
//-------------------------------------------
/// One-Wire - einen Sensormesswert lesen 
/** führt Reset aus und sendet MATCH_ROM, gefolgt von einer Sensor-ID; 
* danach muss mind. 800ms auf die Wandlung gewartet werden!
*@param sensor	Nr des Sensors, der adressiert werden soll (Index in Array sa[])
*@retval e Error*/
uint8_t ow_read_sensor(uint8_t sensor) {
  unsigned char err;
  err=ow_reset();
	ow_byte_write( MATCH_ROM );	// to a single device
	//Schleife aufgelöst:
	ow_byte_write(0x10);   				// 0
	ow_byte_write(sensor_id[sensor][0]);	// 1   ; 
			//ow_byte_write(pgm_read_byte(&(sensor_id[sensor][0])));
	ow_byte_write(sensor_id[sensor][1]);	// 2
			//ow_byte_write(pgm_read_byte(&(sensor_id[sensor][1])));
	ow_byte_write(sensor_id[sensor][2]);	// 3 
		//ow_byte_write(pgm_read_byte(&(sensor_id[sensor][2])));
	ow_byte_write(0x01); 		
	ow_byte_write(0x08);
	ow_byte_write(0x00);
	ow_byte_write(sensor_id[sensor][3]);	// 7
		//ow_byte_write(pgm_read_byte(&(sensor_id[sensor][3])));
	ow_byte_write(READ);
  return(err);
}
//-------------------------------------------
/// One-Wire - Messung in allen Sensoren starten 
/** sendet CONVERT_T an alle Sensoren und aktiviert Pullup für parasitäre Versorgung*/
uint8_t ow_start_meas( void ){
	uint8_t err=0;	
	if ( OW_IN & 1<< OW_PIN ) {
		ow_command(CONVERT_T);
		OW_OUT |= 1<< OW_PIN;
		OW_DDR |= 1<< OW_PIN;			// parasite power on
	}
	else {
		printf( "#1W:KS sm\n" );
		err=1;
	}
return(err);	
}
//-------------------------------------------
/// One-Wire - Messwerte aus alle Sensoren einlesen 
/** alle Sensoren werden einzeln adressiert; IDs liegen im Array sa[]
*@retval T_xx.a	aktuelle Temperaturwerte werden direkt in die jeweilige Temperatur-Struct gechrieben
*@retval e Error*/
uint8_t ow_read_meas( void ) {
	uint8_t err=0, sensor, j, sp[12];
	double temperatur;
	for (sensor = 0; sensor<9; sensor++) {
		if ( (err==0) && (ow_read_sensor(sensor)==0) ) {
			for (j=0; j<8; j++) { 
				sp[j]=ow_byte_read();  // scratchpad auslesen
			}	
			if (sp[0]==0xAA) {
				printf("#1W:85 rm\n" );
				err=1;
				return(err);
			}	
			//                 ganze Grade           0 (pos.) oder 80 (neg.)             0.1°C Auflösung;    sp[1] ist =0xFF bei neg. Temp.
			temperatur= (uint8_t)(sp[0]/2) - (uint8_t)((sp[1]&0x01)*0x80) - 0.25+(16-sp[6])/16.0;  
			// if-Struktur ist hier codesparender als switch...case!
			if (sensor == 0  && temperatur> 0 )  T_VL.a= temperatur; 
			if (sensor == 1 && temperatur > 0)	 T_RL.a=temperatur; 
			if (sensor == 2 && temperatur > -25.0) {
																	T_Aussen.a =temperatur; 
																	T_AussenTag.a=temperatur; // für Langzeitmittelwert
			}						
			if (sensor == 3 && temperatur>0) T_WW.a=temperatur; 
			if (sensor == 4 && temperatur>0) T_Kessel.a =temperatur; 
			if (sensor == 5 && temperatur>0) T_R1.a=temperatur;// ZL.T=temperatur; 
			if (sensor == 6 && temperatur>0) T_R2.a=temperatur; //WWVL.T=temperatur; 
			if (sensor == 7 && temperatur>0) T_Raum2.a=temperatur; // großer Raum OG
			if (sensor == 8 && temperatur>0) T_WoZi.a=temperatur; // WoZi
		}
		else {
			err=1;
		}	
	}
return(err);	
}

// Filterung ======================================================
/// Filterung kurz, jede Sekunde
/** //Bsp: T_f	= T_f - (T_f  -	T)/60.0; 	*/
void Filterung_kurz(struct str_temperatur* Temp) {
	Temp->k = Temp->k- (Temp->k - Temp->a)/zk_xx_k; // für alle gleich
}

/// Filterung lang, jede Minute
/**@param zk Zeitkonstante */
void Filterung_lang(struct str_temperatur* Temp, double zk) {
	Temp->l = Temp->l- (Temp->l- Temp->k)/zk;
}
/// Filterung initialisieren
/**@param Ponter auf Temperaturstruktur
*@retval gefilterte Werte werden auf akt. Messwert gesetzt
*/
void Filter_init(struct str_temperatur* Temp) {
	Temp->k  = Temp->a;
	Temp->l  = Temp->a;
} 