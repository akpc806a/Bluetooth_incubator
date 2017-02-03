/**
  ******************************************************************************
  * File Name          : main.c
  * Description        : Main program body
  ******************************************************************************
  *
  * COPYRIGHT(c) 2016 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "stm32f0xx_hal.h"

/* USER CODE BEGIN Includes */
#include "modbus.h"
#include <math.h>

/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

TIM_HandleTypeDef htim14;
TIM_HandleTypeDef htim15;
TIM_HandleTypeDef htim16;
TIM_HandleTypeDef htim17;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */
/*
// time stamps for period measurements
uint16_t iCrossPos_PosTimestamp = 0;
uint16_t iCrossPos_NegTimestamp = 0;
uint16_t iCrossNeg_PosTimestamp = 0;
uint16_t iCrossNeg_NegTimestamp = 0;
uint16_t iCrossNeg_NegTimestampPrev = 0;

// calculated timing parameters
uint16_t iPeriod;
uint16_t iPausePos;
uint16_t iPauseNeg;
uint16_t iPause;
*/
#define round(x) (int)((x) + 0.5)

// Modbus data
int16_t iTemperature; // temperature: 100 -- 1 oC, 32768 -- 327.68 oC
int16_t iHumidity; // humidity: 100 -- 1%, 10000 -- 100%
uint16_t iTimer; // upcounting timer with precision of 1 sec
uint16_t iOutputState; // bits[7:0] -- heater output, bits[8] -- humidity on, bits[9] -- timer on, bits[15] -- manual output

// outputs of internal regulators
uint8_t iHeaterOn = 0;
uint8_t bHumidityOn = 0;
uint8_t bTimerOn = 0;
// mapping of internal regulators to actual output (0 -- bHeaterOn, 1 -- bHumidityOn, 2 -- bTimerOn)
uint16_t iCH0_Mapping = 0; 
uint16_t iCH1_Mapping = 1;
uint16_t iCH2_Mapping = 2;

// timer values (in sec)
uint16_t iTimerOnInterval = 5;
uint32_t iTimerOffInterval = 60;

#define IsManualControl() ((iOutputState & 0x800) != 0)

#define GetHeaterState() (iOutputState & 0x7FF)
#define SetHeaterState(x) iOutputState = (iOutputState & ~0x7FF) | (((int)(x)) & 0x7FF)
#define iTriacDuty GetHeaterState() // duty cycle of triac power regulator -- is a part of iOutputState vector


#define IsHumidityOn() ((iOutputState & 0x1000) != 0)
#define SetHumidityOn() iOutputState |= 0x1000
#define SetHumidityOff() iOutputState &= ~0x1000

#define IsTimerOn() ((iOutputState & 0x2000) != 0)
#define SetTimerOn() iOutputState |= 0x2000
#define SetTimerOff() iOutputState &= ~0x2000

#define IsHeaterFault() ((iOutputState & 0x4000) != 0)
#define SetHeaterFaultOn() iOutputState |= 0x4000
#define SetHeaterFaultOff() iOutputState &= ~0x4000

#define IsHumidityFault() ((iOutputState & 0x8000) != 0)
#define SetHumidityFaultOn() iOutputState |= 0x8000
#define SetHumidityFaultOff() iOutputState &= ~0x8000

// temperature controller types
#define CTRL_TYPE_ONOFF 0
#define CTRL_TYPE_PI 1
#define CTRL_TYPE_HYBRID 2
uint16_t iHeaterCtrlType = CTRL_TYPE_PI; // temperature controller type

// hybrid controller stuff

typedef enum
{
    HYBRID_STATE_IDLE = 0,
    HYBRID_STATE_ONOFF,
    HYBRID_STATE_RATIO,
    HYBRID_STATE_WAIT_TO_CROSS,
    HYBRID_STATE_PI
} __attribute__ ((__packed__)) T_HybridCtrl_State;

T_HybridCtrl_State bHybridCtrlState = HYBRID_STATE_IDLE;
int iHybridCtrlPulseCount = 0;
#define HYBRID_CTRL_PULSE_COUNT 5 // pulse count before taking on/off ratio
int iHybridCtrlOnCounter = 0;
int iHybridCtrlOffCounter = 0;
uint8_t bHybridCtrlHeaterOn = 0;

uint16_t iHumidityOutputInv = 0; // 1 -- inversion of output for humidity

GPIO_InitTypeDef GPIO_InitStruct;
/* USER CODE END PV */


/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_TIM17_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_TIM14_Init(void);
static void MX_TIM16_Init(void);
void ProgramFlash(void);
void ReadFlash(void);
//static void MX_TIM15_Init(void);

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/
static void Error_Handler(void);

#ifdef __GNUC__
  /* With GCC/RAISONANCE, small printf (option LD Linker->Libraries->Small printf
     set to 'Yes') calls __io_putchar() */
  #define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
  #define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */

/**
  * @brief  Retargets the C library printf function to the USART.
  * @param  None
  * @retval None
  */
PUTCHAR_PROTOTYPE
{
  /* Place your implementation of fputc here */
  /* e.g. write a character to the EVAL_COM1 and Loop until the end of transmission */
  HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 0xFFFF); 

  return ch;
}



#define CH2_ON() HAL_GPIO_WritePin(CH2_GPIO_Port, CH2_Pin, GPIO_PIN_SET);
#define CH2_OFF() HAL_GPIO_WritePin(CH2_GPIO_Port, CH2_Pin, GPIO_PIN_RESET);
#define CH1_ON() HAL_GPIO_WritePin(CH1_GPIO_Port, CH1_Pin, GPIO_PIN_SET);
#define CH1_OFF() HAL_GPIO_WritePin(CH1_GPIO_Port, CH1_Pin, GPIO_PIN_RESET);
#define CH0_ON() HAL_GPIO_WritePin(CH0_GPIO_Port, CH0_Pin, GPIO_PIN_SET);
#define CH0_OFF() HAL_GPIO_WritePin(CH0_GPIO_Port, CH0_Pin, GPIO_PIN_RESET);

void HeaterOn()
{
  if (iCH0_Mapping == 0) CH0_ON();
  if (iCH1_Mapping == 0) CH1_ON();
  if (iCH2_Mapping == 0) CH2_ON();
}

void HeaterOff()
{
  if (iCH0_Mapping == 0) CH0_OFF();
  if (iCH1_Mapping == 0) CH1_OFF();
  if (iCH2_Mapping == 0) CH2_OFF();
}

void HumidityOn()
{
  if (iCH0_Mapping == 1) CH0_ON();
  if (iCH1_Mapping == 1) CH1_ON();
  if (iCH2_Mapping == 1) CH2_ON();
  SetHumidityOn();
}

