#ifndef SYSTICK_H
#define SYSTICK_H

#include<stdint.h>

void systick_init(uint32_t);
void systick_handler(void);
uint32_t uptime_ms(void);
void delay_ms(uint32_t);


#endif