#ifndef PMS7003_H
#define PMS7003_H
#define PMS_FRAME_LEN 32   // độ dài 1 frame dữ liệu PMS7003
#include "stdint.h"
#include "stdbool.h"

typedef enum {
    PMS_PM1_0,
    PMS_PM2_5,
    PMS_PM10,
    PMS_COUNT_0_3,
    PMS_COUNT_0_5,
    PMS_COUNT_1_0,
    PMS_COUNT_2_5,
    PMS_COUNT_5_0,
    PMS_COUNT_10,
    PMS_DATA_MAX
} PMS7003_DataElement;

typedef struct {
    // UART handle (HAL hoặc bất kỳ driver nào)
    void *huart;

    // GPIO control pin cho SET
    int setpin_port;   // nếu cần (hoặc pointer tới GPIO_TypeDef *)
    int setpin_pin;

    uint16_t data[PMS_DATA_MAX];

    uint16_t last_byte;
    uint16_t curr_byte;
    uint16_t recv_checksum;
    uint16_t calc_checksum;
    unsigned int received_bytes;
    uint8_t buffer[PMS_FRAME_LEN];
} PMS7003_Handle;

void PMS7003_Init(PMS7003_Handle *hpms, void *huart, int setpin_port, int setpin_pin);
bool PMS7003_Read(PMS7003_Handle *hpms, uint8_t byte);
uint16_t PMS7003_GetData(PMS7003_Handle *hpms, PMS7003_DataElement element);
void PMS7003_SetSleep(PMS7003_Handle *hpms, bool sleep);

#endif
