/** @mainpage guia2_ej1Distancia
 *
 * @section Diseñar el firmware modelando con un diagrama de flujo de manera que cumpla con las siguientes funcionalidades:

* Mostrar distancia medida utilizando los leds de la siguiente manera:

* Si la distancia es menor a 10 cm, apagar todos los LEDs.
* Si la distancia está entre 10 y 20 cm, encender el LED_1.
* Si la distancia está entre 20 y 30 cm, encender el LED_2 y LED_1.
* Si la distancia es mayor a 30 cm, encender el LED_3, LED_2 y LED_1.

* Mostrar el valor de distancia en cm utilizando el display LCD.
* Usar TEC1 para activar y detener la medición.
* Usar TEC2 para mantener el resultado (“HOLD”).
* Refresco de medición: 1 s
*  Se deberá conectar a la EDU-ESP un sensor de ultrasonido HC-SR04 y una pantalla LCD y utilizando los drivers
*  provistos por la cátedra implementar la aplicación correspondiente. Se debe subir al repositorio el código.
*  Se debe incluir en la documentación, realizada con doxygen, el diagrama de flujo.
*
 *
 * @section hardConn Hardware Connection
 *
 * |    Peripheral  |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	PIN_13	 	| 	GPIO_3		| //ECHO
 *      PIN_12          GPIO_2        //TRIGGER
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 11/08/2024 | Document creation		                         |
 *
 * @author Jonatan Cazon 
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led.h"
#include "hc_sr04.h" 

#include "lcditse0803.h"
#include "switch.h"

/*==================[macros and definitions]=================================*/
/**   
*  @brief Delay1   tiempo para poder mediendo 
*	@brief  Delay2	 tiempo que se usa para poder leer el teclado, asi evitar error en las lecturas
*  @brief  activar  las dos variables se usa para poder llevar un estado de ver si las teclas estan o no activadas
    @brief  hold   
*/
#define Delay1 1000 // 
#define Delay2 100 // 

bool activar=false;
bool hold=false;

/*==================[internal data definition]===============================*/


/*==================[internal functions declaration]=========================*/
 /** @fn Leer_Distancia(void *pvParameter) 
  * @brief funcion para poder medir dados las condiciones y mostar en display
  * @param [*pvParameter]
  * @return [static]
  */
static void Leer_Distancia(void *pvParameter){
   uint16_t distancia;
    while(true){
        if(activar== true){
         distancia=HcSr04ReadDistanceInCentimeters();
         
			if(distancia > 0 && distancia < 10){
				LedOff(LED_1);
                LedOff(LED_2);
                LedOff(LED_3);
			}
			if(distancia >= 10 && distancia < 20){
				LedOn(LED_1);
                LedOff(LED_2); // para prender solo los led es ese rango
                LedOff(LED_3);
			}
			if(distancia >= 20 && distancia < 30){
				LedOn(LED_1);
                LedOn(LED_2);
                LedOff(LED_3);// condicion que se suma,para al volver el solo queden los led en ese
			}                 // en ese rango
			if(distancia >= 30){
				LedOn(LED_1);
                LedOn(LED_2);
                LedOn(LED_3);
			}
         
    if(hold== true)
	LcdItsE0803Write(distancia);
    vTaskDelay(Delay1 / portTICK_PERIOD_MS);
        
    }}
    
    
}
/** @fn LeerTeclado(void *pvParameter) 
  * @brief funcion lee el estado del teclado
  * @param [*pvParameter]
  * @return [static]
  */
static void LeerTeclado(void *pvParameter){
    while(true){
      int8_t teclas=SwitchesRead();
              
	if(teclas == SWITCH_1)
	{
		activar=!activar;
         vTaskDelay(Delay2 / portTICK_PERIOD_MS);
	}

	if(teclas ==SWITCH_2){
		hold=!hold;
        vTaskDelay(Delay2/ portTICK_PERIOD_MS);
	}
        
        vTaskDelay(Delay2 / portTICK_PERIOD_MS);

    }
}


/*==================[external functions definition]==========================*/
void app_main(void){
    LedsInit(); //led
    LcdItsE0803Init(); //display
    HcSr04Init(GPIO_3,GPIO_2); // de digo los puertos que usa la placa
     SwitchesInit(); // teclado

    xTaskCreate(&Leer_Distancia, "Distancia", 512, NULL, 5, NULL);
    xTaskCreate(&LeerTeclado, "teclado", 512, NULL, 5, NULL);
    
}