void HumidityOff()
{
  if (iCH0_Mapping == 1) CH0_OFF();
  if (iCH1_Mapping == 1) CH1_OFF();
  if (iCH2_Mapping == 1) CH2_OFF();
  SetHumidityOff();
}

void TimerOn()
{
  if (iCH0_Mapping == 2) CH0_ON();
  if (iCH1_Mapping == 2) CH1_ON();
  if (iCH2_Mapping == 2) CH2_ON();
  SetTimerOn();
}

void TimerOff()
{
  if (iCH0_Mapping == 2) CH0_OFF();
  if (iCH1_Mapping == 2) CH1_OFF();
  if (iCH2_Mapping == 2) CH2_OFF();
  SetTimerOff();
}


/* ------------------------------------------------------------------------------*/
float fTemprRef = 30.0;
float fTemprHysteresis = 0.1;

float fK_p = 16.23729;
float fK_i = 0.019;

// sampling time [0.1sec]
uint16_t iHeaterCtrlInterval = 1;
#define t_s (iHeaterCtrlInterval/10.0)
// K_i for discrete action
#define K_i (fK_i*t_s)

// output limits
#define u_max 100
#define u_min 0

// integrator
float fInt = 0; // maube use double?

// returns % of triac power: 0..100
float PI_Controller(float e)
{
  float u;
  
  // control
  u = K_i*fInt + fK_p*e;

  // integrator action
  if (u > u_max || u < u_min)
      ; // do nothing with integrator
  else
      fInt = fInt + e;

  // input saturation
  if (u > u_max)
      u = u_max;
  if (u < u_min)
      u = u_min;

  return u;
}

uint16_t iHumidityCtrlInterval = 50;
float fHumidityUpperRef = 80.0;
float fHumidityLowerRef = 20.0;

/* Modbus conversion functions -----------------------------------------------*/
void TemprHysteresis_TxCB(uint16_t* dataOut)
{
  *dataOut = round(fTemprHysteresis*100.0);
}

void TemprHysteresis_RxCB(uint16_t dataIn)
{
  fTemprHysteresis = dataIn/100.0;
}

void TemprKp_RxCB(uint16_t dataIn)
{
  fK_p = dataIn/100.0;
}

void TemprKp_TxCB(uint16_t* dataOut)
{
  *dataOut = round(fK_p*100.0);
}

void TemprKi_RxCB(uint16_t dataIn)
{
  fK_i = dataIn/1000.0;
}

void TemprKi_TxCB(uint16_t* dataOut)
{
  *dataOut = round(fK_i*1000.0);
}

void TemprRef_RxCB(uint16_t dataIn)
{
  fTemprRef = dataIn/100.0;
}

void TemprRef_TxCB(uint16_t* dataOut)
{
  *dataOut = round(fTemprRef*100.0);
}

void HumUp_RxCB(uint16_t dataIn)
{
  fHumidityUpperRef = dataIn/100.0;

}

void HumUp_TxCB(uint16_t* dataOut)
{
  *dataOut = round(fHumidityUpperRef*100.0);
}

void HumLow_RxCB(uint16_t dataIn)
{
  fHumidityLowerRef = dataIn/100.0;
}

void HumLow_TxCB(uint16_t* dataOut)
{
  *dataOut = round(fHumidityLowerRef*100.0);
}

void TimerOff_RxCB(uint16_t dataIn)
{
  iTimerOffInterval = dataIn*60;
}
  
void TimerOff_TxCB(uint16_t* dataOut)
{
  *dataOut = iTimerOffInterval/60;
}

void FlashWrite_RxCB(uint16_t dataIn)
{
  ProgramFlash(); 
}

/* USER CODE END PFP */

/* USER CODE BEGIN 0 */

/* Code for HTU21D -----------------------------------------------------------*/

//https://developer.mbed.org/users/hwing91/code/HTU21D/file/db86ad1b4459/HTU21D

// Acquired from Datasheet.
 
#define HTU21D_I2C_ADDRESS  0x40 
#define TRIGGER_TEMP_MEASURE  0xE3
#define TRIGGER_HUMD_MEASURE  0xE5
 
// Commands.
#define HTU21D_EEPROM_WRITE 0x80
#define HTU21D_EEPROM_READ  0x81

 
float sample_ctemp(void) {
 
    char tx[1];
    char rx[8];
 
    tx[0] = TRIGGER_TEMP_MEASURE; // Triggers a temperature measure by feeding correct opcode.
  
    /*##-2- Start the transmission process #####################################*/  
    /* While the I2C in reception process, user can transmit data through 
       "aTxBuffer" buffer */
    while(HAL_I2C_Master_Transmit_IT(&hi2c1, (uint16_t)((HTU21D_I2C_ADDRESS << 1) & 0xFE), (uint8_t*)tx, 1)!= HAL_OK)
    {
      /* Error_Handler() function is called when Timeout error occurs.
         When Acknowledge failure occurs (Slave don't acknowledge it's address)
         Master restarts communication */
      if (HAL_I2C_GetError(&hi2c1) != HAL_I2C_ERROR_AF)
      {
        Error_Handler();
      }
    }

    HAL_Delay(60);
    
    // Reads triggered measure    
    /*##-4- Put I2C peripheral in reception process ############################*/  
    while(HAL_I2C_Master_Receive_IT(&hi2c1, (uint16_t)((HTU21D_I2C_ADDRESS << 1) | 0x01), (uint8_t *)rx, 2) != HAL_OK)
    {
      /* Error_Handler() function is called when Timeout error occurs.
         When Acknowledge failure occurs (Slave don't acknowledge it's address)
         Master restarts communication */
      if (HAL_I2C_GetError(&hi2c1) != HAL_I2C_ERROR_AF)
      {
        Error_Handler();
      }   
    }
    
    HAL_Delay(1);
    
    // Algorithm from datasheet to compute temperature.
    unsigned int rawTemperature = ((unsigned int) rx[0] << 8) | (unsigned int) rx[1];
    rawTemperature &= 0xFFFC;
 
    float tempTemperature = rawTemperature / (float)65536; //2^16 = 65536
    float realTemperature = -46.85 + (175.72 * tempTemperature); //From page 14
 
    return realTemperature;
 
}
 
