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
/* CONSIGNA:Escriba una función que reciba un dato de 32 bits,  la cantidad de dígitos de salida y un puntero a un arreglo donde se almacene los n dígitos. La función deberá convertir el dato recibido a BCD, guardando cada uno de los dígitos de salida en el arreglo pasado como puntero.
*/
/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
/*==================[macros and definitions]=================================*/

/*==================[internal data definition]===============================*/

/*==================[internal functions declaration]=========================*/





int8_t convertToBcdArray (uint32_t data, uint8_t digits, uint8_t * bcd_number)
{
	int i=0;
	for (i=0; i<digits; i++){
		bcd_number[(digits-1-i)]=data%10;
		data=data/10;
	}
return 0;
}
/*==================[external functions definition]==========================*/
void app_main(void){

	uint32_t numero = 123;
	uint8_t digito = 3;
	uint8_t vect[3] = {0};
	convertToBcdArray (numero, digito, vect);
	//printf("Hello world!\n");
}
/*==================[end of file]============================================*/