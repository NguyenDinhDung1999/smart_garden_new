/*

kenh adc 1 cam bien anh sang
kenh adc 2 cam bien do am dat 1
kenh adc 3 lm35
kenh adc 0 bien tro
kenh adc 5 cam bien do am dat 2
*/

#include "main.h"
#include "CLCD_I2C.h"
#include "stdio.h"
#include "DHT.h"
#include "string.h"

#define BNT_UP HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_12)
#define BNT_DOWN HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_3)
#define BNT_BACK HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_4)
#define BNT_CHOOSE HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_15)
#define PUMP_ON HAL_GPIO_WritePin(GPIOA,GPIO_PIN_8, GPIO_PIN_SET)
#define PUMP_OFF HAL_GPIO_WritePin(GPIOA,GPIO_PIN_8, GPIO_PIN_RESET)
#define LIMIT_SWITCH_1 HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_10)
#define LIMIT_SWITCH_2 HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_11)
#define DOOR_UP HAL_GPIO_WritePin(GPIOA,GPIO_PIN_11, GPIO_PIN_SET);

#define DHT_PORT GPIOB
#define DHT_PIN GPIO_PIN_1

ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;
I2C_HandleTypeDef hi2c1;

UART_HandleTypeDef huart1;

CLCD_I2C_Name LCD1;
DHT_DataTypedef DHT11;

char str[16] ;
int len;
uint32_t time_senddata;
uint8_t time_send;
uint8_t old_time_send;
uint32_t time;
uint16_t count_time;
uint16_t count_next_page;
uint16_t readAdc[5];
uint8_t dataRx[1];



uint8_t temp_dht;
uint8_t hum_dht;
uint8_t count_dht;
uint32_t time_dht;
uint8_t old_count;
uint8_t status_send_zigbee;
uint8_t old_status_send_zigbee;
uint8_t count_send_zigbee;
uint8_t old_count_send_zigbee;
uint32_t time_send_zigbee;
uint8_t status_send_zigbee;

typedef struct{
	uint8_t lm35;
	uint8_t photo_diode;
	float humidity1;
	float humidity2;
	uint8_t potentiometer;
	uint8_t sum_humidity;
	uint8_t digital_photo;
	uint8_t digital_hum1;
	uint8_t digital_hum2;
	uint8_t lms1;
	uint8_t lms2;
}SENSOR;
SENSOR sensor;

typedef struct {
	uint16_t channel_1;
	uint16_t channel_2;
	uint16_t channel_3;
	uint16_t channel_0;
	uint16_t channel_5;
}ADC;
ADC adc;

typedef struct {
	float channel_1;
	float channel_2;
	float channel_3;
	float channel_0;
	float channel_5;
}VOL;
VOL vol;

typedef struct{
 uint8_t status;
 uint8_t status_temp;
	
}PAGE0;
PAGE0 page_0;

typedef struct{
	uint8_t pump;
	uint8_t motor;
	uint8_t count_pump;
	uint8_t count_door_up;
	uint8_t count_door_down;
	uint8_t count_door_stop;
	uint8_t run_motor;
	uint8_t old_run_motor;
	uint8_t old_pump;
	
}PAGE1;
PAGE1 page_1;

typedef struct{
	uint8_t set_temp;
	uint8_t cursor;
	uint8_t count;
	uint8_t set_hum;	
	uint8_t count_up;
	uint8_t count_down;
}PAGE2;
PAGE2 page_2;

typedef struct{
	uint8_t bnt_up;
	uint8_t bnt_down;
	uint8_t bnt_back;
	uint8_t bnt_choose;
	uint8_t page;
	uint8_t change;
	uint8_t next;
	uint8_t cursor;
}STATUS;
STATUS status;

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_ADC1_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_I2C1_Init(void);
void page0(void);
void page1(void);
void page2(void);
void changePage(void);
void read_dht11(void);
void send_data(void);

void door_up(void)
{
	//page_1.run_motor = 1;
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_11, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_15, GPIO_PIN_SET);
}
void door_down(void)
{
	//page_1.run_motor = 2;
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_11, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_15, GPIO_PIN_RESET);
}
void door_stop(void)
{
	//page_1.run_motor = 0;
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_11, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_15, GPIO_PIN_RESET);
}

