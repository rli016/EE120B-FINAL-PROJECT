#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include <string.h>
#include "io.c"
volatile unsigned char TimerFlag = 0; // TimerISR() sets this to 1. C programmer should clear to 0.
// Internal variables for mapping AVR's ISR to our cleaner TimerISR model.
unsigned long _avr_timer_M = 1; // Start count from here, down to 0. Default 1 ms.
unsigned long _avr_timer_cntcurr = 0; // Current internal count of 1ms ticks
unsigned char h0 = 1;
unsigned char h1 = 1;
unsigned char m0 = 9;
unsigned char m1 = 5;
unsigned char s0 = 0;
unsigned char s1 = 5;
unsigned char ah0 = 2;
unsigned char ah1 = 1;
unsigned char am0 = 0;
unsigned char am1 = 0;
unsigned char as0 = 0;
unsigned char as1 = 0;
unsigned long timer = 0;
bool atf = true;
bool alarm = false;
bool snooze = false;
bool flash = true;
unsigned char tmpA = 0;
unsigned char counter = 9;
bool tf = false;
bool contTIME = false;
bool contALARM = false;
bool noDisplay = false;
bool alarmEN = false;
void TimerOn() {
	// AVR timer/counter controller register TCCR1
	TCCR1B = 0x0B;// bit3 = 0: CTC mode (clear timer on compare)
	// bit2bit1bit0=011: pre-scaler /64
	// 00001011: 0x0B
	// SO, 8 MHz clock or 8,000,000 /64 = 125,000 ticks/s
	// Thus, TCNT1 register will count at 125,000 ticks/s

	// AVR output compare register OCR1A.
	OCR1A = 125;	// Timer interrupt will be generated when TCNT1==OCR1A
	// We want a 1 ms tick. 0.001 s * 125,000 ticks/s = 125
	// So when TCNT1 register equals 125,
	// 1 ms has passed. Thus, we compare to 125.
	// AVR timer interrupt mask register
	TIMSK1 = 0x02; // bit1: OCIE1A -- enables compare match interrupt

	//Initialize avr counter
	TCNT1=0;

	_avr_timer_cntcurr = _avr_timer_M;
	// TimerISR will be called every _avr_timer_cntcurr milliseconds

	//Enable global interrupts
	SREG |= 0x80; // 0x80: 1000000
}

void TimerOff() {
	TCCR1B = 0x00; // bit3bit1bit0=000: timer off
}

void TimerISR() {
	TimerFlag = 1;
}

// In our approach, the C programmer does not touch this ISR, but rather TimerISR()
ISR(TIMER1_COMPA_vect) {
	// CPU automatically calls when TCNT1 == OCR1 (every 1 ms per TimerOn settings)
	_avr_timer_cntcurr--; // Count down to 0 rather than up to TOP
	if (_avr_timer_cntcurr == 0) { // results in a more efficient compare
		TimerISR(); // Call the ISR that the user uses
		_avr_timer_cntcurr = _avr_timer_M;
	}
}

// Set TimerISR() to tick every M ms
void TimerSet(unsigned long M) {
	_avr_timer_M = M;
	_avr_timer_cntcurr = _avr_timer_M;
}
void set_PWM(double frequency) {
	static double current_frequency;
	if (frequency != current_frequency) {
		if (!frequency) { TCCR3B &= 0x08; }
		else { TCCR3B |= 0x03; }
		if (frequency < 0.954) { OCR3A = 0xFFFF; }
		else if (frequency > 31250) { OCR3A = 0x0000; }
		else { OCR3A = (short)(8000000 / (128 * frequency)) - 1; }
		TCNT3 = 0;
		current_frequency = frequency;
	}
}

void PWM_on() {
	TCCR3A = (1 << COM3A0);
	TCCR3B = (1 << WGM32) | (1 << CS31) | (1 << CS30);
	set_PWM(0);
}

