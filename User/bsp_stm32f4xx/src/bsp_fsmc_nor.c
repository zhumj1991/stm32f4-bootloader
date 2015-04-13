/******************** (C) COPYRIGHT 2008 STMicroelectronics ********************
* File Name          : bsp_fsmc_nor.c
* Author             : MCD Application Team
* Version            : V2.0.1
* Date               : 06/13/2008
* Description        : This file provides a set of functions needed to drive the
*                      M29W128FL, M29W128GL and S29GL128P NOR memories mounted
*                      on STM3210E-EVAL board.
********************************************************************************
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/
/* Includes ------------------------------------------------------------------*/
#include "bsp_fsmc_nor.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define Bank1_NOR2_ADDR       ((u32)0x64000000)
//#define Bank1_NOR1_ADDR       ((u32)0x60000000)

/* Delay definition */   
#define BlockErase_Timeout    ((u32)0x00A00000)
#define ChipErase_Timeout     ((u32)0x30000000) 
#define Program_Timeout       ((u32)0x00001400)

/* Private macro -------------------------------------------------------------*/
#define ADDR_SHIFT(A) (Bank1_NOR2_ADDR + (2 * (A)))
//#define ADDR_SHIFT(A) (Bank1_NOR1_ADDR + (2 * (A)))
#define NOR_WRITE(Address, Data)  (*(vu16 *)(Address) = (Data))

/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/*******************************************************************************
* Function Name  : FSMC_NOR_Init
* Description    : Configures the FSMC and GPIOs to interface with the NOR memory.
*                  This function must be called before any write/read operation
*                  on the NOR.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
/* NOR 的 GPIO ：
	PD0/FSMC_D2
	PD1/FSMC_D3
	PD4/FSMC_NOE
	PD5/FSMC_NWE
	PD8/FSMC_D13
	PD9/FSMC_D14
	PD10/FSMC_D15
	PD11/FSMC_A16
	PD12/FSMC_A17
	PD13/FSMC_A18
	PD14/FSMC_D0
	PD15/FSMC_D1

	PE0/FSMC_NBL0
	PE1/FSMC_NBL1
	PE3/FSMC_A19
	PE4/FSMC_A20	-- 参与片选的译码
	PE5/FSMC_A21	-- 参与片选的译码
	PE7/FSMC_D4
	PE8/FSMC_D5
	PE9/FSMC_D6
	PE10/FSMC_D7
	PE11/FSMC_D8
	PE12/FSMC_D9
	PE13/FSMC_D10
	PE14/FSMC_D11
	PE15/FSMC_D12

	PF0/FSMC_A0
	PF1/FSMC_A1
	PF2/FSMC_A2
	PF3/FSMC_A3
	PF4/FSMC_A4
	PF5/FSMC_A5
	PF12/FSMC_A6
	PF13/FSMC_A7
	PF14/FSMC_A8
	PF15/FSMC_A9

	PG0/FSMC_A10
	PG1/FSMC_A11
	PG2/FSMC_A12
	PG3/FSMC_A13
	PG4/FSMC_A14
	PG5/FSMC_A15
	PG10/FSMC_NE3	--- 片选主信号
*/
/*******************************************************************************
*????:FSMC_NOR_Init
*??:??FSMC?GPIO????NOR???
*????????????/?????NOR?
*??:?
*??:?
*??:?
************************************************** *****************************/
void FSMC_NOR_Init(void)
{
  FSMC_NORSRAMInitTypeDef  FSMC_NORSRAMInitStructure;
  FSMC_NORSRAMTimingInitTypeDef  p;

  GPIO_InitTypeDef GPIO_InitStructure;
/* FSMC GPIO */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_GPIOE | RCC_AHB1Periph_GPIOF | RCC_AHB1Periph_GPIOG, ENABLE);
	
