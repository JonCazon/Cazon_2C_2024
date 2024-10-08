/*! @mainpage Template
 *
 * @section genDesc General Description
 *
 * This section describes how the program works.
 *
 * <a href="https://drive.google.com/...">Operation Example</a>
 *
 * @section hardConn Hardware Connection
 *
 * |    Peripheral  |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	PIN_X	 	| 	GPIO_X		|
 *
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 12/09/2023 | Document creation		                         |
 *
 * @author Albano Peñalva (albano.penalva@uner.edu.ar)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include "analog_io_mcu.h" // en este punto se usa
#include "uart_mcu.h"
#include "timer_mcu.h"

#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "hc_sr04.h" 
#include "lcditse0803.h"
#include "switch.h"



/*==================[macros and definitions]=================================*/

/*==================[internal data definition]===============================*/
serial_config_t puerto_serie;

unsigned char code_ECG[] = {
17,17,17,17,17,17,17,17,17,17,17,18,18,18,17,17,17,17,17,17,17,18,18,18,18,18,18,18,17,17,16,16,16,16,17,17,18,18,18,17,17,17,17,
18,18,19,21,22,24,25,26,27,28,29,31,32,33,34,34,35,37,38,37,34,29,24,19,15,14,15,16,17,17,17,16,15,14,13,13,13,13,13,13,13,12,12,
10,6,2,3,15,43,88,145,199,237,252,242,211,167,117,70,35,16,14,22,32,38,37,32,27,24,24,26,27,28,28,27,28,28,30,31,31,31,32,33,34,36,
38,39,40,41,42,43,45,47,49,51,53,55,57,60,62,65,68,71,75,79,83,87,92,97,101,106,111,116,121,125,129,133,136,138,139,140,140,139,137,
133,129,123,117,109,101,92,84,77,70,64,58,52,47,42,39,36,34,31,30,28,27,26,25,25,25,25,25,25,25,25,24,24,24,24,25,25,25,25,25,25,25,
24,24,24,24,24,24,24,24,23,23,22,22,21,21,21,20,20,20,20,20,19,19,18,18,18,19,19,19,19,18,17,17,18,18,18,18,18,18,18,18,17,17,17,17,
17,17,17

} ;
int i=0;

/*==================[internal functions declaration]=========================*/

/*==================[external functions definition]==========================*/
void Timer_ADC(){   //llama a la interrupcion cada cierto tiempo
	//AnalogStartConvertion();   // de analogico a digital
	AnalogStartContinuous(CH1);
}

void Timer_DAC(){  //digital a analogico
	//AnalogOutputWrite(uint8_t value);
	AnalogOutputWrite(code_ECG[i]);
	i++;
	if((sizeof(code_ECG)-1 )==i){
		i=0;
	}
}
void Conversion_fin(){
	uint16_t dato;
	//AnalogInputRead(CH1,&dato);
	AnalogInputReadSingle(CH1, &dato);
	UartSendString(puerto_serie.port,( char*)UartItoa(dato,10));
	//UartSendString(uart_mcu_port_t port, const char *msg);

	UartSendString(puerto_serie.port, "\r");
}



void app_main(void){
/* initializations */
	analog_input_config_t inputs;
	inputs.input= CH1;
	inputs.mode = ADC_SINGLE;
	inputs.param_p = Conversion_fin;

	puerto_serie.port = UART_PC;
	puerto_serie.baud_rate = 115200;
	//puerto_serie.param_p = ;

	AnalogInputInit(&inputs);
	//AnalogInputInit(analog_input_config_t *config);
	UartInit(&puerto_serie);
	AnalogOutputInit();


	timer_config_t timer;
	timer.timer= TIMER_A;
	timer.period= 2;
	timer.func_p= Timer_ADC;
	TimerInit(&timer);
	TimerStart(timer.timer);
	
	timer_config_t timer2;
	timer2.timer= TIMER_B;
	timer2.period= 4;
	timer2.func_p= Timer_DAC;
	TimerInit(&timer2);
	TimerStart(timer2.timer);


	//printf("Hello world!\n");
}
/*==================[end of file]============================================*/