float sample_humid(void) {
 
    char tx[1];
    char rx[2];
 
 
    tx[0] = TRIGGER_HUMD_MEASURE; // Triggers a humidity measure by feeding correct opcode.

    /*##-2- Start the transmission process #####################################*/  
    /* While the I2C in reception process, user can transmit data through 
       "aTxBuffer" buffer */
    while(HAL_I2C_Master_Transmit_IT(&hi2c1, (uint16_t)((HTU21D_I2C_ADDRESS << 1) & 0xFE), (uint8_t*)tx, 1)!= HAL_OK)
    {
      /* Error_Handler() function is called when Timeout error occurs.
         When Acknowledge failure occurs (Slave don't acknowledge it's address)
         Master restarts communication */
      if (HAL_I2C_GetError(&hi2c1) != HAL_I2C_ERROR_AF)
      {
        Error_Handler();
      }
    }
  
    HAL_Delay(60);
    
    // Reads triggered measure
    //i2c_->read((HTU21D_I2C_ADDRESS << 1) | 0x01, rx, 2);
    while(HAL_I2C_Master_Receive_IT(&hi2c1, (uint16_t)((HTU21D_I2C_ADDRESS << 1) | 0x01), (uint8_t *)rx, 2) != HAL_OK)
    {
      /* Error_Handler() function is called when Timeout error occurs.
         When Acknowledge failure occurs (Slave don't acknowledge it's address)
         Master restarts communication */
      if (HAL_I2C_GetError(&hi2c1) != HAL_I2C_ERROR_AF)
      {
        Error_Handler();
      }   
    }
    
    HAL_Delay(1);
    
    //Algorithm from datasheet.
    unsigned int rawHumidity = ((unsigned int) rx[0] << 8) | (unsigned int) rx[1];
 
    rawHumidity &= 0xFFFC; //Zero out the status bits but keep them in place
    
    //Given the raw humidity data, calculate the actual relative humidity
    float tempRH = rawHumidity / (float)65536; //2^16 = 65536
    float rh = -6 + (125 * tempRH); //From page 14
 
 
    return rh;
 
}
/* ---------------------------------------------------------------------------*/
// error handler variables
/*
#define TEMP_ERROR_COUNT 5 // max concecutive ds18b20 errors before beep
unsigned char iError_TempCount = 0; // counter for ds18b20 errors
#define HEATER_ERROR_INTERVAL (30*60) // duration of output saturation before error signal [in sec]
#define HEATER_ERROR_COUNT (HEATER_ERROR_INTERVAL/t_s)
int iError_HeaterCount = 0; // counter for output saturation
#define MAX_TEMP_ERROR 2.0 // maximum acceptable temperature error for heater fault check [oC]
*/
void Beep_On()
{
  //TODO:
}

void Beep_Off()
{
  //TODO:
}

/* ---------------------------------------------------------------------------*/
// TRIAC CONTROL 

// duration of tick in sec for all timers -- one timer tick is 10 uS
#define TIMER_TICK 0.00001

//uint8_t bOutputState = 0;

#define MAX_TRIAC_DUTY 1000
//int iTriacDuty = 250; // 0..1000 -- from no to full power
//int iTriacDutyPrev = 0; 
int iPeriod = 810; //1/(2*TIMER_TICK*60.0);
int iTriacOffset = 40;

// load compare value for triac timer, Duty in interval [0..MAX_TRIAC_DUTY]
void TimerTriac_Load(int Duty)
{
  TIM_OC_InitTypeDef sConfigOC;
  
  sConfigOC.OCMode = TIM_OCMODE_TIMING;
  sConfigOC.Pulse = iPeriod - (iPeriod*Duty / MAX_TRIAC_DUTY) + iTriacOffset;
  
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  HAL_TIM_OC_ConfigChannel(&htim16, &sConfigOC, TIM_CHANNEL_1);
}

#define TimerTriac_Restart() __HAL_TIM_SET_COUNTER(&htim16, 0);

/**
  * @brief  Output Compare callback in non blocking mode 
  * @param  htim : TIM OC handle
  * @retval None
  */
void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef *htim)
{
  if (htim == &htim14)
  {
    /*
    __HAL_TIM_SET_COUNTER(htim, 0);
    
    if (bOutputState == 0)
    {
      HAL_GPIO_WritePin(CH0_GPIO_Port, CH0_Pin, GPIO_PIN_RESET);
      for (int i = 0; i < 2000; i++) ;
      HAL_GPIO_WritePin(CH1_GPIO_Port, CH1_Pin, GPIO_PIN_SET);
    }
    else
    {
      HAL_GPIO_WritePin(CH1_GPIO_Port, CH1_Pin, GPIO_PIN_RESET);
      for (int i = 0; i < 2000; i++) ;
      HAL_GPIO_WritePin(CH0_GPIO_Port, CH0_Pin, GPIO_PIN_SET);
    }
    bOutputState = !bOutputState;
    */
  }
  else
  if (htim == &htim16)
  {
    if (iHeaterCtrlType || IsManualControl())
    {
      if (iTriacDuty > 1)
        HeaterOn();
    }
  }
}


/**
  * @brief  Period elapsed callback in non blocking mode 
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if (htim == &htim14)
  {
    HAL_GPIO_TogglePin(CH0_GPIO_Port, CH0_Pin);
    HAL_GPIO_TogglePin(CH1_GPIO_Port, CH1_Pin);  
  }
}
/* ---------------------------------------------------------------------------*/

/**
  * @brief  EXTI line detection callback.
  * @param  GPIO_Pin: Specifies the port pin connected to corresponding EXTI line.
  * @retval None
  */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  if (GPIO_Pin == CROSS_POS_FALL_Pin)
  {
    if (iHeaterCtrlType || IsManualControl()) // if regulator enabled
    {
      
    }
  }
  else
  if (GPIO_Pin == CROSS_POS_Pin)
  {
    if (iHeaterCtrlType || IsManualControl()) // if regulator enabled
    {
      HeaterOff(); // TODO: to enlarge used power, we can turn off after some pause after posedge
      TimerTriac_Restart();
    }
  }
  else
  if (GPIO_Pin == CROSS_NEG_FALL_Pin)
  {
    if (iHeaterCtrlType || IsManualControl()) // if regulator enabled
    {
      
    }
  }
  else
  if (GPIO_Pin == CROSS_NEG_Pin)
  {
    if (iHeaterCtrlType || IsManualControl()) // if regulator enabled
    {
      HeaterOff();
      TimerTriac_Restart();
    }
  }
}

/* ---------------------------------------------------------------------------*/
#define QUEUE_SIZE 64

typedef unsigned char byte;

// this is template-like magic to define charQueue
#include "queue.h"
CREATE_QUEUE_TYPE_H(byte)
CREATE_QUEUE_TYPE_C(byte)

//byteQueue rxQueue;
byteQueue txQueue;

#define RXBUFFERSIZE 1
uint8_t aRxBuffer[RXBUFFERSIZE];

// uart TX implementation for Modbus

// clear uart transmitter Queue
void hal_uart_clear()
{
  byteQueue_Init(&txQueue); 
}

// add data byte to uart TX
void hal_uart_put(uint8_t data)
{  
  byteQueue_Put(&txQueue, data);
}

