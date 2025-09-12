#include "main.h"
#include "PMS7003(code C).h"
#include <stdio.h>

UART_HandleTypeDef huart2;   // CubeMX sẽ generate
PMS7003_Handle pms;

uint8_t rx_byte;

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_USART2_UART_Init();

    // Khởi tạo PMS7003
    PMS7003_Init(&pms, &huart2, (int)GPIOA, GPIO_PIN_1); // giả sử SET ở PA1

    // Bắt đầu nhận UART từng byte (interrupt)
    HAL_UART_Receive_IT(&huart2, &rx_byte, 1);

    while (1)
    {
        // Vòng lặp chính rảnh → làm việc khác
        HAL_Delay(1000);
        uint16_t pm25 = PMS7003_GetData(&pms, PMS_PM2_5);
        printf("PM2.5 = %u ug/m3\r\n", pm25);
    }
}

/**
  * Hàm callback khi nhận xong 1 byte UART
  */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART2) {
        if (PMS7003_Read(&pms, rx_byte)) {
            // Khi có frame hợp lệ
            uint16_t pm10 = PMS7003_GetData(&pms, PMS_PM10);
            uint16_t pm25 = PMS7003_GetData(&pms, PMS_PM2_5);
            uint16_t pm1  = PMS7003_GetData(&pms, PMS_PM1_0);

            printf("PM1.0 = %u | PM2.5 = %u | PM10 = %u\r\n", pm1, pm25, pm10);
        }
        // Nhận tiếp byte kế
        HAL_UART_Receive_IT(&huart2, &rx_byte, 1);
    }
}

/**
  * Hàm printf redirect về UART (debug qua UART2)
  */
int __io_putchar(int ch)
{
    HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
    return ch;
}