/* GPIOD configuration */
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource0, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource1, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource4, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource5, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource8, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource9, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource10, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource11, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource12, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource13, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource14, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource15, GPIO_AF_FSMC);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0  | GPIO_Pin_1  | GPIO_Pin_4  | GPIO_Pin_5  |
	                    GPIO_Pin_8  | GPIO_Pin_9  | GPIO_Pin_10 | GPIO_Pin_11 |
	                    GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;

	GPIO_Init(GPIOD, &GPIO_InitStructure);


	/* GPIOE configuration */
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource0 , GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource1 , GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource3 , GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource4 , GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource5 , GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource7 , GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource8 , GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource9 , GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource10 , GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource11 , GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource12 , GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource13 , GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource14 , GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource15 , GPIO_AF_FSMC);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0  | GPIO_Pin_1  | GPIO_Pin_3 |
	                    GPIO_Pin_4  | GPIO_Pin_5  | GPIO_Pin_7 |
	                    GPIO_Pin_8  | GPIO_Pin_9  | GPIO_Pin_10 | GPIO_Pin_11|
	                    GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;

	GPIO_Init(GPIOE, &GPIO_InitStructure);


	/* GPIOF configuration */
	GPIO_PinAFConfig(GPIOF, GPIO_PinSource0 , GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOF, GPIO_PinSource1 , GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOF, GPIO_PinSource2 , GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOF, GPIO_PinSource3 , GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOF, GPIO_PinSource4 , GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOF, GPIO_PinSource5 , GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOF, GPIO_PinSource12 , GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOF, GPIO_PinSource13 , GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOF, GPIO_PinSource14 , GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOF, GPIO_PinSource15 , GPIO_AF_FSMC);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0  | GPIO_Pin_1  | GPIO_Pin_2  | GPIO_Pin_3  |
	                    GPIO_Pin_4  | GPIO_Pin_5  | GPIO_Pin_12 | GPIO_Pin_13 |
	                    GPIO_Pin_14 | GPIO_Pin_15;

	GPIO_Init(GPIOF, &GPIO_InitStructure);


	/* GPIOG configuration */
	GPIO_PinAFConfig(GPIOG, GPIO_PinSource0 , GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOG, GPIO_PinSource1 , GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOG, GPIO_PinSource2 , GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOG, GPIO_PinSource3 , GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOG, GPIO_PinSource4 , GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOG, GPIO_PinSource5 , GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOG, GPIO_PinSource10 , GPIO_AF_FSMC);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0  | GPIO_Pin_1  | GPIO_Pin_2  | GPIO_Pin_3 |
	                    GPIO_Pin_4  | GPIO_Pin_5  | GPIO_Pin_10;

	GPIO_Init(GPIOG, &GPIO_InitStructure);



  p.FSMC_AddressSetupTime = 0x05;
  p.FSMC_AddressHoldTime = 0x00;
  p.FSMC_DataSetupTime = 0x07;
  p.FSMC_BusTurnAroundDuration = 0x00;
  p.FSMC_CLKDivision = 0x00;
  p.FSMC_DataLatency = 0x00;
  p.FSMC_AccessMode = FSMC_AccessMode_B;

  FSMC_NORSRAMInitStructure.FSMC_Bank = FSMC_Bank1_NORSRAM2;
  FSMC_NORSRAMInitStructure.FSMC_DataAddressMux = FSMC_DataAddressMux_Disable;
  FSMC_NORSRAMInitStructure.FSMC_MemoryType = FSMC_MemoryType_NOR;
  FSMC_NORSRAMInitStructure.FSMC_MemoryDataWidth = FSMC_MemoryDataWidth_16b;
  FSMC_NORSRAMInitStructure.FSMC_BurstAccessMode = FSMC_BurstAccessMode_Disable;
  FSMC_NORSRAMInitStructure.FSMC_WaitSignalPolarity = FSMC_WaitSignalPolarity_Low;
  FSMC_NORSRAMInitStructure.FSMC_WrapMode = FSMC_WrapMode_Disable;
  FSMC_NORSRAMInitStructure.FSMC_WaitSignalActive = FSMC_WaitSignalActive_BeforeWaitState;
  FSMC_NORSRAMInitStructure.FSMC_WriteOperation = FSMC_WriteOperation_Enable;
  FSMC_NORSRAMInitStructure.FSMC_WaitSignal = FSMC_WaitSignal_Disable;
  FSMC_NORSRAMInitStructure.FSMC_ExtendedMode = FSMC_ExtendedMode_Disable;
  FSMC_NORSRAMInitStructure.FSMC_WriteBurst = FSMC_WriteBurst_Disable;
  FSMC_NORSRAMInitStructure.FSMC_ReadWriteTimingStruct = &p;
  FSMC_NORSRAMInitStructure.FSMC_WriteTimingStruct = &p;

  FSMC_NORSRAMInit(&FSMC_NORSRAMInitStructure);

