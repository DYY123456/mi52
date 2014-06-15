/* FreeRTOS.org includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

/* The task functions. */
extern void vTask1( void *pvParameters );
extern void vTask2( void *pvParameters );


int main( void )
{
	/* Create one of the two tasks. Note that a real application should 
	check the return value of the xTaskCreate() call to ensure the task was
	created successfully. */
	xTaskCreate(
		vTask1, /* Pointer to the function that implements the task. */
		(const signed char *) "Task 1",/* Text name for the task. This is to facilitate
				debugging only. */
		256,/* Stack depth  */
		NULL,/* We are not using the task parameter. */
		1,/* This task will run at priority 1. */
		NULL  /* We are not going to use the task handle. */
		);
	/* Create the other task in exactly the same way and at the same 
	priority. */
	xTaskCreate( vTask2, (const signed char *)"Task 2", 256, NULL, 1, NULL );
	/* Start the scheduler so the tasks start executing. */
	vTaskStartScheduler();
	/* If all is well then main() will never reach here as the scheduler
	will now be running the tasks. If main() does reach here then it is 
	likely that there was insufficient heap memory available for the idle
	task to be created. Chapter 5 provides more information on memory 
	management. */
for( ;; );
}


