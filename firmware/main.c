/*
 * Project: Smart Power Bank Keep-Alive
 * Author: Zak Kemble, contact@zakkemble.net
 * Copyright: (C) 2020 by Zak Kemble
 * License: 
 * Web: https://blog.zakkemble.net/smart-power-bank-keep-alive/
 */

#include <stdint.h>
#include <avr/io.h>
#include <avr/power.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
//#include <util/delay.h>

// PB0: Unused
// PB1: LED output (HIGH = on)
// PB2: CC enable output (LOW = enable)

#define WDT_INT_RESET()		(WDTCSR |= _BV(WDIE)|_BV(WDE)) // NOTE: Setting WDIE also enables global interrupts
#define WDT_TIMEDOUT()		(!(WDTCSR & _BV(WDIE)))

void get_rstflr(void) __attribute__((naked, used, section(".init3")));
void get_rstflr()
{
	RSTFLR = 0;
	wdt_disable();
}

static void waitForWDT(uint8_t wdto)
{
	wdt_enable(wdto);
	WDT_INT_RESET();
	
	sleep_enable();
	//sleep_bod_disable();
	//sei();
	sleep_cpu();
	sleep_disable();
}

int main(void)
{
	clock_prescale_set(CPU_DIV);

	// Configure pins
	DDRB |= _BV(DDB2)|_BV(DDB1); // Outputs
	PUEB |= _BV(PUEB0); // Pullups

	// Configure LED PWM fade timer
	TCCR0A = _BV(WGM00);
	TCCR0B = _BV(WGM02)|_BV(CS01);

	ACSR = _BV(ACD); // Power off analog comparator
	power_all_disable(); // Power off everything else

	sei();

	while(1)
	{
		// On period
		PORTB = _BV(PORT1);
		set_sleep_mode(SLEEP_MODE_IDLE);

		// Enable LED PWM timer
		power_timer0_enable();
		OCR0B = 0;
		TCCR0A |= _BV(COM0B1);

		// ~500ms
		// LED Fade in
		for(uint8_t i=0;i<32;i++)
		{
			waitForWDT(WDTO_15MS);
			OCR0BL = (i < 31) ? OCR0BL + 8 : 255;
		}

		// Disable LED PWM timer
		TCCR0A &= ~_BV(COM0B1);
		power_timer0_disable();

		// Wait for a bit
		set_sleep_mode(SLEEP_MODE_PWR_DOWN);
		waitForWDT(WDTO_2S);
		set_sleep_mode(SLEEP_MODE_IDLE);

		// Enable LED PWM timer
		power_timer0_enable();
		TCCR0A |= _BV(COM0B1);

		// ~500ms
		// LED Fade out
		for(uint8_t i=0;i<32;i++)
		{
			waitForWDT(WDTO_15MS);
			OCR0BL = (i < 31) ? OCR0BL - 8 : 0;
		}

		// Disable LED PWM timer
		TCCR0A &= ~_BV(COM0B1);
		power_timer0_disable();



		// Off period
		PORTB = _BV(PORT2);
		set_sleep_mode(SLEEP_MODE_PWR_DOWN);
		waitForWDT(WDTO_4S);
		waitForWDT(WDTO_2S);
	}
}

EMPTY_INTERRUPT(WDT_vect);
