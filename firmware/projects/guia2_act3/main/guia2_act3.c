/** @mainpage guia2_ej1Distancia
 *
 * @section Actividad 3 - Proyecto: Medidor de distancia por ultrasonido c/interrupciones y puerto serie

Cree un nuevo proyecto en el que modifique la actividad del punto 2 agregando ahora el puerto serie. 
Envíe los datos de las mediciones para poder observarlos en un terminal en la PC, siguiendo el siguiente formato:
3 dígitos ascii + 1 carácter espacio + dos caracteres para la unidad (cm) + cambio de línea “ \r\n”
Además debe ser posible controlar la EDU-ESP de la siguiente manera:
Con las teclas “O” y “H”, replicar la funcionalidad de las teclas 1 y 2 de la EDU-ESP


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
/* agrego la parte de comunicacion puertos*/
#include "uart_mcu.h"


 /*==================[macros and definitions]=================================*/

#define CONFIG_BLINK_PERIOD_US 1000000
/**   
* 
*  @brief  activar  las dos variables se usa para poder llevar un estado de ver si las teclas estan o no activadas
*    @brief  hold   
*/

bool activar=false;
bool hold=false;

void Funcion_Leer_teclas(void);


/*==================[internal data definition]===============================*/
serial_config_t uart_pc_monitor = {			
    .port=UART_PC,	/*!< port */
	.baud_rate=115200,		/*!< baudrate (bits per second) */
	.func_p=Funcion_Leer_teclas,			/*!< Pointer to callback function to call when receiving data (= UART_NO_INT if not requiered)*/
	.param_p=NULL			/*!< Pointer to callback function parameters */
} ;

TaskHandle_t Dis_task_handle = NULL;
/*==================[internal functions declaration]=========================*/
/**
 * @brief Función invocada en la interrupción del timer A
 */
void FuncTimer_Distancia(void* param){
    vTaskNotifyGiveFromISR(Dis_task_handle, pdFALSE);    /* Envía una notificación a la tarea asociada al LED_1 */
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
     /* uso la funcion para convertir el numero medido a string y lo mando por la comsula*/
	

    UartSendString(UART_PC,( char*)UartItoa(distancia,10)); //forzar casteo

    UartSendString(UART_PC," cm \r\n" );
    //vTaskDelay(Delay1 / portTICK_PERIOD_MS);
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY); /* La tarea espera en este punto hasta recibir una notificación */
        
    }  
    
}

void Funcion_Tecla_1(){	
	activar=!activar;
	}

void Funcion_Tecla_2(){
	hold=!hold;
	}

void Funcion_Leer_teclas(){
	/// con el switch elegir y hacer lo que hace la tecla 1 y el otro caso lo que hace la tecla 2 
	uint8_t tecla_apretada; 
    UartReadByte(uart_pc_monitor.port, &tecla_apretada);
    
    switch(tecla_apretada){

        case'o': //O
                activar=!activar;
            break;
        case 'h': //H
                hold=!hold;
            break;
    }
}

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
/* agrego la parte de comunicacion puertos*/



UartInit(&uart_pc_monitor);


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
