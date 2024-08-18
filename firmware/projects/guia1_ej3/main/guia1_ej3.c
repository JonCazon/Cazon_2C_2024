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
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led.h"
#include "switch.h"

/*==================[macros and definitions]=================================*/
#define CONFIG_BLINK_PERIOD 1000
#define retardo 100
/*==================[internal data definition]===============================*/
typedef struct leds
{
	uint8_t mode;       //ON=1, OFF=0, TOGGLE=2
	uint8_t n_led;       // indica el número de led a controlar
	uint8_t n_ciclos;   //indica la cantidad de ciclos de encendido/apagado
	uint32_t periodo;    //indica el tiempo de cada ciclo
	
} my_leds;

/*==================[internal functions declaration]=========================*/

void Funcion_Led (struct leds* puntero_Eled){
	   switch(puntero_Eled->mode){
    		case 1:
    			LedOn(LED_1);
				LedOn(LED_2);
				LedToggle(LED_3);
    		   break;
    		case 0:
    			LedOff(LED_1);
				LedOff(LED_2);
				LedOff(LED_3);
    	      break;
		
    	                       }
		if (puntero_Eled->mode==2)
		{   switch(puntero_Eled->n_led){
    		case 1:
    			LedToggle(LED_1);
    		   break;
    		case 2:
    			LedToggle(LED_2);
    	      break;
			  case 3:
    			LedToggle(LED_3);
    	      break;
		
    	                       }}

		    
		}
	
	
	   
		


/*==================[external functions definition]==========================*/
void app_main(void){

	LedsInit();
	my_leds led;
	led.mode =2;
	led.n_led=LED_3;
	led.n_ciclos= 5;
	led.periodo= 3000000;
	Funcion_Led(&led);
while (1){

}

	printf("Hello world!\n");
}
/*==================[end of file]============================================*/