// initiation of data transmission
void hal_uart_send()
{
  if(HAL_UART_Transmit_IT(&huart1, txQueue.data, txQueue.in) != HAL_OK)
  {
    Error_Handler();
  }
}  
/* ---------------------------------------------------------------------------*/

static __IO uint32_t uwTick = 0; // 1 ms tick
static __IO uint32_t uwTick01sec = 0; // 100 ms tick
static __IO uint32_t uwTimerTimeStamp = 0;
static __IO uint32_t uwHeaterTimeStamp = 0;
static __IO uint32_t uwHumidityTimeStamp = 0;
uint32_t iDivTick = 0;
/**
  * @brief This function is called to increment a global variable "uwTick"
  *        used as application time base (overwritten)
  */
void HAL_IncTick(void)
{
  uwTick++;
  iDivTick++;
  
  if (iDivTick == 100)
  {
    iDivTick = 0;
    uwTick01sec++;
  }
}

/**
  * @brief This function is called to increment a global variable "uwTick"
  *        used as application time base (overwritten)
  */
uint32_t HAL_GetTick(void)
{
  return uwTick;
}
/* ---------------------------------------------------------------------------*/
// Protection function
short int iOTPValue; // temperature: 100 -- 1 oC, 32768 -- 327.68 oC
unsigned short int iOTPTime;
short int iUTPValue;
unsigned short int iUTPTime;
short int iOHPValue; // humidity: 100 -- 1%, 10000 -- 100%
unsigned short int iOHPTime;
short int iUHPValue;
unsigned short int iUHPTime;

// counters in seconds
unsigned short int iOTPCounter = 0;
unsigned short int iUTPCounter = 0;
unsigned short int iOHPCounter = 0;
unsigned short int iUHPCounter = 0;

#define BEEP_ON() HAL_GPIO_WritePin(BEEP_GPIO_Port, BEEP_Pin, GPIO_PIN_SET);
#define BEEP_OFF() HAL_GPIO_WritePin(BEEP_GPIO_Port, BEEP_Pin, GPIO_PIN_RESET);

unsigned char bTemperatureFault = 0;
unsigned char bHumidityFault = 0;

void CheckProtection()
{
  if (iTemperature > iOTPValue)
  {
    iOTPCounter++;
    if (iOTPCounter > iOTPTime)
      bTemperatureFault = 1;
  }
  else
    iOTPCounter = 0;
  
  if (iTemperature < iUTPValue)
  {
    iUTPCounter++;
    if (iUTPCounter > iUTPTime)
      bTemperatureFault = 1;
  }
  else
    iUTPCounter = 0;
  
  if (iHumidity > iOHPValue)
  {
    iOHPCounter++;
    if (iOHPCounter > iOHPTime)
      bHumidityFault = 1;
  }
  else
    iOHPCounter = 0;
  
  if (iHumidity < iUHPValue)
  {
    iUHPCounter++;
    if (iUHPCounter > iUHPTime)
      bHumidityFault = 1;
  }
  else
    iUHPCounter = 0;
  
  if (bTemperatureFault || bHumidityFault) BEEP_ON();
}
/* ---------------------------------------------------------------------------*/
/* USER CODE END 0 */