void PWM_off() {
	TCCR3A = 0x00;
	TCCR3B = 0x00;
}
void CLOCK_FSM()
{
	if(alarm == false && noDisplay == false){
		LCD_Cursor(1);
		LCD_WriteData(h1 + '0');
		LCD_Cursor(2);
		LCD_WriteData(h0 + '0');
		LCD_Cursor(3);
		LCD_WriteData(58);
		LCD_Cursor(4);
		LCD_WriteData(m1 + '0');
		LCD_Cursor(5);
		LCD_WriteData(m0 + '0');
		LCD_Cursor(6);
		LCD_WriteData(58);
		LCD_Cursor(7);
		LCD_WriteData(s1 + '0');
		LCD_Cursor(8);
		LCD_WriteData(s0 + '0');
		if(tf){
			LCD_Cursor(9);
			LCD_WriteData(97);
			LCD_Cursor(10);
			LCD_WriteData(109);
		}
		else{
			LCD_Cursor(9);
			LCD_WriteData(112);
			LCD_Cursor(10);
			LCD_WriteData(109);
		}
	}
	LCD_Cursor(0);
}
void TIME_FSM(){
	s0++;
	if(s0 > 9){
		s0 = 0;
		s1++;
	}
	if(s1 > 5){
		s1 = 0;
		m0++;
	}
	if(m0 > 9){
		m0 = 0;
		m1++;
	}
	if(m1 > 5){
		m1 = 0;
		h0++;
	}
	if(h0 > 9){
		h0 = 0;
		h1++;
	}
	if(h1 == 1 && h0 == 2 && m1 == 0 && m0 == 0 && s1 == 0 && s0 == 0)
		tf = !tf;
	if(h1 == 1 && h0 > 2){
		h1 = 0;
		h0 = 1;
	}
	if(h1 > 1)
		h1 = 0;
}
void SET_FSM()
{
	if(alarm)
		return;
	LCD_Cursor(17);
	LCD_WriteData(83);
	LCD_WriteData(69);
	LCD_WriteData(84);
	LCD_WriteData(32);
	LCD_WriteData(84);
	LCD_WriteData(73);
	LCD_WriteData(77);
	LCD_WriteData(69);
	LCD_Cursor(counter);
	if(tmpA == 0x01){
		if (counter == 9)
			tf = !tf;
		else if (counter == 8){
			s0++;
			if(s0 > 9)
				s0 = 0;
		}
		else if (counter == 7){
			s1++;
			if(s1 > 5)
				s1 = 0;
		}
		else if (counter == 5){
			m0++;
			if(m0 > 9)
				m0 = 0;
		}
		else if (counter == 4){
			m1++;
			if(m1 > 5)
				m1 = 0;
		}
		else if (counter == 2){
			h0++;
			if(h0 > 2)
				h1 = 0;
			if(h0 > 9)
				h0 = 0;
		}
		else if (counter == 1){
			h1++;
			if(h1 > 1)
				h1 = 0;
		}
	}
	else if(tmpA == 0x02){
		counter--;
	}
	if(counter == 3 || counter == 6)
		counter--;
	if(counter < 1)
		counter = 9;
}
void ALARM_FSM(){
	if(alarm)
		return;
	LCD_Cursor(1);
	LCD_WriteData(ah1 + '0');
	LCD_Cursor(2);
	LCD_WriteData(ah0 + '0');
	LCD_Cursor(3);
	LCD_WriteData(58);
	LCD_Cursor(4);
	LCD_WriteData(am1 + '0');
	LCD_Cursor(5);
	LCD_WriteData(am0 + '0');
	LCD_Cursor(6);
	LCD_WriteData(58);
	LCD_Cursor(7);
	LCD_WriteData(as1 + '0');
	LCD_Cursor(8);
	LCD_WriteData(as0 + '0');
	if(atf){
		LCD_Cursor(9);
		LCD_WriteData(97);
		LCD_Cursor(10);
		LCD_WriteData(109);
	}
	else{
		LCD_Cursor(9);
		LCD_WriteData(112);
		LCD_Cursor(10);
		LCD_WriteData(109);
	}
	LCD_Cursor(17);
	LCD_WriteData(83);
	LCD_WriteData(69);
	LCD_WriteData(84);
	LCD_WriteData(32);
	LCD_WriteData(65);
	LCD_WriteData(76);
	LCD_WriteData(65);
	LCD_WriteData(82);
	LCD_WriteData(77);
	LCD_Cursor(counter);
	if(tmpA == 0x01){
		if (counter == 9)
			atf = !atf;
		else if (counter == 8){
			as0++;
			if(as0 > 9)
			as0 = 0;
		}
		else if (counter == 7){
			as1++;
			if(as1 > 5)
			as1 = 0;
		}
		else if (counter == 5){
			am0++;
			if(am0 > 9)
			am0 = 0;
		}
		else if (counter == 4){
			am1++;
			if(am1 > 5)
			am1 = 0;
		}
		else if (counter == 2){
			ah0++;
			if(ah0 > 2)
			ah1 = 0;
			if(ah0 > 9)
			ah0 = 0;
		}
		else if (counter == 1){
			ah1++;
			if(ah1 > 1)
				ah1 = 0;
			if(ah0 > 2)
				ah0 = 0;
		}
	}
	else if(tmpA == 0x02){
		counter--;
	}
	if(counter == 3 || counter == 6)
		counter--;
	if(counter < 1)
		counter = 9;
}
void CHECK_FSM(){
	if(alarmEN == false)
		return;
	if(contTIME == false && contALARM == false){
		LCD_Cursor(17);
		LCD_WriteData(65);
		LCD_WriteData(76);
		LCD_WriteData(65);
		LCD_WriteData(82);
		LCD_WriteData(77);
		LCD_WriteData(32);
		LCD_WriteData(79);
		LCD_WriteData(78);
		LCD_WriteData(32);
		LCD_WriteData(32);
		LCD_Cursor(0);
	}
	if(h1 == ah1 && h0 == ah0 && m1 == am1 && m0 == am0 && s1 == as1 && s0 == as0 && tf == atf){
		alarm = true;
	}
	if(timer < 120 && alarm == true){
		if(timer % 2 == 0)
			set_PWM(523.25);
		else
			set_PWM(0);
		noDisplay = true;
		if(flash){
			LCD_Cursor(1);
			LCD_WriteData(87);
			LCD_WriteData(65);
			LCD_WriteData(75);
			LCD_WriteData(69);
			LCD_WriteData(32);
			LCD_WriteData(85);
			LCD_WriteData(80);
			LCD_WriteData(33);
			LCD_WriteData(33);
			LCD_WriteData(33);
			LCD_Cursor(0);
		}
		else{
			LCD_Cursor(1);
			LCD_WriteData(32);
			LCD_WriteData(32);
			LCD_WriteData(32);
			LCD_WriteData(32);
			LCD_WriteData(32);
			LCD_WriteData(32);
			LCD_WriteData(32);
			LCD_WriteData(32);
			LCD_WriteData(32);
			LCD_WriteData(32);
			LCD_Cursor(0);
		}
		flash = !flash;
		timer++;
	}
	else if (!snooze){
		noDisplay = false;
		alarm = false;
		set_PWM(0);
		timer = 0;
	}
	if(tmpA > 0 && alarm == true){
		noDisplay = false;
		snooze = true;
		timer = 0;
		set_PWM(0);
	}
	if(snooze){
		timer++;
		if(timer < 60){
			alarm = false;
		}
		else{
			alarm = true;
			snooze = false;
			timer = 0;
		}
	}
}
int main(void) {
	DDRA = 0x00; PORTA = 0xFF;
	DDRB = 0xFF; PORTB = 0x00;
	DDRC = 0xFF; PORTC = 0x00;
	DDRD = 0xFF; PORTD = 0x00;
	unsigned long ClockTimeElapsed = 0;
	unsigned long TimeElapsed = 0;
	unsigned long SetTimeElapsed = 0;
	unsigned long SetAlarmElapsed = 0;
	unsigned long CheckAlarmElapsed = 0;
	PWM_on();
	LCD_init();
	TimerSet(1);
	TimerOn();
	while(1) {
		tmpA = ~PINA;
		if(TimeElapsed >= 1000){
			TIME_FSM();
			TimeElapsed = 0;
		}
		if(ClockTimeElapsed >= 250){
			CLOCK_FSM();
			ClockTimeElapsed = 0;
		}
		if(SetTimeElapsed >= 150){
			if(tmpA == 0x04 && !contALARM)
				contTIME = !contTIME;
			if(contTIME && !contALARM)
				SET_FSM();
			else if(!contTIME && contALARM == false && alarmEN == false){
				counter = 9;
				LCD_Cursor(17);
				LCD_WriteData(32);
				LCD_WriteData(32);
				LCD_WriteData(32);
				LCD_WriteData(32);
				LCD_WriteData(32);
				LCD_WriteData(32);
				LCD_WriteData(32);
				LCD_WriteData(32);
				LCD_Cursor(0);
			}
			SetTimeElapsed = 0;
		}
		if (SetAlarmElapsed >= 150)
		{
			if(tmpA == 0x08 && !contTIME)
				contALARM = !contALARM;
			if(contALARM && !contTIME){
				noDisplay = true;
				ALARM_FSM();
			}
			else if(!contALARM && contTIME == false && alarmEN == false){
				noDisplay = false;
				counter = 9;
				LCD_Cursor(17);
				LCD_WriteData(32);
				LCD_WriteData(32);
				LCD_WriteData(32);
				LCD_WriteData(32);
				LCD_WriteData(32);
				LCD_WriteData(32);
				LCD_WriteData(32);
				LCD_WriteData(32);
				LCD_WriteData(32);
				LCD_Cursor(0);
			}
			SetAlarmElapsed = 0;
		}
		if(CheckAlarmElapsed >= 250){
			if (tmpA == 0x10){
				timer = 0;
				snooze = false;
				alarm = false;
				set_PWM(0);
				alarmEN = !alarmEN;
			}
			CHECK_FSM();
			if(alarmEN == false && contTIME == false && contALARM == false){
				LCD_Cursor(17);
				LCD_WriteData(32);
				LCD_WriteData(32);
				LCD_WriteData(32);
				LCD_WriteData(32);
				LCD_WriteData(32);
				LCD_WriteData(32);
				LCD_WriteData(32);
				LCD_WriteData(32);
				LCD_Cursor(0);
			}
			CheckAlarmElapsed = 0;
		}
		while (!TimerFlag){
			continue;
		}
		TimerFlag = 0;
		TimeElapsed += 1;
		SetTimeElapsed += 1;
		ClockTimeElapsed += 1;
		SetAlarmElapsed += 1;
		CheckAlarmElapsed += 1;
	}
}