// Enable FSMC Bank1_NOR Bank
// FSMC_NORSRAMCmd(FSMC_Bank1_NORSRAM2, ENABLE);
  FSMC_NORSRAMCmd(FSMC_Bank1_NORSRAM2, ENABLE);

}

/******************************************************************************
* Function Name  : FSMC_NOR_ReadID
* Description    : Reads NOR memory's Manufacturer and Device Code.
* Input          : - NOR_ID: pointer to a NOR_IDTypeDef structure which will hold
*                    the Manufacturer and Device Code.
* Output         : None
* Return         : None
*******************************************************************************/
/******************************************************************************
*????:FSMC_NOR_ReadID
*??:??NOR????????????
*??: -NOR_ID:????NOR_IDTypeDef??,????????????
*??:?
*??:?
************************************************** *****************************/
void FSMC_NOR_ReadID(NOR_IDTypeDef* NOR_ID)
{
  NOR_WRITE(ADDR_SHIFT(0x05555), 0x00AA);
  NOR_WRITE(ADDR_SHIFT(0x02AAA), 0x0055);
  NOR_WRITE(ADDR_SHIFT(0x05555), 0x0090);

  NOR_ID->Manufacturer_Code = *(vu16 *) ADDR_SHIFT(0x0000);
  NOR_ID->Device_Code1 = *(vu16 *) ADDR_SHIFT(0x0001);

  NOR_ID->Device_Code2 = *(vu16 *) ADDR_SHIFT(0x000E);
  NOR_ID->Device_Code3 = *(vu16 *) ADDR_SHIFT(0x000F);

}

/*******************************************************************************
* Function Name  : FSMC_NOR_EraseBlock
* Description    : Erases the specified Nor memory block.
* Input          : - BlockAddr: address of the block to erase.
* Output         : None
* Return         : NOR_Status:The returned value can be: NOR_SUCCESS, NOR_ERROR
*                  or NOR_TIMEOUT
*******************************************************************************/
/*******************************************************************************
*????:FSMC_NOR_EraseBlock
*??:?????NOR?????
*??: -BlockAddr:????????
*??:?
*??:NOR_Status:???????:NOR??,NOR???NOR??
************************************************** *****************************/
NOR_Status FSMC_NOR_EraseBlock(u32 BlockAddr)
{
  NOR_WRITE(ADDR_SHIFT(0x05555), 0x00AA);
  NOR_WRITE(ADDR_SHIFT(0x02AAA), 0x0055);
  NOR_WRITE(ADDR_SHIFT(0x05555), 0x0080);
  NOR_WRITE(ADDR_SHIFT(0x05555), 0x00AA);
  NOR_WRITE(ADDR_SHIFT(0x02AAA), 0x0055);
//  NOR_WRITE((Bank1_NOR2_ADDR + BlockAddr), 0x30);
  NOR_WRITE((Bank1_NOR2_ADDR + BlockAddr), 0x0030);//此处必须为0x0030否则会擦不掉数据
//  NOR_WRITE((Bank1_NOR1_ADDR + BlockAddr), 0x0030);//此处必须为0x0030否则会擦不掉数据

  return (FSMC_NOR_GetStatus(BlockErase_Timeout));
}

/*******************************************************************************
* Function Name  : FSMC_NOR_EraseChip
* Description    : Erases the entire chip.
* Input          : None                      
* Output         : None
* Return         : NOR_Status:The returned value can be: NOR_SUCCESS, NOR_ERROR
*                  or NOR_TIMEOUT
*******************************************************************************/
/*******************************************************************************
*????:FSMC_NOR_EraseChip
*??:???????
*??:?
*??:?
*??:NOR_Status:???????:NOR??,NOR???NOR??
************************************************** *****************************/
NOR_Status FSMC_NOR_EraseChip(void)
{
  NOR_WRITE(ADDR_SHIFT(0x05555), 0x00AA);
  NOR_WRITE(ADDR_SHIFT(0x02AAA), 0x0055);
  NOR_WRITE(ADDR_SHIFT(0x05555), 0x0080);
  NOR_WRITE(ADDR_SHIFT(0x05555), 0x00AA);
  NOR_WRITE(ADDR_SHIFT(0x02AAA), 0x0055);
  NOR_WRITE(ADDR_SHIFT(0x05555), 0x0010);

  return (FSMC_NOR_GetStatus(ChipErase_Timeout));
}