int main(void)
{
  /* USER CODE BEGIN 1 */
  float fRes;
  float fE;
  /* USER CODE END 1 */

  /* MCU Configuration----------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_TIM17_Init();
  MX_USART1_UART_Init();
  MX_TIM14_Init();
  MX_TIM16_Init();
  //MX_TIM15_Init();

  /* USER CODE BEGIN 2 */
  // init modbus rx and tx Queues
  //byteQueue_Init(&rxQueue);
  byteQueue_Init(&txQueue);
  
  // creating Modbus entries
  Modbus_Init(6);
  Modbus_CreateEntry("temperature", 0, &iTemperature, 0, 0);
  Modbus_CreateEntry("humidity", 1, &iHumidity, 0, 0);
  Modbus_CreateEntry("timer", 2, &iTimer, 0, 0);
  Modbus_CreateEntry("output state", 3, &iOutputState, 0, 0);
  
  Modbus_CreateEntry("CH0 mapping", 4, &iCH0_Mapping, 0, 0);
  Modbus_CreateEntry("CH1 mapping", 5, &iCH1_Mapping, 0, 0);
  Modbus_CreateEntry("CH2 mapping", 6, &iCH2_Mapping, 0, 0);
  
  Modbus_CreateEntry("Temp.Ctrl type", 7, &iHeaterCtrlType, 0, 0);
  Modbus_CreateEntry("Temp.Ctrl interval", 8, &iHeaterCtrlInterval, 0, 0);
  Modbus_CreateEntry("Temp.Ctrl hysteresis", 9, 0, TemprHysteresis_RxCB, TemprHysteresis_TxCB);
  Modbus_CreateEntry("Temp.Ctrl Kp", 10, 0, TemprKp_RxCB, TemprKp_TxCB);
  Modbus_CreateEntry("Temp.Ctrl Ki", 11, 0, TemprKi_RxCB, TemprKi_TxCB);
  Modbus_CreateEntry("Temp.Ctrl Ref", 12, 0, TemprRef_RxCB, TemprRef_TxCB);

  Modbus_CreateEntry("Hum.Ctrl interval", 13, &iHumidityCtrlInterval, 0, 0);
  Modbus_CreateEntry("Hum.Ctrl low value", 14, 0, HumLow_RxCB, HumLow_TxCB);
  Modbus_CreateEntry("Hum.Ctrl up value", 15, 0, HumUp_RxCB, HumUp_TxCB);
  Modbus_CreateEntry("Hum.Ctrl invert out", 16, &iHumidityOutputInv, 0, 0);
  
  Modbus_CreateEntry("Timer on interval", 17, &iTimerOnInterval, 0, 0);
  Modbus_CreateEntry("Timer off interval", 18, 0, TimerOff_RxCB, TimerOff_TxCB);
  
  Modbus_CreateEntry("Protection OTP value", 19, &iOTPValue, 0, 0); // MODBUS_ADDR_OTP_VALUE 19
  Modbus_CreateEntry("Protection OTP time", 20, &iOTPTime, 0, 0); // MODBUS_ADDR_OTP_TIME 20
  Modbus_CreateEntry("Protection UTP value", 21, &iUTPValue, 0, 0); // MODBUS_ADDR_UTP_VALUE 21
  Modbus_CreateEntry("Protection UTP time", 22, &iUTPTime, 0, 0); // MODBUS_ADDR_UTP_TIME 22
  Modbus_CreateEntry("Protection OHP value", 23, &iOHPValue, 0, 0); // MODBUS_ADDR_OHP_VALUE 23
  Modbus_CreateEntry("Protection OHP time", 24, &iOHPTime, 0, 0); // MODBUS_ADDR_OHP_TIME 24
  Modbus_CreateEntry("Protection UHP value", 25, &iUHPValue, 0, 0); // MODBUS_ADDR_UHP_VALUE 25
  Modbus_CreateEntry("Protection UHP time", 26, &iUHPTime, 0, 0); // MODBUS_ADDR_UHP_TIME 26

  Modbus_CreateEntry("Flash write", 27, 0, FlashWrite_RxCB, 0);
  
  ReadFlash(); // read parameters from Flash memory
  
  // TIM14 -- external signal generation
  __HAL_DBGMCU_FREEZE_TIM14();
  //HAL_GPIO_WritePin(CH1_GPIO_Port, CH1_Pin, GPIO_PIN_SET);
  //HAL_TIM_Base_Start_IT(&htim14);
  HAL_TIM_OC_Start_IT(&htim14, TIM_CHANNEL_1);
  
  // TIM15 -- time stamp for period measurements
  //__HAL_DBGMCU_FREEZE_TIM15();
  //HAL_TIM_Base_Start_IT(&htim15);
  
  // TIM16 -- timer for triac control
  __HAL_DBGMCU_FREEZE_TIM16();
  HAL_TIM_OC_Start_IT(&htim16, TIM_CHANNEL_1);
  
  
  // Put UART peripheral in reception process
  // receiving by one byte  
  if(HAL_UART_Receive_IT(&huart1, (uint8_t *)aRxBuffer, 1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */
    
    /* USER CODE BEGIN 3 */  
    
    // incrementing timer
    if (uwTick01sec >= uwTimerTimeStamp)
    {
      iTimer++;
      uwTimerTimeStamp += 10; // this is one second
      
      if (!IsManualControl()) // if timer enabled
      {
        if (bTimerOn)
        {
          if (iTimer > iTimerOnInterval)
          {
            bTimerOn = 0;
            iTimer = 0;
            TimerOff();
          }
        }
        else
        {
          if (iTimer > iTimerOffInterval)
          {
            bTimerOn = 1;
            iTimer = 0;
            TimerOn();
          }
        }
      }
      
      CheckProtection();
    }
    
    // heater controller action
    if (uwTick01sec >= uwHeaterTimeStamp)
    {
      uwHeaterTimeStamp += iHeaterCtrlInterval;
      
      // reading temperature
      fRes = sample_ctemp();
      iTemperature = fRes*100;
      
      if (!IsManualControl()) // if regulator enabled
      {
        fE = fTemprRef - fRes;
        if (iHeaterCtrlType == CTRL_TYPE_PI)
        {
          // PI controller action
          fRes = PI_Controller(fE); 
          
          SetHeaterState(fRes*10); // setting iTriacDuty variable
          if (iTriacDuty > 1)
            TimerTriac_Load(iTriacDuty);
          else
            HeaterOff();
          
          
          bHybridCtrlState = HYBRID_STATE_IDLE; // TODO: move it to Modbus write callback
        }
        else
        if (iHeaterCtrlType == CTRL_TYPE_ONOFF)
        {
          if (fE < -fTemprHysteresis)
          {
            SetHeaterState(0);
            HeaterOff();
          }
          else
          if (fE > fTemprHysteresis)
          {
            SetHeaterState(MAX_TRIAC_DUTY);
            HeaterOn();
          }
          
          bHybridCtrlState = HYBRID_STATE_IDLE; // TODO: move it to Modbus write callback
        }
        else
        {
          if (bHybridCtrlState == HYBRID_STATE_IDLE)
          {
            // reset all stuff
            iHybridCtrlPulseCount = 0;
            iHybridCtrlOnCounter = 0;
            iHybridCtrlOffCounter = 0;
            bHybridCtrlState = HYBRID_STATE_ONOFF;
            SetHeaterState(0);
          }
          else
          if (bHybridCtrlState == HYBRID_STATE_ONOFF)
          {
            // on-off stage

            // calculating of on and off durations
            if (bHybridCtrlHeaterOn)
              iHybridCtrlOnCounter++;
            else
              iHybridCtrlOffCounter++;
            
            if (fE < -fTemprHysteresis)
            {
              SetHeaterState(0);
              HeaterOff();
              
              if (bHybridCtrlHeaterOn)
              {
                // incrementing on-off cycle count
                iHybridCtrlPulseCount++;
                if (iHybridCtrlPulseCount == HYBRID_CTRL_PULSE_COUNT) // this is the last cycle
                {
                  if (iHybridCtrlOnCounter + iHybridCtrlOffCounter == 0)
                    fRes = 0.5; // this is only to prevent division by 0
                  else
                    fRes = (float)(iHybridCtrlOnCounter) / (float)(iHybridCtrlOnCounter + iHybridCtrlOffCounter); // duty cycle
                  // (u_max*fRes) == PI controller output (+ some nonlinear correction) == K_i*fInt 
                  
                  // nonlinear compensation based on 
                  // "An Accurate Formula For The Firing Angle Of The Phase Angle Control In Terms Of The Duty Cycle Of The Integral Cycle Control"
                  // http://jase.esrgroups.org/papers/6_1_4_12.pdf
                  fRes = 1 - ((-4.499*fRes*fRes*fRes + 6.79*fRes*fRes - 4.68*fRes + 2.77))/3.1415926;
                  
                  if (fabs(K_i) > 0.001)
                    fInt = (u_max*fRes) / K_i;
                  else
                    fInt = 0;
                  
                  bHybridCtrlState = HYBRID_STATE_WAIT_TO_CROSS; 
                }
                
                iHybridCtrlOnCounter = 0;
                iHybridCtrlOffCounter = 0;
              }
              
              bHybridCtrlHeaterOn = 0;
            }
            else
            if (fE > fTemprHysteresis)
            {
              SetHeaterState(MAX_TRIAC_DUTY);
              HeaterOn();
              
              bHybridCtrlHeaterOn = 1;
            }
          }
          else
          if (bHybridCtrlState == HYBRID_STATE_WAIT_TO_CROSS)
          {
            // switch to PI only when we just went below the reference point
            if (fE > 0)
              bHybridCtrlState = HYBRID_STATE_PI;
          }
          else
          if (bHybridCtrlState == HYBRID_STATE_PI)
          {
            // PI controller action
            fRes = PI_Controller(fE); 
            
            SetHeaterState(fRes*10); // setting iTriacDuty variable
            if (iTriacDuty > 1)
              TimerTriac_Load(iTriacDuty);
            else
              HeaterOff();
            
            // we out of the boundaries -- go to initial state
            if (fE < -fTemprHysteresis) bHybridCtrlState = HYBRID_STATE_IDLE;
            if (fE > fTemprHysteresis) bHybridCtrlState = HYBRID_STATE_IDLE;
          }
          
          
          
          if (iTriacDuty > 1)
            TimerTriac_Load(iTriacDuty);
          else
            HeaterOff();
        }
      }
      /*
      // heater fault check
      if (fabs(fE) > MAX_TEMP_ERROR)
      {
        iError_HeaterCount++; 
        if (iError_HeaterCount >= HEATER_ERROR_COUNT) 
        {
          Beep_On();
        }
      }
      else
      {
        iError_HeaterCount = 0;
        Beep_Off();
      }
      */
      
      HAL_GPIO_TogglePin(GPIOA, LEDG_Pin);
    }

    // humidity controller action
    if (uwTick01sec >= uwHumidityTimeStamp)
    {
      uwHumidityTimeStamp += iHumidityCtrlInterval;
      
      fRes = sample_humid();
      iHumidity = fRes*100;
      
      if (!IsManualControl()) // if regulator enabled
      {
        if (fRes < fHumidityLowerRef)
        {
          if (iHumidityOutputInv == 0)
            HumidityOn();
          else
            HumidityOff();
        }
        else
        if (fRes > fHumidityUpperRef)
        {
          if (iHumidityOutputInv == 0)
            HumidityOff();
          else
            HumidityOn();
        }
      }
		}
    
    
    if (IsManualControl())
    {
      if (iTriacDuty > 1)
        TimerTriac_Load(iTriacDuty);
      else
        HeaterOff();

      if (IsHumidityOn())
        HumidityOn();
      else
        HumidityOff();
      
      if (IsTimerOn())
        TimerOn();
      else
        TimerOff();
      
      HAL_Delay(100); // TODO: implement normal time-sample based timing
    }
  }
  /* USER CODE END 3 */

}

/** System Clock Configuration
*/
void SystemClock_Config(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_PeriphCLKInitTypeDef PeriphClkInit;

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = 16;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL6;
  RCC_OscInitStruct.PLL.PREDIV = RCC_PREDIV_DIV1;
  HAL_RCC_OscConfig(&RCC_OscInitStruct);

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1);

  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1|RCC_PERIPHCLK_I2C1;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK1;
  PeriphClkInit.I2c1ClockSelection = RCC_I2C1CLKSOURCE_HSI;
  HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit);

  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/* I2C1 init function */
