/** @mainpage Recuperatorio
 *
 * @section se describe un sistema de balanzas para camniones

*@section hardConn Conexión de Hardware
 *
 * - Canal ADC conectado al CH1.
     Canal ADC conectado al CH2.
 * - UART configurada a 115200 baud.
 *
 * @section hardConn Hardware Connection
 *
 * |    Peripheral  |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	PIN_13	 	| 	GPIO_3		| //ECHO
 *      PIN_12          GPIO_2        //TRIGGER
 *
 *  | 	PIN_8	 	| 	GPIO_1		| //ECHO
 *      PIN_12          GPIO_2        //TRIGGER
 
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 11/11/2024 | Document creation		                         |
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

//#include "lcditse0803.h"
//#include "switch.h"
/* agrego la parte de comunicacion puertos*/
#include "uart_mcu.h"
#include "analog_io_mcu.h" //conversorres

#include "gpio_mcu.h"

 /*==================[macros and definitions]=================================*/

#define CONFIG_BLINK_PERIOD_US 1000000 // para el tiempo daoo que la computadora es rapida orden de microsegundos 
#define CONFIG_Distancia_PERIOD_US 100000 // para el tiempo para tomar velocidades
/**
 * @brief constante Período del Timer A en microsegundos.
 */
#define CONFIG_PERIOD_ADC 5000 // 2ms período del Timer

#define CONFIG_Pesar_PERIOD_US 5000 // para el tiempo adquirir pesos
/**
 * @brief Variable para almacenar el valor leído en el  ADC.
 */
uint16_t dato1; // Valor leído del ADC
uint16_t dato2;
uint16_t peso;






//#define Delay1 1000000
/**
 * @brief Handle de la tarea encargada de la conversión ADC.
 */
TaskHandle_t conversion_ADC_task_handle = NULL; //Handle de la tarea ADC
/**   
* 
*  @brief  bool_velociad  las dos variables se usa para poder llevar un estado de ver si las teclas estan o no activadas
*    @brief  bool_peso   
*/

bool bool_velocidad=false;
bool bool_peso=false;

void Funcion_Leer_teclas(void);


/*==================[internal data definition]===============================*/
// estructura con los parametros para inicializar el puerto y sus interrupciones  
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
void FuncTimer_Velocidad(void* param){
    vTaskNotifyGiveFromISR(Dis_task_handle, pdFALSE);    /* Envía una notificación a la tarea asociada al LED_1 */
}
 /** @fn Leer_Distancia(void *pvParameter) 
  * @brief funcion para poder medir dados las condiciones y mostar en display
  * @param [*pvParameter]
  * @return [static]
  */
 
static void Leer_Velocidad_Task(void *pvParameter){
   uint16_t distancia1;
   uint16_t distancia2;
   uint16_t velocidad;
    while(true){
         distancia1=HcSr04ReadDistanceInCentimeters();
		 //vTaskDelay(Delay1 / portTICK_PERIOD_MS); // demoro un tiempo  de  1 seg para tomar otra muestra
		 ulTaskNotifyTake(pdTRUE, portMAX_DELAY); /* La tarea espera en este punto hasta recibir una notificación */
		 distancia2=HcSr04ReadDistanceInCentimeters();
		 velocidad=((distancia2-distancia1)/1)/100; // lo convierto en m/s
         bool_velocidad=true;
			if(velocidad > 8 ){
				LedOff(LED_1);
                LedOff(LED_2);
                LedOn(LED_3);
			}
			if(velocidad > 0 && velocidad < 8){
				LedOff(LED_1);
                LedOn(LED_2); // para prender solo los led es ese rango
                LedOff(LED_3);
			}
			
			if(velocidad <=0){
				LedOn(LED_1);
                LedOff(LED_2);
                LedOff(LED_3);
			}
         
    //if(hold== true)
	//LcdItsE0803Write(distancia);
     /* uso la funcion para convertir el numero medido a string y lo mando por la comsula*/
	
    UartSendString(UART_PC," Velocidad Maxima" );
    UartSendString(UART_PC,( char*)UartItoa(velocidad,10)); //forzar casteo
    UartSendString(UART_PC," m/s \r\n" );
    //vTaskDelay(Delay1 / portTICK_PERIOD_MS);
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY); /* La tarea espera en este punto hasta recibir una notificación */
        
    }  
    
}

/**
 * @brief Función conversión ADC de los valores que entran por el ch1 y los envia al puerto.
 * 
 * la conversión ADC se dispara con la interrupcion
 * interrupción del Timer A. Convierte los datos a formato ASCII y los envía por UART.
 * 
 * @param pParam Puntero a los parámetros de la tarea (no se utiliza).
 */