void send_zigbee(void)
{
	if(HAL_GetTick() -  time_send_zigbee > 1100)
	{
		count_send_zigbee ++;
		time_send_zigbee = HAL_GetTick();
	}
	if(count_send_zigbee != old_count_send_zigbee)
	{
		count_send_zigbee = old_count_send_zigbee;
	}
}




int main(void)
{


  HAL_Init();
  SystemClock_Config();

  MX_GPIO_Init();
  MX_DMA_Init();
  MX_ADC1_Init();
  MX_USART1_UART_Init();
  MX_I2C1_Init();
	///MX_TIM4_Init();
	//HAL_TIM_Base_Start_IT(&htim4);
	HAL_Delay(1000);
	
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_11, GPIO_PIN_RESET);
	//HAL_GPIO_WritePin(GPIOA,GPIO_PIN_8, GPIO_PIN_SET);
	PUMP_OFF;
	
	CLCD_I2C_Init(&LCD1,&hi2c1,0x4e,16,2);
	CLCD_I2C_Clear(&LCD1);
	status.page = 0;
	status.change = 1;

  while (1)
  {
		HAL_UART_Receive(&huart1,dataRx,1, 10);	
		send_data();
		read_dht11();
		//HAL_UART_Transmit(&huart1, "/r/n", 4, 10);
    HAL_ADC_Start_DMA(&hadc1, (uint32_t*)readAdc, 5);
		adc.channel_0 = readAdc[0];
		adc.channel_1 = readAdc[1];
		adc.channel_2 = readAdc[2];
		adc.channel_3 = readAdc[3];
		adc.channel_5 = readAdc[4];
		
		vol.channel_0 = ((3.3*adc.channel_0)/4095.0);
		vol.channel_1 = ((3.3*adc.channel_1)/4095.0);
		vol.channel_2 = ((3.3*adc.channel_2)/4095.0);
		vol.channel_3 = ((3.3*adc.channel_3)/4096.0);
		vol.channel_5 = ((3.3*adc.channel_5)/4095.0);

		sensor.humidity1 = (100 - ((100*vol.channel_2)/3.3));
		sensor.humidity2 = (100 - ((100*vol.channel_5)/3.3));
		sensor.sum_humidity = (sensor.humidity1+sensor.humidity2)/2;
		sensor.photo_diode = (100 - ((100*vol.channel_1)/3.3));
		sensor.lm35 = (vol.channel_3*100)/1.5;
		status.bnt_up = BNT_UP;
		status.bnt_down = BNT_DOWN;
		status.bnt_choose = BNT_CHOOSE;
		status.bnt_back = BNT_BACK;
		sensor.lms1 = LIMIT_SWITCH_1;
		sensor.lms2 = LIMIT_SWITCH_2;
		
		changePage();
		
		switch (status.page)
		{
			case 0 :
				page0();
			break;
			case 1 :
				page1();
			break;		
			case 2 :
				page2();
			break;				
		}
		
  }// end loop
}// end main
/*void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
 if(htim->Instance == htim4.Instance)
 {
*
 }
}*/

/*void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart->Instance == huart1.Instance)
	{
		HAL_UART_Receive_IT(&huart1, dataRx, 1);	
	}
}*/
void send_data(void)
{
	
	
	if(status.bnt_choose == 1 && status.bnt_back == 1 && status.bnt_down == 1 && status.bnt_up == 1 && page_1.old_pump == 0 && page_1.old_run_motor == 0)
	{	
		if(HAL_GetTick() - time_senddata > 2000)
		{
			time_send ++;
			time_senddata = HAL_GetTick();
		}
		if(time_send != old_time_send )
		{
			len = sprintf(str, "ab%d+%d-%d*%d:%d", temp_dht, hum_dht,(uint8_t)sensor.lm35,sensor.sum_humidity, sensor.photo_diode);
			HAL_UART_Transmit(&huart1, (uint8_t*)str, 16, 100);
			memset(str, 0, 16);
			
			time_send = old_time_send ;
		}
  }
}

