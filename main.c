/* FreeRTOS.org includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "pio.h"
#include "uart0.h"
#include "general.h"
#include "config.h"

/* The task functions. */
extern void vTask1( void *pvParameters );
extern void vTask2( void *pvParameters );
extern void vTask3( void *pvParameters );

/* the stdout semaphore */
xSemaphoreHandle stdio_mutex;
xSemaphoreHandle timer_event;


static void vT2_ISR( void ) __attribute__ ((naked));
void vT2_init(tU32 ms );

static void vT2_ISR( void ) 
{
static portBASE_TYPE xHigherPriorityTaskWoken;

 /* save task context */
 portSAVE_CONTEXT();
 
 /* test the interrupt, it schould be 0x1 (MR0 interrupt) */
 if (T2IR == 0x1) {
	/* reset the timer 1 interrupt */
	T2IR  = 0x01;
	/* signal the binary semaphore */
	xHigherPriorityTaskWoken = pdFALSE;
	xSemaphoreGiveFromISR(timer_event, &xHigherPriorityTaskWoken);
	/* if a higher priority task can run, schedule */
	if( xHigherPriorityTaskWoken == pdTRUE ){
			portYIELD_FROM_ISR(  );
	}	
 }
 /* acknowledge the VIC */
 VICVectAddr = 1;		

 /* restore task context and resume */	
 portRESTORE_CONTEXT();
}

void vT2_init(tU32 time_slice)
{
 vSemaphoreCreateBinary( timer_event );
 if( timer_event != NULL )
    {
        // The semaphore was created successfully.
        // The semaphore is created unsignaled

	//taskDISABLE_INTERRUPTS();
	/* init timer 2 */
	PCONP |= 1<<22;		/* power on for T2 */
	PCLKSEL1 &= ~(3 << 12); /* clock frequency selection : Fcclk/4 for T2 */
	T2TCR = 0x02;   		/* disable and reset timer*/
	T2PR  = 17 ;   			/* set prescaler to n-1 */
	T2MR0 = (time_slice * (Fcclk / (1000 * 4 * (T2PR+1))))-1 ;
	T2IR  = 0xff;   		/*reset all interrrupt flags*/
	T2MCR = 0x03;   		/*reset timer and interrupt on match*/
	T2TCR = 0x01;   		/*start timer*/
	
	/*init vic for T0 */
	VICIntSelect &= ~(1<<26) ;		/* IRQ contribution */
	VICVectAddr26 = (unsigned long) vT2_ISR ;/* ISR address */
	VICVectPriority26 = 10;			/* priority */
	VICIntEnable |= 1<<26 ;			/* enable TC2 IRQ */
	taskENABLE_INTERRUPTS();
    }

}





int main( void )
{
		led_joystick_init();
		uart0Init();
		vT2_init(500); //500ms
		stdio_mutex = xSemaphoreCreateMutex();

	/* Create one of the two tasks. Note that a real application should 
	check the return value of the xTaskCreate() call to ensure the task was
	created successfully. */
	xTaskCreate(
		vTask1, /* Pointer to the function that implements the task. */
		(const signed char *) "Task 1",/* Text name for the task. This is to facilitate
				debugging only. */
		256,/* Stack depth  */
		NULL,/* We are not using the task parameter. */
		5,/* This task will run at priority 5. */
		NULL  /* We are not going to use the task handle. */
		);
	/* Create the other task in exactly the same way and at the same 
	priority. */
	xTaskCreate( vTask2, (const signed char *)"Task 2", 256, NULL, 5, NULL );
	/* Create the other task in exactly the same way and a higher priority */
	xTaskCreate( vTask3, (const signed char *)"Task 2", 256, NULL, 3, NULL );
	/* Start the scheduler so the tasks start executing. */
	vTaskStartScheduler();
	/* If all is well then main() will never reach here as the scheduler
	will now be running the tasks. If main() does reach here then it is 
	likely that there was insufficient heap memory available for the idle
	task to be created. Chapter 5 provides more information on memory 
	management. */
for( ;; );
}