void MX_I2C1_Init(void)
{

  hi2c1.Instance = I2C1;
  hi2c1.Init.Timing = 0x00101D2D;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLED;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLED;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLED;
  HAL_I2C_Init(&hi2c1);

    /**Configure Analogue filter 
    */
  HAL_I2CEx_AnalogFilter_Config(&hi2c1, I2C_ANALOGFILTER_DISABLED);

}

/* TIM14 init function */
void MX_TIM14_Init(void)
{

  TIM_OC_InitTypeDef sConfigOC;

  htim14.Instance = TIM14;
  htim14.Init.Prescaler = (uint16_t) (SystemCoreClock * TIMER_TICK) - 1;
  htim14.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim14.Init.Period = 65535;
  htim14.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  HAL_TIM_Base_Init(&htim14);

  HAL_TIM_OC_Init(&htim14);

  sConfigOC.OCMode = TIM_OCMODE_TIMING;
  sConfigOC.Pulse = 833;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  HAL_TIM_OC_ConfigChannel(&htim14, &sConfigOC, TIM_CHANNEL_1);

}

/* TIM15 init function 
void MX_TIM15_Init(void)
{

  TIM_MasterConfigTypeDef sMasterConfig;
  TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig;
  TIM_OC_InitTypeDef sConfigOC;

  htim15.Instance = TIM15;
  htim15.Init.Prescaler = (uint16_t) (SystemCoreClock * TIMER_TICK) - 1;
  htim15.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim15.Init.Period = 65535;
  htim15.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim15.Init.RepetitionCounter = 0;
  HAL_TIM_OC_Init(&htim15);

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  HAL_TIMEx_MasterConfigSynchronization(&htim15, &sMasterConfig);

  sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
  sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
  sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
  sBreakDeadTimeConfig.DeadTime = 0;
  sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
  sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
  sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
  HAL_TIMEx_ConfigBreakDeadTime(&htim15, &sBreakDeadTimeConfig);

  sConfigOC.OCMode = TIM_OCMODE_TIMING;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  HAL_TIM_OC_ConfigChannel(&htim15, &sConfigOC, TIM_CHANNEL_1);

}
*/
/* TIM16 init function */
void MX_TIM16_Init(void)
{

  TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig;
  TIM_OC_InitTypeDef sConfigOC;

  htim16.Instance = TIM16;
  htim16.Init.Prescaler = (uint16_t) (SystemCoreClock * TIMER_TICK) - 1;
  htim16.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim16.Init.Period = 65535;
  htim16.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim16.Init.RepetitionCounter = 0;
  HAL_TIM_Base_Init(&htim16);

  HAL_TIM_OC_Init(&htim16);

  sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
  sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
  sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
  sBreakDeadTimeConfig.DeadTime = 0;
  sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
  sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
  sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
  HAL_TIMEx_ConfigBreakDeadTime(&htim16, &sBreakDeadTimeConfig);

  sConfigOC.OCMode = TIM_OCMODE_TIMING;
  sConfigOC.Pulse = 833/2;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  HAL_TIM_OC_ConfigChannel(&htim16, &sConfigOC, TIM_CHANNEL_1);

}

/* TIM17 init function */
void MX_TIM17_Init(void)
{

  htim17.Instance = TIM17;
  htim17.Init.Prescaler = (uint16_t) (SystemCoreClock * TIMER_TICK) - 1;
  htim17.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim17.Init.Period = 65535;
  htim17.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim17.Init.RepetitionCounter = 0;
  HAL_TIM_Base_Init(&htim17);

}

/* USART1 init function */
void MX_USART1_UART_Init(void)
{

  huart1.Instance = USART1;
  huart1.Init.BaudRate = 9600;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONEBIT_SAMPLING_DISABLED ;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  HAL_UART_Init(&huart1);

}

