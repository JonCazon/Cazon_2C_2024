/** @mainpage guia2_ej1Distancia
 *
 * @section Actividad 2 - Proyecto: Medidor de distancia por ultrasonido c/interrupciones
*Cree un nuevo proyecto en el que modifique la actividad del punto 1 de manera de utilizar 
* para el control de las teclas y el control de tiempos (Timers). 
*
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

#include "timer_mcu.h"
#include "led.h"
#include "hc_sr04.h" 

#include "lcditse0803.h"
#include "switch.h"

 /*==================[macros and definitions]=================================*/
/** 
 * @def CONFIG_BLINK_PERIOD_US
 *@brief  variable para dar una el retardo de tiempo 
*/
#define CONFIG_BLINK_PERIOD_US 1000000
/**   
* @def activar
* @brief  activar  las dos variables se usa para poder llevar un estado de ver si las teclas
* @brief  estan o no activadas
* @def hold
* @brief  hold para que se mantenga la medicion en el display
*/
bool activar=false;
bool hold=false;


/*==================[internal data definition]===============================*/

TaskHandle_t Dis_task_handle = NULL;
/*==================[internal functions declaration]=========================*/
/** @fn FuncTimer_Distancia(void* param)
 *@brief Función invocada en la interrupción del timer A
 */
void FuncTimer_Distancia(void* param){
    vTaskNotifyGiveFromISR(Dis_task_handle, pdFALSE);  /* Envía una notificación a la tarea asociada al LED_1 */
}
 /** @fn Leer_Distancia(void *pvParameter) 
  * @brief funcion para poder medir dados las condiciones y mostar en display
  * @param [*pvParameter]
  * @return [static]
  */
static void Leer_Distancia_Task(void *pvParameter){
   uint16_t distancia;
    while(true){
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
    //vTaskDelay(Delay1 / portTICK_PERIOD_MS);
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY); /* La tarea espera en este punto hasta recibir una notificación */
        
    }  
    
}
/** @fn Funcion_Tecla_1() 
  * @brief funcion llamada en la interrupcion del teclado para activar la cuenta o no
   */
void Funcion_Tecla_1(){	
	activar=!activar;
	}
/** @fn Funcion_Tecla_2() 
  * @brief funcion llamada en la interrupcion del teclado para mantener ek numero en dispaly  o no
   */
void Funcion_Tecla_2(){
	hold=!hold;
	}

//memo aqui usando las interrupciones del teclado, configuro para cuando se aprepietan 
// hacen lo mismo que el punto anterior

/*==================[external functions definition]==========================*/
void app_main(void){
    LedsInit(); //led

/* Inicialización de timers */
    timer_config_t timer_momento_1 = {
        .timer = TIMER_A,
        .period = CONFIG_BLINK_PERIOD_US,
        .func_p = FuncTimer_Distancia,
        .param_p = NULL
    };
TimerInit(&timer_momento_1);
    LcdItsE0803Init(); //display
    HcSr04Init(GPIO_3,GPIO_2); // de digo los puertos que usa la placa
     SwitchesInit(); // teclado

	 SwitchActivInt(SWITCH_1, &Funcion_Tecla_1,NULL); //interrupciones para el teclado
	SwitchActivInt(SWITCH_2, &Funcion_Tecla_2, NULL);

	//SwitchActivInt(switch_t sw, void *ptr_int_func, void *args)
    /* Creación de tareas */
    xTaskCreate(&Leer_Distancia_Task, "Distancia", 512, NULL, 5, &Dis_task_handle);
    //xTaskCreate(&LeerTeclado, "teclado", 512, NULL, 5, NULL);
     /* Inicialización del conteo de timers */
    TimerStart(timer_momento_1.timer);
    
}