void read_dht11(void)
{
	if(status.bnt_choose == 1 && status.bnt_back == 1 && status.bnt_down == 1 && status.bnt_up == 1)
	{	
		if(HAL_GetTick() - time_dht > 1200)
		{	
			count_dht++;
			time_dht = HAL_GetTick();
		}
		if(count_dht != old_count)
		{
			DHT_GetData(&DHT11);
			temp_dht = DHT11.Temperature;
			hum_dht = DHT11.Humidity;
			count_dht = old_count;
		}
  }
}


void changePage(void)
{
	if(status.change == 1) 
	{
		CLCD_I2C_Clear(&LCD1);
		HAL_Delay(50);
		status.change = 0;
		count_next_page = 0;
	}
	
}

void page0(void)
{
	if(status.bnt_down == 0)
	{
		if(HAL_GetTick() - time > 1)
		{
			count_time ++;
			if(count_time > 2) count_time = 3;
			time = HAL_GetTick();	
		}
		if(count_time == 2) 
		{
			page_0.status = !page_0.status;
			CLCD_I2C_Clear(&LCD1);
		}
		else page_0.status = page_0.status;
	}
	else count_time = 0;
	
	if(page_0.status == 0)
	{
		CLCD_I2C_SetCursor(&LCD1, 0,0);
		CLCD_I2C_WriteString(&LCD1, "Do am dat:");
		CLCD_I2C_SetCursor(&LCD1, 10,0);
		char humdity_land[3];
		sprintf(humdity_land,"%d", sensor.sum_humidity);
		CLCD_I2C_WriteString(&LCD1, humdity_land);
		CLCD_I2C_SetCursor(&LCD1, 13,0);
		CLCD_I2C_WriteString(&LCD1, "%");
			
			
		CLCD_I2C_SetCursor(&LCD1, 0,1);
		CLCD_I2C_WriteString(&LCD1, "Anh sang :");
		char light[3];
		CLCD_I2C_SetCursor(&LCD1, 10,1);
		sprintf(light,"%d", sensor.photo_diode);
		CLCD_I2C_WriteString(&LCD1, light);
		CLCD_I2C_SetCursor(&LCD1, 13,1);
		CLCD_I2C_WriteString(&LCD1, "%");
	}
	else
	{
		CLCD_I2C_SetCursor(&LCD1, 0,0);
		CLCD_I2C_WriteString(&LCD1, "H:");
		char dht_h[3];
		CLCD_I2C_SetCursor(&LCD1, 2,0);
		sprintf(dht_h,"%d", hum_dht);
		CLCD_I2C_WriteString(&LCD1, dht_h);
		CLCD_I2C_SetCursor(&LCD1, 5,0);
		CLCD_I2C_WriteString(&LCD1, "%");
		
		CLCD_I2C_SetCursor(&LCD1, 8,0);
		CLCD_I2C_WriteString(&LCD1, "T:");
		char dht_t[3];
		CLCD_I2C_SetCursor(&LCD1, 10,0);
		sprintf(dht_t,"%d", temp_dht);
		CLCD_I2C_WriteString(&LCD1, dht_t);
		CLCD_I2C_SetCursor(&LCD1, 13,0);
		CLCD_I2C_WriteChar(&LCD1, 223);
		CLCD_I2C_WriteString(&LCD1, "C");
		
		
		CLCD_I2C_SetCursor(&LCD1, 0,1);
		CLCD_I2C_WriteString(&LCD1, "Nhiet do:");	
		char lm35[3];
		CLCD_I2C_SetCursor(&LCD1, 10,1);
		sprintf(lm35,"%d", sensor.lm35);
		CLCD_I2C_WriteString(&LCD1, lm35);
		CLCD_I2C_SetCursor(&LCD1, 13,1);
		CLCD_I2C_WriteChar(&LCD1, 223);
		CLCD_I2C_WriteString(&LCD1, "C");
	}
	
	
  if(status.bnt_back == 1 && status.bnt_choose == 1) status.next = 0;
	if(status.next == 0)
	{
		if(status.bnt_back == 0 && status.bnt_choose == 0)
		{
			if(HAL_GetTick() - time > 1000)
			{
				count_next_page ++;
				time = HAL_GetTick();
			}
			if(count_next_page > 3)
			{
				status.page = 1;
				status.next = 1;
				status.change = 1;
			}
		}
		else count_next_page = 0;
  }
	
	if(page_1.old_pump == 1)
	{
		if(page_1.pump == 0)
		{
			uint8_t dat[2] = "c0";
			HAL_UART_Transmit(&huart1, dat, 2, 1000);
			//HAL_Delay(1000);
			send_zigbee();
		}
		else
		{
			uint8_t dat[2] = "c1";
			HAL_UART_Transmit(&huart1, dat, 2, 1000);
			//HAL_Delay(1000);	
			send_zigbee();			
		}
		page_1.old_pump = 0;
	}
	
	if(sensor.sum_humidity < page_2.set_hum)
	{
		PUMP_ON;
		page_1.pump = 1;
		page_1.old_pump = 1	;	
	}
	else
	{
		PUMP_OFF;
		page_1.pump = 0; 
		page_1.old_pump = 1;
	}
	/***** nhiet do dong cua *******/
	if(temp_dht > page_2.set_temp )
	{
		if(sensor.lms2 != 0)
		{
			door_down();
			uint8_t dat[2] = "d2";
			HAL_UART_Transmit(&huart1, dat, 2, 100);
			//HAL_Delay(1000);
			send_zigbee();
		}
		else
		{
			page_0.status_temp = 1;
			door_stop();
			uint8_t dat[2] = "d0";
			HAL_UART_Transmit(&huart1, dat, 2, 100);
			//HAL_Delay(1000);
			send_zigbee();
		}
	}
	else if(temp_dht <= page_2.set_temp )
	{
		if(sensor.lms1 != 0)
		{
			door_up();
			uint8_t dat[2] = "d1";
			HAL_UART_Transmit(&huart1, dat, 2, 100);
			//HAL_Delay(1000);
			send_zigbee();
		}
		else
		{
		  page_0.status_temp = 0;
			door_stop();
			uint8_t dat[2] = "d0";
			HAL_UART_Transmit(&huart1, dat, 2, 1000);
			//HAL_Delay(1000);
			send_zigbee();
		}	
	}
	
	
	
}

