/** @mainpage guia2_ej1Distancia
 *
 * @section momento de examen
*
*
 *
 * @section hardConn Hardware Connection
 *
 * |    Peripheral  |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	PIN_13	 	| 	GPIO_3		| //ECHO
 *      PIN_12          GPIO_2        //TRIGGER
 *      PIN_11          GPIO_11        //pin para el buzzer
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 4/11/2024 | Document creation		                         |
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

//#include "lcditse0803.h"
//#include "switch.h"
#include "buzzer.h"

#include "ble_mcu.h"

/*==================[macros and definitions]=================================*/
/**
 * @brief constante Período del Timer A en microsegundos.
 */
#define CONFIG_PERIOD_ADC 10 // 2ms período del Timer

/**
 * @brief Variable para almacenar el valor leído en el  ADC.
 */
uint16_t valore1; // Valor leído del ADC
uint16_t valore2;
uint16_t valore3;
/** 
 * @def CONFIG_BLINK_PERIOD_US
 *@brief  variable para dar una el retardo de tiempo 
*/
#define CONFIG_BLINK_PERIOD_US 1000000/2
/**   
*  @brief Delay1   tiempo para poder mediendo 
*	@brief  Delay2	 tiempo que se usa para poder leer el teclado, asi evitar error en las lecturas
*  @brief  activar  las dos variables se usa para poder llevar un estado de ver si las teclas estan o no activadas
    @brief  hold   
*/
//#define Delay1 1000 // 
//#define Delay2 100 // 

//bool activar=false;
//bool hold=false;

/*==================[internal data definition]===============================*/
TaskHandle_t Dis_task_handle = NULL;

/**
 * @brief Handle de la tarea encargada de la conversión ADC.
 */
TaskHandle_t conversion_ADC_task_handle = NULL; //Handle de la tarea ADC


/*==================[internal functions declaration]=========================*/


static void conversionADC(void *pParam) {
    while (true) {
        
        // obtiene valor analógico (ADC) del canal CH1 y lo guarda en la variable global valores.
		AnalogInputReadSingle(CH1, &valore1);
       AnalogInputReadSingle(CH2, &valore2);
       AnalogInputReadSingle(CH3, &valore3);
       uint16_t sumado= valore1+valore2+valore3;  
         if(sumado>1.2)(
        UartSendString(UART_PC, (char*) UartItoa( sumado, 10));
		// fuerza la converison casteo
		UartSendString(UART_PC, "\r"); ) // para establecer el caracter fin 

		ulTaskNotifyTake(pdTRUE, portMAX_DELAY); // para la interrupcion
    }
}


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
/**
 * @brief Envía la temperatura por Bluetooth en el formato *Txx*
 * @param distancia La temperatura medida en grados Celsius.
 */
 static void enviarDistanciaPorBluetooth(uint16_t distancia, bool estado_cerca_lejos) {
    strcpy(mensaje, "");
    sprintf(mensaje, "*D%u*", distancia); // Formato *Txx*
    BleSendString(mensaje);
    if (estado_cerca_lejos) {
        strcpy(mensaje, "");
        sprintf(mensaje, "*S Peligro, vehículo cerca\n*"); // Formato *M*
        BleSendString(mensaje);
    }
    else {
        strcpy(mensaje, "");
        sprintf(mensaje, "*S Precaución, vehículo cerca\n*"); // Formato *M*
        BleSendString(mensaje);
    } 
}
static void Leer_Distancia(void *pvParameter){
   uint16_t distancia;
    while(true){
         distancia=HcSr04ReadDistanceInCentimeters();
         
			if(distancia >5  ){
				LedOn(LED_1);
                LedOff(LED_2); // para prender solo los led es ese rango
                LedOff(LED_3);
                BuzzerOff(void);
			}
			if(distancia < 5 && distancia > 3){
				LedOn(LED_1);
                LedOn(LED_2);
                LedOff(LED_3);// condicion que se suma,para al volver el solo queden los led en ese
                 BuzzerSetFrec(0.016666);
                 BuzzerOn(void);
                 enviarDistanciaPorBluetooth(distancia, true)
                 //BuzzerOff(void);
			}                 // en ese rango
			if(distancia < 3){
				LedOn(LED_1);
                LedOn(LED_2);
                LedOn(LED_3);
                BuzzerSetFrec(0.03333);
                BuzzerOn(void);
                enviarDistanciaPorBluetooth(distancia, false)
                //BuzzerOff(void)
                
			}
         
    if(hold== true)
	//LcdItsE0803Write(distancia);
    //vTaskDelay(Delay1 / portTICK_PERIOD_MS);
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY); /* La tarea espera en este punto hasta recibir una notificación */    
     BuzzerOff(void);
    }

    
    
}


void funcion_TimerA(void *pParam) {
    vTaskNotifyGiveFromISR(conversion_ADC_task_handle, pdFALSE); // Notificación a la tarea ADC
}
/*
/** @fn LeerTeclado(void *pvParameter) 
  * @brief funcion lee el estado del teclado
  * @param [*pvParameter]
  * @return [static]
  */
/*static void LeerTeclado(void *pvParameter){
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
*/

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

    //LcdItsE0803Init(); //display
    HcSr04Init(GPIO_3,GPIO_2); // de digo los puertos que usa la placa
     //SwitchesInit(); // teclado

 // Configuración del canal ADC en modo de conversión única
    analog_input_config_t ADC_config = {
        .input = CH1,
        .mode = ADC_SINGLE
    };

    AnalogInputInit(&ADC_config); //Inicializar el canal ADC
    AnalogOutputInit(); //Inicializa la salida analógica

    // Configuración del Timer A
    timer_config_t timerA = {
        .timer = TIMER_A,
        .period = CONFIG_PERIOD_ADC,
        .func_p = funcion_TimerA,
        .param_p = NULL
    };

    TimerInit(&timerA); // Inicialización del Timer A

    // Configuración de la UART
    serial_config_t Uart = {
        .port = UART_PC,
        .baud_rate = 115200,
        .func_p = NULL,
        .param_p = NULL
    };

    UartInit(&Uart); ///< Inicializo la UART

    xTaskCreate(&Leer_Distancia, "Distancia", 512, NULL, 5, NULL);
    xTaskCreate(&conversionADC, "giroscopio", 512, NULL, 5, NULL);
	 /* Inicialización del conteo de timers */
    TimerStart(timer_momento_1.timer);

    BuzzerInit(GPIO_11);
    
}
