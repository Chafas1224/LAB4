/*
 * Prelab_4.c
 *
 * Created: 7/04/2024 23:02:55
 * Author : pablo
 */ 
#define F_CPU 16000000UL // Definir la frecuencia del reloj del microcontrolador (en este caso, 16 MHz)
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

uint8_t contador_var = 0; // Variable para almacenar el valor del contador
uint8_t digito1;
uint8_t digito2;
uint8_t display1 = 0x00;
uint8_t display2 = 0x00;
uint8_t v_adc = 0x00;

void initADC(void);
void setup() {
	cli();
	// Configurar pines de entrada para los botones
	PORTC |= (1 << PC2) | (1 << PC3); // Habilitar resistencias pull-up internas para los botones
	DDRC &= ~((1 << PC2) | (1 << PC3));
	
	// Configurar pines de salida para los LEDs
	DDRB |= 0x3F; // Los primeros 6 pines del puerto B
	DDRC |= 0x33; // Los dos primeros pines del puerto C
	DDRD |= 0xFF; // Todos los dígitos del puerto D
	PORTD = 0;
	UCSR0B = 0;
	initADC();
	sei(); // Habilitar interrupciones globales
}

uint8_t lista7seg[16] = {0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F,0x77,0x7C,0x39,0x5E,0x79,0x71};

void actualizar_leds() {
	// Función para cargar el valor del contador en los LEDs
	PORTB = (PORTB & 0xC0) | (contador_var & 0x3F);
	PORTC = (PORTC & 0xFC) | ((contador_var >> 6) & 0x03);

	// Mostrar el valor del ADC en los LEDs de 7 segmentos
}

int main(void) {
	setup();
	digito1 = v_adc >> 4; // Los 4 bits más significativos
	digito2 = v_adc & 0x0F; // Los 4 bits menos significativos

	while (1) {
		ADCSRA |= (1<<ADSC);
		PORTD = lista7seg[digito2];
		PORTC |= (1 << PC4);
		_delay_ms(2);
		PORTC &= ~(1 << PC4);

		PORTD = lista7seg[digito1];
		PORTC |= (1 << PC5);
		_delay_ms(2);
		PORTC &= ~(1 << PC5);
		// Verificar si se presiona el botón de suma
		if (!(PINC & (1 << PC2))) {

			// Antirrebote
			_delay_ms(8);
			
			// Verificar nuevamente si sigue presionado
			if (!(PINC & (1 << PC2))) {
				if (contador_var < 255)
				contador_var++;
				else
				contador_var = 0;
				actualizar_leds();
			}
		}

		// Verificar si se presiona el botón de decremento
		if (!(PINC & (1 << PC3))) {

			// Antirrebote
			_delay_ms(8);
	
			// Verificar nuevamente si sigue presionado
			if (!(PINC & (1 << PC3))) {
				if (contador_var > 0)
				contador_var--;
				else
				contador_var = 255;
				actualizar_leds();
			}
		}
	}
}

void initADC(void) {
	ADMUX = 0; // INICIO EN 0
	// REFERENCIA AVCC = 5V
	ADMUX |= (1 << REFS0);
	ADMUX &= ~(1 << REFS1);
	// SELECCIONO EL ADC[5] = 0110
	ADMUX |= (1 << MUX2) | (1 << MUX1);
	
	// JUSTIFICACION A LA IZQUIERDA
	ADMUX |= (1 << ADLAR);
	
	ADCSRA = 0;
	
	
	// HABILITAR LA INTERRUPCION DEL ADC
	ADCSRA |= (1 << ADIE);
	
	// HABILITAMOS EL PRESCALER 128  FADC = 125 KHz
	ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
	
	// HABILITAMOS EL ADC
	ADCSRA |= (1 << ADEN);

}

void alarma(){
	if (v_adc > contador_var){
		PORTD |= (1<<PD7);
	}
	else{
		PORTD &= ~ (1<<PD7);
	}
	
}
ISR(ADC_vect) {
	v_adc = ADCH;
	digito1 = v_adc >> 4; // Los 4 bits más significativos
	digito2 = v_adc & 0x0F; // Los 4 bits menos significativos
	alarma();
	ADCSRA |= (1<<ADIF); // Resetear la bandera de ADC
	
}