void page1(void)
{
	if(dataRx[0] == 0x6E)
		{
			page_1.pump = 0;
			dataRx[0] = 0x00;
			
		}
		else if(dataRx[0] == 0x6D)
		{
			page_1.pump = 1;
			dataRx[0] = 0x00;
		}
		else if(dataRx[0] == 0x78)
		{
			page_1.run_motor = 1;
			dataRx[0] = 0x00;
		}
		else if(dataRx[0] == 0x79)
		{
			page_1.run_motor = 0;		
			dataRx[0] = 0x00;
		}
		else if(dataRx[0] == 0x7A)
		{
			page_1.run_motor = 2;
			dataRx[0] = 0x00;
		}
		
	CLCD_I2C_SetCursor(&LCD1, 3,0);
  CLCD_I2C_WriteString(&LCD1, "Thu cong");
	/******** chuyen trang *****************/
	if(status.bnt_back == 1 && status.bnt_choose == 1) status.next = 2;
	if(status.next == 2)
	{
		if(status.bnt_back == 0 && status.bnt_choose == 0)
		{
			if(HAL_GetTick() - time > 1000)
			{
				count_next_page ++;
				if(count_next_page > 2) count_next_page = 3;
				time = HAL_GetTick();				
			}
			if(count_next_page > 2)
			{
				status.page = 2;
				status.next = 3;
				status.change = 1;			
			}	
		}
		else count_next_page = 0;
	}
	/******** hien thi dieu kien ***********/
	if(status.bnt_back == 0 && status.bnt_choose == 1 && status.bnt_down == 1 && status.bnt_up == 1)
	{
		if(HAL_GetTick() - time > 1)
		{
			page_1.count_pump ++;
			if(page_1.count_pump > 2) page_1.count_pump = 3;
			time = HAL_GetTick();
		}
		if(page_1.count_pump == 2)
		{
			page_1.pump = !page_1.pump;
			page_1.old_pump = 1;	
		}
	}
	else page_1.count_pump = 0;
	if(page_1.old_pump == 1)
	{
		if(page_1.pump == 0)
		{
			uint8_t dat[2] = "c0";
			HAL_UART_Transmit(&huart1, dat, 2, 1000);
			//HAL_Delay(1000);
			send_zigbee();
		}
		else
		{
			uint8_t dat[2] = "c1";
			HAL_UART_Transmit(&huart1, dat, 2, 1000);
			//HAL_Delay(1000);		
			send_zigbee();
		}
		page_1.old_pump = 0;
	}
	
	if(status.bnt_back == 1 && status.bnt_choose == 1 && status.bnt_down == 0 && status.bnt_up == 1)
	{
		if(HAL_GetTick() - time > 1)
		{
			page_1.count_door_down ++;
			if(page_1.count_door_down > 2) page_1.count_door_down = 3;
			time = HAL_GetTick();
		}
		if(page_1.count_door_down == 2)
		{
			page_1.run_motor = 2;
			page_1.old_run_motor = 1;
		}
	}
	else page_1.count_door_down = 0;
	
	if(status.bnt_back == 1 && status.bnt_choose == 1 && status.bnt_down == 1 && status.bnt_up == 0)
	{
		if(HAL_GetTick() - time > 1)
		{
			page_1.count_door_up ++;
			if(page_1.count_door_up > 2) page_1.count_door_up = 3;
			time = HAL_GetTick();
		}
		if(page_1.count_door_up == 2)
		{
			page_1.run_motor = 1;
			page_1.old_run_motor = 1;
		}
	}
	else page_1.count_door_up = 0;
	
	if(status.bnt_back == 1 && status.bnt_choose == 0 && status.bnt_down == 1 && status.bnt_up == 1)
	{
		if(HAL_GetTick() - time > 1)
		{
			page_1.count_door_stop ++;
			if(page_1.count_door_stop > 2) page_1.count_door_stop = 3;
			time = HAL_GetTick();
		}
		if(page_1.count_door_stop == 2)
		{
			page_1.run_motor = 0;
			page_1.old_run_motor = 1;
		}
	}
	else page_1.count_door_stop = 0;
	
	
	
	CLCD_I2C_SetCursor(&LCD1, 1,1);
  CLCD_I2C_WriteString(&LCD1, "Bom:");
	
	CLCD_I2C_SetCursor(&LCD1, 11,1);
  CLCD_I2C_WriteString(&LCD1, "Mai:");
	
	if(page_1.pump == 0) 
	{
			CLCD_I2C_SetCursor(&LCD1, 5,1);
			CLCD_I2C_WriteString(&LCD1, "T");
			PUMP_OFF;
		  //uint8_t dat[2] = "c0";
			//HAL_UART_Transmit(&huart1, dat, 2, 1000);
			///HAL_Delay(1000);
	}
	else
	{
			CLCD_I2C_SetCursor(&LCD1, 5,1);
			CLCD_I2C_WriteString(&LCD1, "B");
			PUMP_ON;
		  //uint8_t dat[2] = "c1";
			//HAL_UART_Transmit(&huart1, dat, 2, 1000);
			//HAL_Delay(1000);
	}
	
	if(page_1.run_motor == 0)
	{
		CLCD_I2C_SetCursor(&LCD1, 15,1);
		CLCD_I2C_WriteString(&LCD1, "D");
		door_stop();
		//uint8_t dat[2] = "d0";
	  //HAL_UART_Transmit(&huart1, dat, 2, 1000);
		//HAL_Delay(1000);
	}
	else if(page_1.run_motor == 1)
	{
		if(sensor.lms1 != 0)
		{
			CLCD_I2C_SetCursor(&LCD1, 15,1);
			CLCD_I2C_WriteString(&LCD1, "L");
			door_up();
		}
		else 
		{
			page_1.run_motor = 0;
			page_1.old_run_motor = 1;
		}
		
		//uint8_t dat[2] = "d1";
		//HAL_UART_Transmit(&huart1, dat, 2, 1000);
		//HAL_Delay(1000);
	}
	else if(page_1.run_motor == 2)
	{
		if(sensor.lms2 != 0)
		{
			CLCD_I2C_SetCursor(&LCD1, 15,1);
			CLCD_I2C_WriteString(&LCD1, "X");
			door_down();
		}
		else
		{
			page_1.run_motor = 0;
			page_1.old_run_motor = 1;
		}
	}
	/************** gui du lieu *************/
	if(page_1.old_run_motor == 1)
	{
		if(page_1.run_motor == 0)
		{
			uint8_t dat[2] = "d0";
			HAL_UART_Transmit(&huart1, dat, 2, 1000);
			//HAL_Delay(1000);
			send_zigbee();
		}
		if(page_1.run_motor == 1)
		{
			uint8_t dat[2] = "d1";
			HAL_UART_Transmit(&huart1, dat, 2, 1000);
			//HAL_Delay(1000);
			send_zigbee();
		}
		if(page_1.run_motor == 2)
		{
			uint8_t dat[2] = "d2";
			HAL_UART_Transmit(&huart1, dat, 2, 1000);
			//HAL_Delay(1000);
			send_zigbee();
		}
		page_1.old_run_motor = 0;
	}
}


