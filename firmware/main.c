// Smart Power Bank Keep-Alive
// https://blog.zakkemble.net/smart-power-bank-keep-alive/
// https://github.com/ZakKemble/SmartPowerBankKeepAlive
// Copyright (C) 2023, Zak Kemble

#include <stdint.h>
#include <avr/io.h>
#include <avr/power.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>

// PB0: Unused / Off-time ADC0 input
// PB1: LED output (HIGH = on)
// PB2: CC enable output (LOW = enable)

#define ADC_VAL_PER_SEC		11

#define WDT_INT_RESET()		(WDTCSR |= _BV(WDIE)|_BV(WDE)) // NOTE: Setting WDIE also enables global interrupts
#define WDT_TIMEDOUT()		(!(WDTCSR & _BV(WDIE)))

void __attribute__((naked, used, section(".init3"))) get_rstflr(void)
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

static void fadeIn(void)
{
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

	set_sleep_mode(SLEEP_MODE_PWR_DOWN);
}

static void fadeOut(void)
{
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

	set_sleep_mode(SLEEP_MODE_PWR_DOWN);
}

static inline void ccOn(void)
{
	PORTB = _BV(PORT1);
}

static inline void ccOff(void)
{
	PORTB = _BV(PORT2);
}

static void waitOn(void)
{
	waitForWDT(WDTO_2S);
}

static void waitOff(uint8_t seconds)
{
	while(1)
	{
		waitForWDT(WDTO_1S);
		seconds -= ADC_VAL_PER_SEC;
		if(seconds > (255 - ADC_VAL_PER_SEC))
			break;
	}
}

static uint8_t getOffTime(void)
{
	power_adc_enable();
	ADCSRA = _BV(ADEN)|_BV(ADSC)|_BV(ADPS2)|_BV(ADPS0); // Div32 = 125KHz
	while(ADCSRA & _BV(ADSC));
	uint8_t adcTimeVal = ADCL;
	power_adc_disable();
	
	// 5k pot with 1k on Vin + internal pullup
	// @5V
	// Max: 4.16V = 212 (219 with +25% tolerance, 201 with -25% tolerance)
	// Min: 0V = 0
	
	// If the pot has gone bad (wiper has disconnected) then the ADC input
	// will be pulled up to VCC via the internal pullup and read ~255.
	// Default to 6 seconds when this happens.
	if(adcTimeVal > 220)
		adcTimeVal = ADC_VAL_PER_SEC * 6;
	
	return adcTimeVal;
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

	// Configure ADC
	//ADMUX = 0; // ADC0/PB0
	DIDR0 = _BV(ADC0D);

	ACSR = _BV(ACD); // Power off analog comparator
	power_all_disable(); // Power off everything else

	sei();

	while(1)
	{
		// On period
		ccOn();
		fadeIn();
		waitOn();
		fadeOut();

		// Off period
		uint8_t offTime = getOffTime();
		ccOff();
		waitOff(offTime);
	}
}

EMPTY_INTERRUPT(WDT_vect);
