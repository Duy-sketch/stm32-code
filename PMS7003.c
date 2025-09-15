#include "PMS7003.h"
#include "stm32f1xx_hal.h"   // cần cho HAL_GPIO_WritePin, HAL_Delay

#define PMS_START1   0x42
#define PMS_START2   0x4D
#define PMS_FRAME_LEN 32

void PMS7003_Init(PMS7003_Handle *hpms, void *huart, int setpin_port, int setpin_pin) {
    hpms->huart = huart;
    hpms->setpin_port = setpin_port;
    hpms->setpin_pin = setpin_pin;

    hpms->received_bytes = 0;
    hpms->calc_checksum = 0;
    hpms->recv_checksum = 0;

    // Wake sensor (SET = High)
    HAL_GPIO_WritePin((GPIO_TypeDef*)hpms->setpin_port,
                      hpms->setpin_pin,
                      GPIO_PIN_SET);
    HAL_Delay(1000);
}

bool PMS7003_Read(PMS7003_Handle *hpms, uint8_t byte) {
    // Lưu byte trước đó
    hpms->last_byte = hpms->curr_byte;
    hpms->curr_byte = byte;

    // Start frame
    if (hpms->received_bytes == 0 && byte != PMS_START1) return false;
    if (hpms->received_bytes == 1 && byte != PMS_START2) {
        hpms->received_bytes = 0;
        return false;
    }

    // Tích lũy checksum (tính từ byte 0 đến n-3)
    if (hpms->received_bytes < PMS_FRAME_LEN - 2) {
        hpms->calc_checksum += byte;
    }

    hpms->received_bytes++;

    if (hpms->received_bytes == PMS_FRAME_LEN) {
        // Lấy checksum từ 2 byte cuối
        hpms->recv_checksum = (hpms->last_byte << 8) | hpms->curr_byte;

        if (hpms->recv_checksum == hpms->calc_checksum) {
            // Parse data
            hpms->data[PMS_PM1_0]     = (hpms->buffer[10] << 8) | hpms->buffer[11];
            hpms->data[PMS_PM2_5]     = (hpms->buffer[12] << 8) | hpms->buffer[13];
            hpms->data[PMS_PM10]      = (hpms->buffer[14] << 8) | hpms->buffer[15];
            hpms->data[PMS_COUNT_0_3] = (hpms->buffer[22] << 8) | hpms->buffer[23];
            hpms->data[PMS_COUNT_0_5] = (hpms->buffer[24] << 8) | hpms->buffer[25];
            hpms->data[PMS_COUNT_1_0] = (hpms->buffer[26] << 8) | hpms->buffer[27];
            hpms->data[PMS_COUNT_2_5] = (hpms->buffer[28] << 8) | hpms->buffer[29];
            hpms->data[PMS_COUNT_5_0] = (hpms->buffer[30] << 8) | hpms->buffer[31];
            // (nếu cần thêm thì chỉnh offset tiếp)

            // Reset để nhận frame mới
            hpms->received_bytes = 0;
            hpms->calc_checksum = 0;
            return true;
        } else {
            // Checksum fail → reset
            hpms->received_bytes = 0;
            hpms->calc_checksum = 0;
            return false;
        }
    }

    return false;
}

uint16_t PMS7003_GetData(PMS7003_Handle *hpms, PMS7003_DataElement element) {
    if (element < PMS_DATA_MAX) {
        return hpms->data[element];
    }
    return 0;
}

void PMS7003_SetSleep(PMS7003_Handle *hpms, bool sleep) {
    if (sleep) {
        HAL_GPIO_WritePin((GPIO_TypeDef*)hpms->setpin_port,
                          hpms->setpin_pin,
                          GPIO_PIN_RESET);
    } else {
        HAL_GPIO_WritePin((GPIO_TypeDef*)hpms->setpin_port,
                          hpms->setpin_pin,
                          GPIO_PIN_SET);
    }
}
