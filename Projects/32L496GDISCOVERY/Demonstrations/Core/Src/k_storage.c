/**
  ******************************************************************************
  * @file    k_storage.c
  * @author  MCD Application Team
  * @brief   This file provides the kernel storage functions
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2017 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "k_storage.h"

/** @addtogroup CORE
  * @{
  */

/** @defgroup KERNEL_STORAGE
  * @brief Kernel storage routines
  * @{
  */

/* External variables --------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
FATFS mSDDISK_FatFs;         /* File system object for MSD disk logical drive */
char mSDDISK_Drive[4];       /* MSD Host logical drive number */
osMessageQId StorageEvent;
DIR dir;

static uint32_t StorageStatus[NUM_DISK_UNITS];
/* Private function prototypes -----------------------------------------------*/
static void StorageThread(void const * argument);

/* Private functions ---------------------------------------------------------*/


/**
  * @brief  Storage drives initialization
  * @param  None 
  * @retval None
  */
void k_StorageInit(void)
{  
   /* Force DMA mode */
   //BSP_SD_ForceDmaMode();
  
   /* Link the micro SD disk I/O driver */
   if(FATFS_LinkDriver(&SD_Driver, mSDDISK_Drive) != 0)
   {
     /* FatFs Format Error */
     Error_Handler();
   }  
   else
   {
     /* Create USB background task */
     osThreadDef(STORAGE_Thread, StorageThread, osPriorityLow, 0, 128);
     osThreadCreate (osThread(STORAGE_Thread), NULL);
    
     /* Create Storage Message Queue */
     osMessageQDef(osqueue, 10, uint16_t);
     StorageEvent = osMessageCreate (osMessageQ(osqueue), NULL);
    
     /* Enable SD Interrupt mode */
     BSP_SD_Init();
     BSP_SD_ITConfig();

     if(BSP_SD_IsDetected())
     {
       osMessagePut ( StorageEvent, MSDDISK_CONNECTION_EVENT, 0);
     }  
   }
}

/**
  * @brief  Storage Thread
  * @param  argument: pointer that is passed to the thread function as start argument.
  * @retval None
  */
static void StorageThread(void const * argument)
{
  osEvent event;
  
  for( ;; )
  {
    event = osMessageGet( StorageEvent, osWaitForever );
    
    if( event.status == osEventMessage )
    {
      switch(event.value.v)
      {        
      case MSDDISK_CONNECTION_EVENT:
        f_mount(&mSDDISK_FatFs, mSDDISK_Drive, 0);
        StorageStatus[MSD_DISK_UNIT] = 1;
        break;
        
      case MSDDISK_DISCONNECTION_EVENT:
        f_mount(0, mSDDISK_Drive, 0);
        StorageStatus[MSD_DISK_UNIT] = 0;        
        break;         
      }
    }
  }
}

/**
  * @brief  Storage get status
  * @param  unit: logical storage unit index.
  * @retval int
  */
uint8_t k_StorageGetStatus (uint8_t unit)
{  
  return StorageStatus[unit];
}

/**
  * @}
  */

/**
  * @}
  */