/** Configure pins as 
        * Analog 
        * Input 
        * Output
        * EVENT_OUT
        * EXTI
*/
void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct;

  /* GPIO Ports Clock Enable */
  __GPIOF_CLK_ENABLE();
  __GPIOA_CLK_ENABLE();
  __GPIOC_CLK_ENABLE();
  __GPIOD_CLK_ENABLE();
  __GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, LEDG_Pin|LEDR_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(CH0_GPIO_Port, CH0_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  //HAL_GPIO_WritePin(GPIOB, CH1_Pin|CH2_Pin, GPIO_PIN_RESET);
  //HAL_GPIO_WritePin(CH1_GPIO_Port, CH1_Pin, GPIO_PIN_SET);

  /*Configure GPIO pins : LEDG_Pin LEDR_Pin */
  GPIO_InitStruct.Pin = LEDG_Pin|LEDR_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : CROSS_POS_Pin CROSS_NEG_Pin */
  GPIO_InitStruct.Pin = CROSS_POS_Pin | CROSS_NEG_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(CROSS_POS_GPIO_Port, &GPIO_InitStruct);
  
  GPIO_InitStruct.Pin = CROSS_POS_FALL_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(CROSS_POS_FALL_GPIO_Port, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = CROSS_NEG_FALL_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(CROSS_NEG_FALL_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : CH0_Pin */
  GPIO_InitStruct.Pin = CH0_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
  HAL_GPIO_Init(CH0_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : CH1_Pin CH2_Pin */
  GPIO_InitStruct.Pin = CH1_Pin;
  HAL_GPIO_Init(CH1_GPIO_Port, &GPIO_InitStruct);
  
  GPIO_InitStruct.Pin = CH2_Pin;
  HAL_GPIO_Init(CH2_GPIO_Port, &GPIO_InitStruct);
  
  GPIO_InitStruct.Pin = BEEP_Pin;
  HAL_GPIO_Init(BEEP_GPIO_Port, &GPIO_InitStruct);
  HAL_GPIO_WritePin(BEEP_GPIO_Port, BEEP_Pin, GPIO_PIN_RESET);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI4_15_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI4_15_IRQn);
  HAL_NVIC_SetPriority(EXTI2_3_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI2_3_IRQn);
}

/* USER CODE BEGIN 4 */
/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
static void Error_Handler(void)
{
  //while(1)
  {
    /* Toggle LED2 */
    //HAL_GPIO_TogglePin(GPIOA, LEDR_Pin);
    HAL_GPIO_WritePin(GPIOA, LEDR_Pin, GPIO_PIN_SET);
    HAL_Delay(50);
  }
}

/**
  * @brief  Rx Transfer completed callback
  * @param  UartHandle: UART handle
  * @note   This example shows a simple way to report end of DMA Rx transfer, and 
  *         you can add your own implementation.
  * @retval None
  */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *UartHandle)
{
  /* Set transmission flag: trasfer complete*/
  
  //HAL_UART_Transmit_IT(UartHandle, (uint8_t *)aRxBuffer, 1);
  //HAL_UART_Receive_IT(UartHandle, (uint8_t *)aRxBuffer, 1);
  
  //UartReady = SET;
  
  //byteQueue_Put(&rxQueue, UartHandle->pRxBuffPtr[0]);
  Modbus_Process(aRxBuffer[0]);
  // continue receiving by one byte 
  if (HAL_UART_Receive_IT(UartHandle, (uint8_t *)aRxBuffer, 1) != HAL_OK)
  {
    Error_Handler();
  }
  
  HAL_GPIO_TogglePin(LEDG_GPIO_Port, LEDG_Pin);
  

}


/**
  * @brief  Error callback
  * @param  UartHandle: UART handle
  * @note   This example shows a simple way to report end of IT Rx transfer, and 
  *         you can add your own implementation.
  * @retval None
  */
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
  //Many macros/proc on this code .. is just to be a hint (mem/rec about this possibility)
  __HAL_UART_CLEAR_OREFLAG(huart);
  __HAL_UART_CLEAR_NEFLAG(huart);
  __HAL_UART_CLEAR_FEFLAG(huart);
  __HAL_UART_CLEAR_NEFLAG(huart);

  /* Disable the UART Error Interrupt: (Frame error, noise error, overrun error) */
  __HAL_UART_DISABLE_IT(huart, UART_IT_ERR);
 

  //The most important thing when UART framing error occur/any error is restart the RX process 
  HAL_UART_Receive_IT(huart, (uint8_t *)aRxBuffer, 1);
}

/* Flash memory utils---------------------------------------------------------*/

#define FLASH_USER_OFFSET 0x8007800