/******************************************************************************
* Function Name  : FSMC_NOR_WriteHalfWord
* Description    : Writes a half-word to the NOR memory. 
* Input          : - WriteAddr : NOR memory internal address to write to.
*                  - Data : Data to write. 
* Output         : None
* Return         : NOR_Status:The returned value can be: NOR_SUCCESS, NOR_ERROR
*                  or NOR_TIMEOUT
*******************************************************************************/
/******************************************************************************
*????:FSMC_NOR_WriteHalfWord
*??:???NOR?????
*??: -WriteAddr:NOR??????????
* -??:??????
*??:?
*??:NOR_Status:???????:NOR??,NOR???NOR??
************************************************** *****************************/
NOR_Status FSMC_NOR_WriteHalfWord(u32 WriteAddr, u16 Data)
{
  NOR_WRITE(ADDR_SHIFT(0x05555), 0x00AA);
  NOR_WRITE(ADDR_SHIFT(0x02AAA), 0x0055);
  NOR_WRITE(ADDR_SHIFT(0x05555), 0x00A0);
  NOR_WRITE((Bank1_NOR2_ADDR + WriteAddr), Data);
//  NOR_WRITE((Bank1_NOR1_ADDR + WriteAddr), Data);

  return (FSMC_NOR_GetStatus(Program_Timeout));
}

/*******************************************************************************
* Function Name  : FSMC_NOR_WriteBuffer
* Description    : Writes a half-word buffer to the FSMC NOR memory. 
* Input          : - pBuffer : pointer to buffer. 
*                  - WriteAddr : NOR memory internal address from which the data 
*                    will be written.
*                  - NumHalfwordToWrite : number of Half words to write. 
* Output         : None
* Return         : NOR_Status:The returned value can be: NOR_SUCCESS, NOR_ERROR
*                  or NOR_TIMEOUT
*******************************************************************************/
/*******************************************************************************
*????            :FSMC_NOR_WriteBuffer
*??                :   ???FSMC?NOR?????????
*??                : -pbuffer           :??????
*                       -WriteAddr         :NOR???????????????
*                       -NumHalfwordToWrite:?????
*??                :?
*??                :NOR_Status:???????:NOR??,NOR???NOR??
************************************************** *****************************/
NOR_Status FSMC_NOR_WriteBuffer(u16* pBuffer, u32 WriteAddr, u32 NumHalfwordToWrite)
{
  NOR_Status status = NOR_ONGOING; 

  do
  {
    /* Transfer data to the memory */
    status = FSMC_NOR_WriteHalfWord(WriteAddr, *pBuffer++);
    WriteAddr = WriteAddr + 2;
    NumHalfwordToWrite--;
  }
  while((status == NOR_SUCCESS) && (NumHalfwordToWrite != 0));
  
  return (status); 
}

