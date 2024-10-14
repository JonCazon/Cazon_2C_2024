/*!
 * @mainpage Guía 2 - Actividad 4 - conversion Analogico a digital 
 * @section genDesc Descripción General
 *
 * problema a resolver: diseniar un programa dispara la conversión A/D a través de una interrupción de timer
 * cada 2 ms, lo que genera una frecuencia de muestreo de 500 Hz.
 * 
 * Los datos de la conversión son transmitidos por UART en formato ASCII y visualizados
 * mediante un graficador serie de código abierto llamado "Serial Oscilloscope".
 * 
 * El formato de envío de datos es "11\r". Por ejemplo, si hubiera más de un canal,
 * sería "11,22,33\r", donde 11, 22 y 33 corresponden a los canales 1, 2 y 3 respectivamente.
 *
 * @section hardConn Conexión de Hardware
 *
 * - Canal ADC conectado al CH1.
 * - UART configurada a 115200 baud.
 *
 * @section changelog Historial de Cambios
 *
 * |   Fecha      | Descripción                                    |
 * |:------------:|:-----------------------------------------------|
 * | xx/xx/2024   | Creación del documento                         |
 * | xx/xx/2024   | Agregaron comentarios y documentación          |
 *
 * @author
 * Jontan Cazon
 */

/*==================[inclusions]=============================================*/

#include <stdio.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "analog_io_mcu.h" //conversorres
#include "uart_mcu.h" // puertos
#include "timer_mcu.h" // timmers
#include "gpio_mcu.h" //  pines 

/*==================[macros and definitions]=================================*/

/**
 * @brief constante Período del Timer A en microsegundos.
 */
#define CONFIG_PERIOD_ADC 2000 // 2ms período del Timer

/**
 * @brief Variable para almacenar el valor leído en el  ADC.
 */
uint16_t valores; // Valor leído del ADC

/*==================[internal data definition]===============================*/

/**
 * @brief Handle de la tarea encargada de la conversión ADC.
 */
TaskHandle_t conversion_ADC_task_handle = NULL; //Handle de la tarea ADC

/*==================[internal functions declaration]=========================*/

/**
 * @brief Función conversión ADC de los valores que entran por el ch1 y los envia al puerto.
 * 
 * la conversión ADC se dispara con la interrupcion
 * interrupción del Timer A. Convierte los datos a formato ASCII y los envía por UART.
 * 
 * @param pParam Puntero a los parámetros de la tarea (no se utiliza).
 */
static void conversionADC(void *pParam) {
    while (true) {
        
        // obtiene valor analógico (ADC) del canal CH1 y lo guarda en la variable global valores.
		AnalogInputReadSingle(CH1, &valores);
       
        UartSendString(UART_PC, (char*) UartItoa(valores, 10));
		// fuerza la converison casteo
		UartSendString(UART_PC, "\r"); // para establecer el caracter fin 
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY); // para la interrupcion
    }
}

/**
 * @brief Función que se ejecuta en la interrupción del Timer A.
 * 
 * 
 * @param pParam Puntero a los parámetros de la función (no se utiliza).
 */
void funcion_TimerA(void *pParam) {
    vTaskNotifyGiveFromISR(conversion_ADC_task_handle, pdFALSE); // Notificación a la tarea ADC
}


/*==================[external functions definition]==========================*/

/**
 * @brief Función Main donde se unifica para resolver el planteo del problema.
 
 
 */
void app_main(void) {
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

    // Creación de la tarea de conversión ADC
    xTaskCreate(&conversionADC, "digitalizar_senial_CH1", 512, NULL, 5, &conversion_ADC_task_handle);

    TimerStart(timerA.timer); // Inicio del Timer A
}

/*==================[end of file]============================================*/



