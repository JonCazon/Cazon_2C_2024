/*!
 * @mainpage Guía 2 - Actividad 4 - Segunda parte
 * @section Enunciado
 *Prueba del osciloscopio:
Convierta una señal digital de un ECG (provista por la cátedra) en una señal analógica 
y visualice esta señal utilizando el osciloscopio que acaba de implementar. Se sugiere 
utilizar el potenciómetro para conectar la salida del DAC a la entrada CH1 del AD

 * 
 *
 * @section hardConn Conexión de Hardware
 *
 * - Canal ADC conectado al CH1.
 * - UART configurada a 115200 baud.
 * 
 *  |    Peripheral  |   ESP32   	   |
 * |:--------------:|:--------------   
 * | PIN_8     	|Potenciometro GPIO1/CH1  |	
 * |     VCC        |   5V              |
 * |     GND        |   GND             |
 * @section changelog Historial de Cambios
 *
 * |   Fecha      | Descripción                                    |
 * |:------------:|:-----------------------------------------------|
 * | 14/10/2024   | Creación del documento                         |
 * | 14/10/2024   | Agregaron comentarios y documentación          |
 *
 * @author
 * Jonatan Cazon
 */

/*==================[inclusions]=============================================*/

#include <stdio.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "analog_io_mcu.h"
#include "uart_mcu.h"
#include "timer_mcu.h"
#include "gpio_mcu.h"

/*==================[macros and definitions]=================================*/

#define CONFIG_PERIOD_A_DAC 4000 // 4mse seg
/**
 * @brief constante Período del Timer B en microsegundos.
 */
#define CONFIG_PERIOD_B_ADC 2000 // 2mseg - 500Hz
/**
 * @brief Variable para almacenar el valor leído en el  ADC.
 */
uint16_t valores; 
/**
 * @brief contador para llegar el mumero de valores.
 */
uint16_t contador_posicion =0;

//#define BUFFER_SIZE 231


/*==================[internal data definition]===============================*/
// /* /**
 //* @brief Handle de la tarea encargada de la conversión ADC.
 //*/
//Ta1skHandle_t main_task_handle = NULL; 


/**
 * @brief Vector ecg donde esta almacenado la senial ECG.
 */
const char code_ECG[]  = {
17,17,17,17,17,17,17,17,17,17,17,18,18,18,17,17,17,17,17,17,17,18,18,18,18,18,18,18,17,17,16,16,16,16,17,17,18,18,18,17,17,17,17,
18,18,19,21,22,24,25,26,27,28,29,31,32,33,34,34,35,37,38,37,34,29,24,19,15,14,15,16,17,17,17,16,15,14,13,13,13,13,13,13,13,12,12,
10,6,2,3,15,43,88,145,199,237,252,242,211,167,117,70,35,16,14,22,32,38,37,32,27,24,24,26,27,28,28,27,28,28,30,31,31,31,32,33,34,36,
38,39,40,41,42,43,45,47,49,51,53,55,57,60,62,65,68,71,75,79,83,87,92,97,101,106,111,116,121,125,129,133,136,138,139,140,140,139,137,
133,129,123,117,109,101,92,84,77,70,64,58,52,47,42,39,36,34,31,30,28,27,26,25,25,25,25,25,25,25,25,24,24,24,24,25,25,25,25,25,25,25,
24,24,24,24,24,24,24,24,23,23,22,22,21,21,21,20,20,20,20,20,19,19,18,18,18,19,19,19,19,18,17,17,18,18,18,18,18,18,18,18,17,17,17,17,
17,17,17

} ;

/*==================[internal data definition]===============================*/
/**
 * @brief Handle de la tarea encargada de la conversión.
 */
TaskHandle_t conversion_DAC_task_handle = NULL;

TaskHandle_t conversion_ADC_task_handle = NULL;

/**
 * @brief Función conversión DAC lee los valores del vector y los convierte.
 * 
 * la conversión DAC se dispara con la interrupcion A
 * 
 * 
 * @param pParam Puntero a los parámetros de la tarea (no se utiliza).
 */
/*==================[internal functions declaration]=========================*/
static void conversionDAC(void *pParam){
	while(true){
		
		if(contador_posicion<sizeof(code_ECG)){
			//ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
			AnalogOutputWrite(code_ECG[contador_posicion]);
			contador_posicion++;
			ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		}
		else{
			contador_posicion = 0;
		}
	}
}
/**
 * @brief Función conversión ADC de los valores que entran por el ch1 y los envia al puerto.
 * 
 * la conversión ADC se dispara con la interrupcion
 * interrupción del Timer B. Convierte los datos a formato ASCII y los envía por UART.
 * 
 * @param pParam Puntero a los parámetros de la tarea (no se utiliza).
 */
static void conversionADC(void *pParam){

	while(true){
		//ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		AnalogInputReadSingle(CH1, &valores);
		// la funcion Itoa convierte de int --> ascii
		UartSendString(UART_PC,(char*) UartItoa(valores, 10));
		UartSendString(UART_PC, "\r");
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
	}

}
/**
 * @brief Función que se ejecuta en la interrupción del Timer A.
 * 
 * 
 * @param pParam Puntero a los parámetros de la función (no se utiliza).
 */
void funcionTimerA(void *pParam){
	//agrego notificaciones
	vTaskNotifyGiveFromISR(conversion_DAC_task_handle, pdFALSE);

}
/**
 * @brief Función que se ejecuta en la interrupción del Timer B.
 * 
 * 
 * @param pParam Puntero a los parámetros de la función (no se utiliza).
 */
void funcionTimerB(void *pParam){
	//agrego notificaciones
	vTaskNotifyGiveFromISR(conversion_ADC_task_handle, pdFALSE);

}


/*==================[external functions definition]==========================*/

/**
 * @brief Función Main donde se unifica para resolver el planteo del problema.
 
 
 */
void app_main(void){
	// inicializo el struct para configurar qué puerto voy a utilizar y en qué modo
	analog_input_config_t Analog_config = {
		.input = CH1,
		.mode = ADC_SINGLE
	};

	AnalogInputInit(&Analog_config); // inicializo puertos
	AnalogOutputInit();

	// Configuro la estructura del timerA
	timer_config_t timerA = {
		.timer = TIMER_A,
		.period = CONFIG_PERIOD_A_DAC,
		.func_p = funcionTimerA,
		.param_p = NULL
	};
	// inicialización del timerA
	TimerInit(&timerA);

	// Configuro la estructura del timerB
	timer_config_t timerB = {
		.timer = TIMER_B,
		.period = CONFIG_PERIOD_B_ADC,
		.func_p = funcionTimerB,
		.param_p = NULL
	};
	 /// inicialización del timer B
	TimerInit(&timerB);

	
	/// configuración de la UART
	serial_config_t Uart = {
		.port = UART_PC,
		.baud_rate = 115200,
		.func_p = NULL,
		.param_p = NULL
	};

	/// inicializo la UART
	UartInit(&Uart);

	/// creación de las tareas que quiero ejecutar 
	xTaskCreate(&conversionDAC, "convertir_ECG_a_Analogico", 2048, NULL, 5, &conversion_DAC_task_handle);
	xTaskCreate(&conversionADC, "digitalizar_senial_del_CH1", 2048, NULL, 5, &conversion_ADC_task_handle);
	
	// 
	TimerStart(timerA.timer);
	TimerStart(timerB.timer);
}

/*==================[end of file]============================================*/
