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

///#include “gpio_mcu.h”
#include "gpio_mcu.h"

/*==================[macros and definitions]=================================*/


/*==================[internal data definition]===============================*/

typedef struct
{
	  gpio_t pin;			/*!< GPIO pin number */
	  io_t dir;			/*!< GPIO direction '0' IN;  '1' OUT*/
}     gpioConf_t;

gpioConf_t mis_pines[]={
	{GPIO_20, GPIO_OUTPUT},
	{GPIO_21, GPIO_OUTPUT},
	{GPIO_22, GPIO_OUTPUT},
	{GPIO_23, GPIO_OUTPUT}
};
/*==================[internal functions declaration]=========================*/

//FUncion que pone el dato bcd en los pines de la entradra del conversor bcd de 7 segmentos
void bcdToPin(uint8_t bcd, gpioConf_t *gpio) {   //recibe el dato en bcd
	for(int i=0; i<4; i++) {                     // y recibe el vector que contiene las estructuras
		GPIOInit(gpio[i].pin, gpio[i].dir);      // gpioConf_t con el pin y su direccion 
		if(bcd&(1 << i)) {
			GPIOOn(gpio[i].pin);
		} else GPIOOff(gpio[i].pin);
	}
}

/*==================[external functions definition]==========================*/
void app_main(void){
/* initializations */
	uint8_t bcd=4; // este numero ya es bcd o lo tengo que convertir ¡?

	bcdToPin(bcd, mis_pines);

    while(1){
		/* main loop */

	}
    
	/* program should never reach this line */
	//return 0;

	printf("Hello world!\n");
}
/*==================[end of file]============================================*/