/*******************************************************************************
* Function Name  : FSMC_NOR_ProgramBuffer
* Description    : Writes a half-word buffer to the FSMC NOR memory. This function 
*                  must be used only with S29GL128P NOR memory.
* Input          : - pBuffer : pointer to buffer. 
*                  - WriteAddr: NOR memory internal address from which the data 
*                    will be written.
*                  - NumHalfwordToWrite: number of Half words to write.
*                    The maximum allowed value is 32 Half words (64 bytes).
* Output         : None
* Return         : NOR_Status:The returned value can be: NOR_SUCCESS, NOR_ERROR
*                  or NOR_TIMEOUT
*******************************************************************************/ 
/*******************************************************************************
*????            :FSMC_NOR_ProgramBuffer
*??                :???FSMC?NOR??????????????????S29GL128P NOR????
*??                : -pbuffer?:??????
* -WriteAddr         :NOR???????????????
* -NumHalfwordToWrite:?????
*                      ??????32???(64??)?
*??:?
*??                :NOR_Status:???????:NOR??,NOR???NOR??
************************************************** *****************************/
NOR_Status FSMC_NOR_ProgramBuffer(u16* pBuffer, u32 WriteAddr, u32 NumHalfwordToWrite)
{       
  u32 lastloadedaddress = 0x00;
  u32 currentaddress = 0x00;
  u32 endaddress = 0x00;

  /* Initialize variables */
  currentaddress = WriteAddr;
  endaddress = WriteAddr + NumHalfwordToWrite - 1;
  lastloadedaddress = WriteAddr;

  /* Issue unlock command sequence */
  NOR_WRITE(ADDR_SHIFT(0x005555), 0x00AA);

  NOR_WRITE(ADDR_SHIFT(0x02AAA), 0x0055);  

  /* Write Write Buffer Load Command */
  NOR_WRITE(ADDR_SHIFT(WriteAddr), 0x0025);
  NOR_WRITE(ADDR_SHIFT(WriteAddr), (NumHalfwordToWrite - 1));

  /* Load Data into NOR Buffer */
  while(currentaddress <= endaddress)
  {
    /* Store last loaded address & data value (for polling) */
    lastloadedaddress = currentaddress;

    NOR_WRITE(ADDR_SHIFT(currentaddress), *pBuffer++);
    currentaddress += 1; 
  }

  NOR_WRITE(ADDR_SHIFT(lastloadedaddress), 0x29);
  
  return(FSMC_NOR_GetStatus(Program_Timeout));
}

/******************************************************************************
* Function Name  : FSMC_NOR_ReadHalfWord
* Description    : Reads a half-word from the NOR memory. 
* Input          : - ReadAddr : NOR memory internal address to read from.
* Output         : None
* Return         : Half-word read from the NOR memory
*******************************************************************************/
/******************************************************************************
*????:FSMC_NOR_ReadHalfWord
*??:???NOR??????
*??: -ReadAddr:NOR???????????
*??:?
*??:???NOR????
************************************************** *****************************/
u16 FSMC_NOR_ReadHalfWord(u32 ReadAddr)
{
  NOR_WRITE(ADDR_SHIFT(0x005555), 0x00AA); 
  NOR_WRITE(ADDR_SHIFT(0x002AAA), 0x0055);  
  NOR_WRITE((Bank1_NOR2_ADDR + ReadAddr), 0x00F0 );
//  NOR_WRITE((Bank1_NOR1_ADDR + ReadAddr), 0x00F0 );

  return (*(vu16 *)((Bank1_NOR2_ADDR + ReadAddr)));
//  return (*(vu16 *)((Bank1_NOR1_ADDR + ReadAddr)));
}

/*******************************************************************************
* Function Name  : FSMC_NOR_ReadBuffer
* Description    : Reads a block of data from the FSMC NOR memory.
* Input          : - pBuffer : pointer to the buffer that receives the data read 
*                    from the NOR memory.
*                  - ReadAddr : NOR memory internal address to read from.
*                  - NumHalfwordToRead : number of Half word to read.
* Output         : None
* Return         : None
*******************************************************************************/
/*******************************************************************************
*????           :FSMC_NOR_ReadBuffer
*??               :?????FSMC?NOR????????
*??               : -pbuffer?:??????,????NOR????????
* -ReadAddr         :NOR???????????
* -NumHalfwordToRead:??????
*??:?
*??:?
************************************************** *****************************/
void FSMC_NOR_ReadBuffer(u16* pBuffer, u32 ReadAddr, u32 NumHalfwordToRead)
{
  NOR_WRITE(ADDR_SHIFT(0x05555), 0x00AA);
  NOR_WRITE(ADDR_SHIFT(0x02AAA), 0x0055);
  NOR_WRITE((Bank1_NOR2_ADDR + ReadAddr), 0x00F0);
//  NOR_WRITE((Bank1_NOR1_ADDR + ReadAddr), 0x00F0);

  for(; NumHalfwordToRead != 0x00; NumHalfwordToRead--) /* while there is data to read */
  {
    /* Read a Halfword from the NOR */
    *pBuffer++ = *(vu16 *)((Bank1_NOR2_ADDR + ReadAddr));
//    *pBuffer++ = *(vu16 *)((Bank1_NOR1_ADDR + ReadAddr));
    ReadAddr = ReadAddr + 2; 
  }  
}