void page2(void)
{
	CLCD_I2C_SetCursor(&LCD1, 5,0);
  CLCD_I2C_WriteString(&LCD1, "Cai dat");
	CLCD_I2C_SetCursor(&LCD1, 1,1);
  CLCD_I2C_WriteString(&LCD1, "Dat:");
	CLCD_I2C_SetCursor(&LCD1, 12,1);
  CLCD_I2C_WriteString(&LCD1, "T:");
	
	char set_humdity[3];
	if(page_2.set_hum < 10)
	{
		CLCD_I2C_SetCursor(&LCD1, 5,1);
		CLCD_I2C_WriteString(&LCD1, "00");
	}
	else
	{
		if(page_2.set_hum == 100) CLCD_I2C_SetCursor(&LCD1, 5,1);
		else 
		{
			CLCD_I2C_SetCursor(&LCD1, 5,1);
			CLCD_I2C_WriteString(&LCD1, "0");
		}
		
	}
	sprintf(set_humdity,"%d", page_2.set_hum);
	CLCD_I2C_WriteString(&LCD1, set_humdity);
	
	
	char set_temp[3];
	if(page_2.set_temp < 10)
	{
		CLCD_I2C_SetCursor(&LCD1, 14,1);
		CLCD_I2C_WriteString(&LCD1, "0");
	}
	else
	{
		CLCD_I2C_SetCursor(&LCD1, 14,1);	
	}
	sprintf(set_temp,"%d", page_2.set_temp);
	CLCD_I2C_WriteString(&LCD1, set_temp);
	/********* cai do am ************/
	if(status.bnt_back == 0 && status.bnt_choose == 1)
	{
		if(HAL_GetTick() - time > 1)
		{
			page_2.count ++;
			if(page_2.count > 2) page_2.count = 3;
			time = HAL_GetTick();
		}
		if(page_2.count == 2)
		{
			page_2.cursor = !page_2.cursor;
			CLCD_I2C_Clear(&LCD1);
		}
	}
	else page_2.count = 0;
	
	
	if(page_2.cursor == 0)
	{
		CLCD_I2C_SetCursor(&LCD1, 0,1);
		CLCD_I2C_WriteString(&LCD1, ">");
		if(status.bnt_up == 0)
		{
			if(HAL_GetTick() - time > 1)
			{
				page_2.count_up ++;
				if(page_2.count_up > 9) page_2.count_up = 10;
				time = HAL_GetTick();		
			}
			if(page_2.count_up == 2)
			{
				page_2.set_hum ++;
				if(page_2.set_hum > 99) page_2.set_hum = 100 ;
				//CLCD_I2C_Clear(&LCD1);
			}
			if(page_2.count_up == 10)
			{
				page_2.set_hum ++;
				if(page_2.set_hum > 99) page_2.set_hum = 100 ;
				//CLCD_I2C_Clear(&LCD1);
			}
		}
		else page_2.count_up = 0;
		
		if(status.bnt_down == 0)
		{
			if(HAL_GetTick() - time > 1)
			{
				page_2.count_down ++;
				if(page_2.count_down > 9) page_2.count_down = 10;
				time = HAL_GetTick();		
			}
			if(page_2.count_down == 2)
			{
				page_2.set_hum --;
				if(page_2.set_hum < 2) page_2.set_hum = 1 ;
				//CLCD_I2C_Clear(&LCD1);
			}
			if(page_2.count_down == 10)
			{
				page_2.set_hum --;
				if(page_2.set_hum < 2) page_2.set_hum = 1 ;
				//CLCD_I2C_Clear(&LCD1);
			}
		}
		else page_2.count_down = 0;		
	}
	/*************** cai nhiet do **********/
	else
	{
		CLCD_I2C_SetCursor(&LCD1, 11,1);
		CLCD_I2C_WriteString(&LCD1, ">");
		if(status.bnt_up == 0)
		{
			if(HAL_GetTick() - time > 1)
			{
				page_2.count_up ++;
				if(page_2.count_up > 9) page_2.count_up = 10;
				time = HAL_GetTick();		
			}
			if(page_2.count_up == 2)
			{
				page_2.set_temp ++;
				if(page_2.set_temp > 98) page_2.set_temp = 99 ;
				//CLCD_I2C_Clear(&LCD1);
			}
			if(page_2.count_up == 10)
			{
				page_2.set_temp ++;
				if(page_2.set_temp > 98) page_2.set_temp = 99 ;
				//CLCD_I2C_Clear(&LCD1);
			}
		}
		else page_2.count_up = 0;
		
		if(status.bnt_down == 0)
		{
			if(HAL_GetTick() - time > 1)
			{
				page_2.count_down ++;
				if(page_2.count_down > 9) page_2.count_down = 10;
				time = HAL_GetTick();		
			}
			if(page_2.count_down == 2)
			{
				page_2.set_temp --;
				if(page_2.set_temp < 2) page_2.set_temp = 1 ;
				//CLCD_I2C_Clear(&LCD1);
			}
			if(page_2.count_down == 10)
			{
				page_2.set_temp --;
				if(page_2.set_temp < 2) page_2.set_temp = 1 ;
				//CLCD_I2C_Clear(&LCD1);
			}
		}
		else page_2.count_down = 0;	
	}
	if(status.bnt_back == 1 && status.bnt_choose == 1) status.next = 4;
	if(status.next == 4)
	{
		if(status.bnt_back == 0 && status.bnt_choose == 0)
		{
			if(HAL_GetTick() - time > 1000)
			{
				count_next_page ++;
				if(count_next_page > 2) count_next_page = 3;
				time = HAL_GetTick();				
			}
			if(count_next_page > 2)
			{
				status.page = 0;
				status.next = 5;
				status.change = 1;			
			}		
		}
		else count_next_page = 0;	
	}
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Common config
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ScanConvMode = ADC_SCAN_ENABLE;
  hadc1.Init.ContinuousConvMode = ENABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 5;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_0;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_1;
  sConfig.Rank = ADC_REGULAR_RANK_2;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_3;
  sConfig.Rank = ADC_REGULAR_RANK_3;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_5;
  sConfig.Rank = ADC_REGULAR_RANK_4;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_6;
  sConfig.Rank = ADC_REGULAR_RANK_5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 9600;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8|GPIO_PIN_11, GPIO_PIN_RESET);

  /*Configure GPIO pin : PC13 */
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PA2 PA4 PA12 PA15 */
  GPIO_InitStruct.Pin = GPIO_PIN_2|GPIO_PIN_4|GPIO_PIN_12|GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PB10 PB11 PB3 PB4
                           PB8 */
  GPIO_InitStruct.Pin = GPIO_PIN_10|GPIO_PIN_11|GPIO_PIN_3|GPIO_PIN_4
                          |GPIO_PIN_8;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : PB15 */
  GPIO_InitStruct.Pin = GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : PA8 PA11 */
  GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_11;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