void ProgramFlash()
{
  FLASH_EraseInitTypeDef EraseInitStruct;
  int iMemRWAddr = FLASH_USER_OFFSET;
  uint32_t PAGEError = 0;
  
  // Unlock the Flash to enable the flash control register access 
  HAL_FLASH_Unlock();
  
  // Fill EraseInit structure
  EraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGES;
  EraseInitStruct.PageAddress = FLASH_USER_OFFSET;
  EraseInitStruct.NbPages     = 1; // we arasing only one page (teh last one)
  
  // Erase the user Flash area
  if (HAL_FLASHEx_Erase(&EraseInitStruct, &PAGEError) != HAL_OK)
  {
    HAL_GPIO_WritePin(LEDR_GPIO_Port, LEDR_Pin, GPIO_PIN_SET);
    return;
  }

  // writing data
  if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, iMemRWAddr, iCH0_Mapping) != HAL_OK) { HAL_GPIO_WritePin(LEDR_GPIO_Port, LEDR_Pin, GPIO_PIN_SET);  return; } iMemRWAddr+=4;
  if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, iMemRWAddr, iCH1_Mapping) != HAL_OK) { HAL_GPIO_WritePin(LEDR_GPIO_Port, LEDR_Pin, GPIO_PIN_SET);  return; } iMemRWAddr+=4;
  if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, iMemRWAddr, iCH2_Mapping) != HAL_OK) { HAL_GPIO_WritePin(LEDR_GPIO_Port, LEDR_Pin, GPIO_PIN_SET);  return; } iMemRWAddr+=4;
  
  if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, iMemRWAddr, iHeaterCtrlType) != HAL_OK) { HAL_GPIO_WritePin(LEDR_GPIO_Port, LEDR_Pin, GPIO_PIN_SET);  return; } iMemRWAddr+=4;
  if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, iMemRWAddr, iHeaterCtrlInterval) != HAL_OK) { HAL_GPIO_WritePin(LEDR_GPIO_Port, LEDR_Pin, GPIO_PIN_SET);  return; } iMemRWAddr+=4;
  if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, iMemRWAddr, *((uint32_t*)(&fTemprHysteresis))) != HAL_OK) { HAL_GPIO_WritePin(LEDR_GPIO_Port, LEDR_Pin, GPIO_PIN_SET);  return; } iMemRWAddr+=4;
  if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, iMemRWAddr, *((uint32_t*)(&fK_p))) != HAL_OK) { HAL_GPIO_WritePin(LEDR_GPIO_Port, LEDR_Pin, GPIO_PIN_SET);  return; } iMemRWAddr+=4;
  if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, iMemRWAddr, *((uint32_t*)(&fK_i))) != HAL_OK) { HAL_GPIO_WritePin(LEDR_GPIO_Port, LEDR_Pin, GPIO_PIN_SET);  return; } iMemRWAddr+=4;
  if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, iMemRWAddr, *((uint32_t*)(&fTemprRef))) != HAL_OK) { HAL_GPIO_WritePin(LEDR_GPIO_Port, LEDR_Pin, GPIO_PIN_SET);  return; } iMemRWAddr+=4;

  if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, iMemRWAddr, iHumidityCtrlInterval) != HAL_OK) { HAL_GPIO_WritePin(LEDR_GPIO_Port, LEDR_Pin, GPIO_PIN_SET);  return; } iMemRWAddr+=4;
  if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, iMemRWAddr, *((uint32_t*)(&fHumidityLowerRef))) != HAL_OK) { HAL_GPIO_WritePin(LEDR_GPIO_Port, LEDR_Pin, GPIO_PIN_SET);  return; } iMemRWAddr+=4;
  if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, iMemRWAddr, *((uint32_t*)(&fHumidityUpperRef))) != HAL_OK) { HAL_GPIO_WritePin(LEDR_GPIO_Port, LEDR_Pin, GPIO_PIN_SET);  return; } iMemRWAddr+=4;
  if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, iMemRWAddr, iHumidityOutputInv) != HAL_OK) { HAL_GPIO_WritePin(LEDR_GPIO_Port, LEDR_Pin, GPIO_PIN_SET);  return; } iMemRWAddr+=4;
  
  if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, iMemRWAddr, iTimerOnInterval) != HAL_OK) { HAL_GPIO_WritePin(LEDR_GPIO_Port, LEDR_Pin, GPIO_PIN_SET);  return; } iMemRWAddr+=4;
  if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, iMemRWAddr, iTimerOffInterval) != HAL_OK) { HAL_GPIO_WritePin(LEDR_GPIO_Port, LEDR_Pin, GPIO_PIN_SET);  return; } iMemRWAddr+=4;
 
  if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, iMemRWAddr, iOTPValue) != HAL_OK) { HAL_GPIO_WritePin(LEDR_GPIO_Port, LEDR_Pin, GPIO_PIN_SET);  return; } iMemRWAddr+=4;
  if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, iMemRWAddr, iOTPTime) != HAL_OK) { HAL_GPIO_WritePin(LEDR_GPIO_Port, LEDR_Pin, GPIO_PIN_SET);  return; } iMemRWAddr+=4;
  if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, iMemRWAddr, iUTPValue) != HAL_OK) { HAL_GPIO_WritePin(LEDR_GPIO_Port, LEDR_Pin, GPIO_PIN_SET);  return; } iMemRWAddr+=4;
  if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, iMemRWAddr, iUTPTime) != HAL_OK) { HAL_GPIO_WritePin(LEDR_GPIO_Port, LEDR_Pin, GPIO_PIN_SET);  return; } iMemRWAddr+=4;
  if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, iMemRWAddr, iOHPValue) != HAL_OK) { HAL_GPIO_WritePin(LEDR_GPIO_Port, LEDR_Pin, GPIO_PIN_SET);  return; } iMemRWAddr+=4;
  if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, iMemRWAddr, iOHPTime) != HAL_OK) { HAL_GPIO_WritePin(LEDR_GPIO_Port, LEDR_Pin, GPIO_PIN_SET);  return; } iMemRWAddr+=4;
  if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, iMemRWAddr, iUHPValue) != HAL_OK) { HAL_GPIO_WritePin(LEDR_GPIO_Port, LEDR_Pin, GPIO_PIN_SET);  return; } iMemRWAddr+=4;
  if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, iMemRWAddr, iUHPTime) != HAL_OK) { HAL_GPIO_WritePin(LEDR_GPIO_Port, LEDR_Pin, GPIO_PIN_SET);  return; } iMemRWAddr+=4;
  
    // lock flash control
  HAL_FLASH_Lock();
}

void ReadFlash()
{
  int iMemRWAddr = FLASH_USER_OFFSET;

  // reading data  
  iCH0_Mapping = *((uint32_t*)(iMemRWAddr)); iMemRWAddr+=4;
  iCH1_Mapping = *((uint32_t*)(iMemRWAddr)); iMemRWAddr+=4;
  iCH2_Mapping = *((uint32_t*)(iMemRWAddr)); iMemRWAddr+=4;
  
  iHeaterCtrlType = *((uint32_t*)(iMemRWAddr)); iMemRWAddr+=4;
  iHeaterCtrlInterval = *((uint32_t*)(iMemRWAddr)); iMemRWAddr+=4;
  fTemprHysteresis = *((float*)(iMemRWAddr)); iMemRWAddr+=4;
  fK_p = *((float*)(iMemRWAddr)); iMemRWAddr+=4;
  fK_i = *((float*)(iMemRWAddr)); iMemRWAddr+=4;
  fTemprRef = *((float*)(iMemRWAddr)); iMemRWAddr+=4;

  iHumidityCtrlInterval = *((uint32_t*)(iMemRWAddr)); iMemRWAddr+=4;
  fHumidityLowerRef = *((float*)(iMemRWAddr)); iMemRWAddr+=4;
  fHumidityUpperRef = *((float*)(iMemRWAddr)); iMemRWAddr+=4;
  iHumidityOutputInv = *((uint32_t*)(iMemRWAddr)); iMemRWAddr+=4;
  
  iTimerOnInterval = *((uint32_t*)(iMemRWAddr)); iMemRWAddr+=4;
  iTimerOffInterval = *((uint32_t*)(iMemRWAddr)); iMemRWAddr+=4;
  
  iOTPValue = *((uint32_t*)(iMemRWAddr)); iMemRWAddr+=4;
  iOTPTime = *((uint32_t*)(iMemRWAddr)); iMemRWAddr+=4;
  iUTPValue = *((uint32_t*)(iMemRWAddr)); iMemRWAddr+=4;
  iUTPTime = *((uint32_t*)(iMemRWAddr)); iMemRWAddr+=4;
  iOHPValue = *((uint32_t*)(iMemRWAddr)); iMemRWAddr+=4;
  iOHPTime = *((uint32_t*)(iMemRWAddr)); iMemRWAddr+=4;
  iUHPValue = *((uint32_t*)(iMemRWAddr)); iMemRWAddr+=4;
  iUHPTime = *((uint32_t*)(iMemRWAddr)); iMemRWAddr+=4;
}
/* USER CODE END 4 */

#ifdef USE_FULL_ASSERT

/**
   * @brief Reports the name of the source file and the source line number
   * where the assert_param error has occurred.
   * @param file: pointer to the source file name
   * @param line: assert_param error line source number
   * @retval None
   */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
    ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */

}

#endif

/**
  * @}
  */ 

/**
  * @}
*/ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
