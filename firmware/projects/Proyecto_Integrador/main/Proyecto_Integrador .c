/*! @mainpage Template
 *
 * @section genDesc General Description
 *
 * Este programa controla un sistema de temperatura y luz en un entorno UTI, donde
 * un ESP32-C6 mide temperatura y luminosidad, activando el aire acondicionado, calefacción o
 * el motor de cortina según umbrales configurados.
 *
 * <a href="https://drive.google.com/...">Operation Example</a>
 *
 * @section hardConn Hardware Connection
 *
 * | Peripheral  | ESP32        |
 * |-------------|--------------|
 * | Sensor Temp | GPIO_CH1     |
 * | Sensor Luz  | GPIO_CH2     |
 *
 * @section changelog Changelog
 *
 * |   Date     | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 12/09/2023 | Document creation                              |
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "analog_io_mcu.h"
#include "uart_mcu.h"
#include "timer_mcu.h"
#include "gpio_mcu.h"
#include "ble_mcu.h"
#include "led.h"
#include "string.h"
#include "rtc_mcu.h"
#include "dht11.h"

/*==================[macros and definitions]=================================*/
#define CONFIG_PERIOD_S_A 1000000  // Periodo de 1 segundos

/// Inicialización del Identificador de tareas
TaskHandle_t conversion_ADC_task_handle = NULL;
TaskHandle_t control_luz_task_handle = NULL;

/*==================[internal data definition]===============================*/

/// Variable global para encendido/apagado del sistema
bool system_on = false;

char mensaje[20];

/*==================[internal functions declaration]=========================*/

void enviarFechaHoraPorBluetooth() {
    rtc_t current_time;
    RtcRead(&current_time);

    char fecha_hora[30];
    sprintf(fecha_hora, "*D%02d/%02d/%04d %02d:%02d:%02d*\n", 
            current_time.mday, current_time.month, current_time.year,
            current_time.hour, current_time.min, current_time.sec);

    BleSendString(fecha_hora);
}

/**
 * @brief Envía la temperatura por Bluetooth en el formato *Txx*
 * @param temperatura La temperatura medida en grados Celsius.
 */
static void enviarTemperaturaPorBluetooth(uint16_t temperatura, bool estado_aire_acondicionado) {
    strcpy(mensaje, "");
    sprintf(mensaje, "*T%u*", temperatura); // Formato *Txx*
    BleSendString(mensaje);
    if (estado_aire_acondicionado) {
        strcpy(mensaje, "");
        sprintf(mensaje, "*S ENCENDIDO\n*"); // Formato *M*
        BleSendString(mensaje);
    }
    else {
        strcpy(mensaje, "");
        sprintf(mensaje, "*S APAGADO\n*"); // Formato *M*
        BleSendString(mensaje);
    } 
}

/**
 * @brief Envía la temperatura por Bluetooth en el formato *Txx*
 * @param temperatura La temperatura medida en grados Celsius.
 */
static void enviarLuminosidadPorBluetooth(uint16_t luminosidad, bool estado_motor) {
    strcpy(mensaje, "");
    sprintf(mensaje, "*L %u\n*", luminosidad); // Formato *Txx*
    BleSendString(mensaje);
    if (estado_motor) {
        strcpy(mensaje, "");
        sprintf(mensaje, "*M ABIERTA\n*"); // Formato *M*
        BleSendString(mensaje);
    }
    else {
        strcpy(mensaje, "");
        sprintf(mensaje, "*M CERRADA\n*"); // Formato *M*
        BleSendString(mensaje);
    }   
}

/**
 * @brief Tarea que realiza la conversión ADC de temperatura, verifica umbrales y envía la temperatura
 */
static void conversion_ADC_task(void *pParam) {
    
    //uint16_t temperature_samples[10];
    uint16_t umbral_temp_alto = 24;   // Umbral alto para encender aire acondicionado
    //uint16_t umbral_calefaccion = 18; // Umbral bajo para encender calefacción
    //uint16_t apagar_aire_acond = 21;  // Umbral para apagar aire acondicionado o calefacción
    bool state_air = false;           // Estado del aire acondicionado o calefacción

    enviarFechaHoraPorBluetooth();

    while (true) {
        
        float humedad = 0;
        float temperatura = 0; 
        
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);  // Espera notificación del timer

        if (system_on) {  // Verifica si el sistema está encendido
            
            /*Realiza 10 muestras del sensor de temperatura
            for (int i = 0; i < 10; i++) {
                AnalogInputReadSingle(CH1, &temperature_samples[i]);
            }*/

            dht11Read( &humedad, &temperatura );
            
            enviarTemperaturaPorBluetooth(temperatura, state_air);

            // Verifica y ejecuta las acciones según los umbrales en grados Celsius
            if (temperatura > umbral_temp_alto) {
                printf("Temperatura alta: Encendiendo aire acondicionado\n");
                //printf("Temperatura: %d\n", temperatura);
                printf("Temperatura: %.2f\n", temperatura);
                state_air = true;
                enviarTemperaturaPorBluetooth(temperatura, state_air); // Envía la temperatura por Bluetooth con el estado del aire acondicionadoLuminosidadPorBluetooth(avg_light, state_motor);
                LedOn(LED_2); // Simula encender aire acondicionado
                
            }else {
                // Apaga aire acondicionado o calefacción si el promedio está en el rango de 21-24 grados
                printf("Temperatura baja: Apagando aire acondicionado\n");
                //printf("Temperatura: %d\n", temperatura);
                printf("Temperatura: %.2f\n", temperatura);
                state_air = false;
                LedOff(LED_2);
            }
        }
    }
}

