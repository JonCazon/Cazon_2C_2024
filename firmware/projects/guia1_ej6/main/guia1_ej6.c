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
 * | 30/08/2024 | Document creation		                         |
 *
 * @author Cazon Jonatan (cuarla17@gmail.com)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>

#include "gpio_mcu.h"

/*==================[macros and definitions]=================================*/
/** @struct typedef struct  
*  @brief gpio_t pin	 GPIO pin number
*	@brief  io_t dir	 GPIO direction '0' IN;  '1' OUT
*  @brief  gpioConf_t   nombre de la structura con el numero de pin y la direccion del pin
*/

typedef struct
{
	gpio_t pin;			/*!< GPIO pin number */
	io_t dir;			/*!< GPIO direction '0' IN;  '1' OUT*/
} gpioConf_t;       // nombre de la structura con el numero de pin y la direccion del pin?

/*==================[internal data definition]===============================*/
/** @def mis_pines
 * @brief vector que contiene estructuras de gpioConf_t, pines (datos en BCD) que serian los pines de entrada del decodificador de 7 segmentos
*/

gpioConf_t mis_pines[]={      // vector que contiene estructuras de gpioConf_t
	{GPIO_20, GPIO_OUTPUT},   // pines (datos en BCD) que serian los pines de entrada del decodificador
	{GPIO_21, GPIO_OUTPUT},   //  de 7 segmentos 
	{GPIO_22, GPIO_OUTPUT},
	{GPIO_23, GPIO_OUTPUT}
};
/** @def  pinSelector[]
 *  @brief vector que contiene los pines que mapean, segune el dato bcd se vera que display se prende o no
 *  
 */
gpioConf_t pinSelector[]={          //vector que contiene los pines que mapean 
		{GPIO_19, GPIO_OUTPUT},   //segune el dato bcd se vera que display se prende o no
		{GPIO_18, GPIO_OUTPUT},
		{GPIO_9, GPIO_OUTPUT}
};
/*==================[internal functions declaration]=========================*/

/*==================[external functions definition]==========================*/
 //Funcion que convierte un numero (data) decinal a numero en bcd
 /** @fn convertToBcdArray (uint32_t data, uint8_t digits, uint8_t * bcd_number)
  * @brief Funcion que convierte un numero (data) decinal a numero en bcd
  * @param [data]
  * @param [digit]
  * @param [bcd_number]
  * @return
  */
int8_t convertToBcdArray (uint32_t data, uint8_t digits, uint8_t * bcd_number)
{
	int i=0;                             //digits= cuantos digitos tiene el numero
	for (i=0; i<digits; i++){
		bcd_number[(digits-1-i)]=data%10;  // *bcd_number es el puntero al verctor donde se va a guardar el numeor convertido
		data=data/10;
	}
return 0;
}
//FUncion que pone el dato bcd en los pines de la entradra del conversor bcd de 7 segmentos
/** @fn bcdToPin(uint8_t bcd, gpioConf_t *gpio)
  * @brief Funcion que pone el dato bcd en los pines de la entradra del conversor bcd de 7 segmentos
  * @param [bcd]
  * @param [gpio]
  * 
  * @return
  */
void bcdToPin(uint8_t bcd, gpioConf_t *gpio) {
	for(int i=0; i<4; i++) {                       //recibe el dato en bcd
		GPIOInit(gpio[i].pin, gpio[i].dir);        // y recibe el vector que contiene las estructuras
		if(bcd&(1 << i)) {                         // gpioConf_t con el pin y su direccion 
			GPIOOn(gpio[i].pin);
		} else GPIOOff(gpio[i].pin);
	}
}
//Funcion que integra las funciones anteriores para tomar el numero en decimal lo convierte 
// a bcd y luego se tiene que mostrar en los display de 7 segmento

/** @fn Display_MostrarBcd(uint32_t dato, uint8_t digitos, gpioConf_t *gpio_pinesConversor, gpioConf_t *gpio_pinesMapeo)
  * @brief Funcion que integra las funciones anteriores para tomar el numero en decimal lo convierte
  * @brief a bcd y luego se tiene que mostrar en los display de 7 segmento
  * @param [data]
  * @param [digitos]
  * @param [gpio_pinesConversor]
  * @param [gpio_pinesMapeo]
  * @return
  */
void Display_MostrarBcd(uint32_t dato, uint8_t digitos, gpioConf_t *gpio_pinesConversor, gpioConf_t *gpio_pinesMapeo){
	uint8_t digitosBcd[digitos]; // vector donde se guarda el dato concertido a bcd
	convertToBcdArray(dato, digitos, digitosBcd); //llamo a converir
	for(int i=0; i<3; i++){     // inicializo en cero los que muestra
		GPIOInit(gpio_pinesMapeo[i].pin, gpio_pinesMapeo[i].dir);
	}
	for(int i=0; i<3; i++){ // recorro el vector con los datos convertidos
		bcdToPin(digitosBcd[i], gpio_pinesConversor); // lo pongo en los pines y lo saco
		GPIOOn(gpio_pinesMapeo[i].pin);   // preendo y al sacarlo el display queda con el numero 
		GPIOOff(gpio_pinesMapeo[i].pin); // y este no se apago o cambia hasta qque se vuelva a subir otro numero
	}
}

void app_main(void){
	
	/* initializations */

	uint32_t dato=27;

	Display_MostrarBcd(dato, 3, mis_pines, pinSelector);
	
	
    while(1){
		/* main loop */

	}
    
	
	


	//printf("Hello world!\n");
}
/*==================[end of file]============================================*/