void conversionADC() {
    
        
        // obtiene valor analógico (ADC) del canal CH1 y lo guarda en la variable global valores.
		AnalogInputReadSingle(CH1, &dato1);
		AnalogInputReadSingle(CH2, &dato2);
        //dato1=dato1++;
		//dato2=dato2++;

		ulTaskNotifyTake(pdTRUE, portMAX_DELAY); // para la interrupcion
    
}
/**
 * @brief Función que se ejecuta en la interrupción del Timer A.
 * 
 * 
 * @param pParam Puntero a los parámetros de la función (no se utiliza).
 */
void funcion_TimerB(void *pParam) {
    vTaskNotifyGiveFromISR(conversion_ADC_task_handle, pdFALSE); // Notificación a la tarea ADC
}



static void Leer_Peso_Task(void *pvParameter){
   
    while(true){

             uint16_t aux1; // Valor leído del ADC
             uint16_t aux2;

               for(int i=0; i<50; i++){
				conversionADC( );
				aux1=aux1+dato1;
		        aux2=aux2+dato2;
			   }
			   aux1=(aux1/50)*(3.3/20000); //los convierto kilos
			   aux2=(aux2/50)*(3.3/20000);
              peso=aux1+aux2;

		UartSendString(UART_PC, "Peso:");	  
        UartSendString(UART_PC, (char*) UartItoa(peso, 10));
		 //fuerza la converison casteo
		UartSendString(UART_PC, " kg \r"); // para establecer el caracter fin 

	}
	dato1=0; //reset
	dato2=0;

}

static void Funcion_Leer_teclas_BarreraTask(void *pvParameter){
	/// con el switch elegir y hacer lo que hace la tecla 1 y el otro caso lo que hace la tecla 2 
	uint8_t tecla_apretada; 
    UartReadByte(uart_pc_monitor.port, &tecla_apretada);
    
    switch(tecla_apretada){
//GPIOInit(gpio[i].pin, gpio[i].dir)
        case'o': //O
                
				//gpioSetLevel(GPIO_1, activar);
                GPIOOn(GPIO_20);
				UartSendString(UART_PC, "Barrera abierta");

            break;
        case 'c': //c
                				//gpioSetLevel(GPIO_1, hold);
				GPIOOff(GPIO_20);
				UartSendString(UART_PC, "Barrera  cerrada");
            break;
    }
}

/*==================[external functions definition]==========================*/
void app_main(void){
GPIOInit(GPIO_20, GPIO_OUTPUT);

	// Configuración del canal ADC en modo de conversión única
    analog_input_config_t ADC_config1 = {
        .input = CH1,
        .mode = ADC_SINGLE
    };

    AnalogInputInit(&ADC_config1); //Inicializar el canal ADC
	analog_input_config_t ADC_config2 = {
        .input = CH2,
        .mode = ADC_SINGLE
    };

    AnalogInputInit(&ADC_config1); //Inicializar el canal ADC
	AnalogInputInit(&ADC_config2); //Inicializar el canal ADC
    AnalogOutputInit(); //Inicializa la salida analógica

	timer_config_t timerB = {
        .timer = TIMER_B,
        .period = CONFIG_PERIOD_ADC,
        .func_p = funcion_TimerB,
        .param_p = NULL
    };

    TimerInit(&timerB); // Inicialización del Timer B
	 // Configuración de la UART
    serial_config_t Uart = {
        .port = UART_PC,
        .baud_rate = 115200,
        .func_p = NULL,
        .param_p = NULL
    };

    UartInit(&Uart); ///< Inicializo la UART



    LedsInit(); //led

/* Inicialización de timers */
    timer_config_t timer_momento_velocidad = {
        .timer = TIMER_B,
        .period = CONFIG_Distancia_PERIOD_US,
        .func_p = FuncTimer_Velocidad,
        .param_p = NULL
    };
TimerInit(&timer_momento_velocidad);
/* agrego la parte de comunicacion puertos*/



UartInit(&uart_pc_monitor);


    //LcdItsE0803Init(); //display
    HcSr04Init(GPIO_3,GPIO_2); // de digo los puertos que usa la placa
     //SwitchesInit(); // teclado

	 //SwitchActivInt(SWITCH_1, &Funcion_Tecla_1,NULL); //interrupciones para el teclado
	//SwitchActivInt(SWITCH_2, &Funcion_Tecla_2, NULL);

	//SwitchActivInt(switch_t sw, void *ptr_int_func, void *args)
    /* Creación de tareas */
    xTaskCreate(&Leer_Velocidad_Task, "Velocidad", 512, NULL, 5, &Dis_task_handle);
	xTaskCreate(&Leer_Peso_Task, "Balanza", 512, NULL, 5, &Dis_task_handle);
    xTaskCreate(&Funcion_Leer_teclas_BarreraTask, "Barrera", 512, NULL, 5, NULL);


    //xTaskCreate(&LeerTeclado, "teclado", 512, NULL, 5, NULL);

     /* Inicialización del conteo de timers */
    TimerStart(timer_momento_velocidad.timer);
	TimerStart(timerB.timer); 
    
}