/**
 * @brief Tarea que controla la luminosidad y acciona el motor de cortina
 */
static void control_luz_task(void *pParam) {
    
    uint16_t light_samples[10];
    uint16_t umbral_luz_encendido = 1000;  // Umbral de luz para activar cortina
    bool state_motor = false;             // Estado del motor de cortina

    while (true) {
        
        vTaskDelay(pdMS_TO_TICKS(1000));  // Espera de 1 segundo

        if (system_on) {  // Verifica si el sistema está encendido
            
            // Realiza 10 muestras del sensor de luz
            for (int i = 0; i < 10; i++) {
                AnalogInputReadSingle(CH2, &light_samples[i]);
            }

            // Promedia las muestras de luz
            uint32_t light_sum = 0;
            for (int i = 0; i < 10; i++) {
                light_sum += light_samples[i];
            }
            uint16_t avg_light = light_sum / 10;

            // Acciona el motor de cortina según el umbral de luz
            if (avg_light > umbral_luz_encendido) {
                printf("Alta luminosidad: Activando motor de cortina\n");
                printf("Luminosidad: %d\n", avg_light);
                state_motor = true;
                enviarLuminosidadPorBluetooth(avg_light, state_motor);
                LedOn(LED_3);  // Simula activación de cortina
            } else {
                printf("Luminosidad baja: Cortina desactivada\n");
                printf("Luminosidad: %d\n", avg_light);
                state_motor = false;
                enviarLuminosidadPorBluetooth(avg_light, state_motor);
                LedOff(LED_3);  // Simula desactivación de cortina
            }
        }
    }
}

/**
 * @brief función que lee datos recibidos por Bluetooth para activar/desactivar el sistema
 */
static void leerDatosBle(uint8_t *data, uint8_t length) {
    if (data[0] == 'E') {
        system_on = true;
        printf("Sistema encendido\n");
        LedOn(LED_1); // Encender LED 1
    } else if (data[0] == 'A') {
        system_on = false;
        printf("Sistema apagado\n");
        LedOff(LED_1); // Apagar LED 1
    }
}

/**
 * @brief Función de callback para el Timer A
 */
void funcionTimerA(void *pParam) {
    vTaskNotifyGiveFromISR(conversion_ADC_task_handle, pdFALSE);  // Notificación a la tarea de conversión ADC
}

/*==================[external functions definition]==========================*/
void app_main(void){
    
    // Inicializa los LEDs para
    // 1: Sistema encendido
    // 2: Aire acondicionado encendido
    // 3: Motor cortina encendido

    LedsInit();  
    
    /// Configuración de Bluetooth
    ble_config_t ble_configuration = {
        "Sistema_UTI_EPROG",
        leerDatosBle
    };
    BleInit(&ble_configuration);

    /*Configuración de entrada del sensor de temperatura
    analog_input_config_t Analog_config_temperature = {
        .input = CH1,
        .mode = ADC_SINGLE
    };
    AnalogInputInit(&Analog_config_temperature);*/

    //dht11Init(GPIO_1);
       dht11Init(GPIO_1);

    // Configuración de entrada del sensor de luz
    analog_input_config_t Analog_config_light = {
        .input = CH2,
        .mode = ADC_SINGLE
    };
    AnalogInputInit(&Analog_config_light);
    
    // Configuración e inicialización del timerA
    timer_config_t timerA = {
        .timer = TIMER_A,
        .period = CONFIG_PERIOD_S_A,
        .func_p = funcionTimerA,
        .param_p = NULL
    };
    TimerInit(&timerA);

    // Creación de las tareas
    xTaskCreate(&conversion_ADC_task, "conversion_ADC_task", 2048, NULL, 4, &conversion_ADC_task_handle);
    xTaskCreate(&control_luz_task, "control_luz_task", 2048, NULL, 4, &control_luz_task_handle);

    TimerStart(timerA.timer);
    
}
/*==================[end of file]============================================*/