/******************************************************************************
* Function Name  : FSMC_NOR_ReturnToReadMode
* Description    : Returns the NOR memory to Read mode.
* Input          : None
* Output         : None
* Return         : NOR_SUCCESS
*******************************************************************************/ 
/******************************************************************************
*????:FSMC_NOR_ReturnToReadMode
*??:???NOR???????
*??:?
*??:?
*??:NOR_??
************************************************** *****************************/
NOR_Status FSMC_NOR_ReturnToReadMode(void)
{
  NOR_WRITE(Bank1_NOR2_ADDR, 0x00F0);
//  NOR_WRITE(Bank1_NOR1_ADDR, 0x00F0);
  return (NOR_SUCCESS);
}

/******************************************************************************
* Function Name  : FSMC_NOR_Reset
* Description    : Returns the NOR memory to Read mode and resets the errors in
*                  the NOR memory Status Register.
* Input          : None
* Output         : None
* Return         : NOR_SUCCESS
*******************************************************************************/
/******************************************************************************
*????:FSMC_NOR_Reset
*??:??NOR???????,?NOR?????????????
*??:?
*??:?
*??:NOR_??
************************************************** *****************************/
NOR_Status FSMC_NOR_Reset(void)
{
  NOR_WRITE(ADDR_SHIFT(0x005555), 0x00AA); 
  NOR_WRITE(ADDR_SHIFT(0x002AAA), 0x0055); 
  NOR_WRITE(Bank1_NOR2_ADDR, 0x00F0); 
//  NOR_WRITE(Bank1_NOR1_ADDR, 0x00F0); 

  return (NOR_SUCCESS);
}

/******************************************************************************
* Function Name  : FSMC_NOR_GetStatus
* Description    : Returns the NOR operation status.
* Input          : - Timeout: NOR progamming Timeout
* Output         : None
* Return         : NOR_Status:The returned value can be: NOR_SUCCESS, NOR_ERROR
*                  or NOR_TIMEOUT
*******************************************************************************/
/******************************************************************************
*????:FSMC_NOR_GetStatus
*??    :???NOR??????
*??    : -??:???NOR progamming
*??    :?
*??    :NOR_Status:???????:NOR??,NOR???NOR??
************************************************** *****************************/
NOR_Status FSMC_NOR_GetStatus(u32 Timeout)
{ 
  u16 val1 = 0x00, val2 = 0x00;
  NOR_Status status = NOR_ONGOING; 
  u32 timeout = Timeout;

  /* Poll on NOR memory Ready/Busy signal ------------------------------------*/
  while((GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_6) != RESET) && (timeout > 0)) 
  {
    timeout--;
  }

  timeout = Timeout;
  
  while((GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_6) == RESET) && (timeout > 0))   
  {
    timeout--;
  }
  
  /* Get the NOR memory operation status -------------------------------------*/
  while((Timeout != 0x00) && (status != NOR_SUCCESS))
  {
    Timeout--;

          /* Read DQ6 and DQ5 */
    val1 = *(vu16 *)(Bank1_NOR2_ADDR);
//    val1 = *(vu16 *)(Bank1_NOR1_ADDR);
    val2 = *(vu16 *)(Bank1_NOR2_ADDR);
//    val2 = *(vu16 *)(Bank1_NOR1_ADDR);

    /* If DQ6 did not toggle between the two reads then return NOR_Success */
    if((val1 & 0x0040) == (val2 & 0x0040)) 
    {
      return NOR_SUCCESS;
    }

    if((val1 & 0x0020) != 0x0020)
    {
      status = NOR_ONGOING;
    }

    val1 = *(vu16 *)(Bank1_NOR2_ADDR);
//    val1 = *(vu16 *)(Bank1_NOR1_ADDR);
    val2 = *(vu16 *)(Bank1_NOR2_ADDR);
//    val2 = *(vu16 *)(Bank1_NOR1_ADDR);
    
    if((val1 & 0x0040) == (val2 & 0x0040)) 
    {
      return NOR_SUCCESS;
    }
    else if((val1 & 0x0020) == 0x0020)
    {
      return NOR_ERROR;
    }
  }

  if(Timeout == 0x00)
  {
    status = NOR_TIMEOUT;
  } 

  /* Return the operation status */
  return (status);
}

/******************* (C) COPYRIGHT 2008 STMicroelectronics *****END OF FILE****/
