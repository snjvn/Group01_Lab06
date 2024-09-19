
#include <stdint.h>
#include <stdbool.h>
#include "tm4c123gh6pm.h"
/**
 * main.c
 */

void GPIOInterrupt(void);
void INIT_SYS_CTRL_REGISTERS(void);
void INIT_GPIO_PORTF_REGISTERS(void);
void INIT_TIMER1_REGISTERS(void);

uint32_t PORTF_Interrupt = 0x00;
int duty = 80;
int main(void)
{
    INIT_SYS_CTRL_REGISTERS();
    INIT_GPIO_PORTF_REGISTERS();
    INIT_TIMER1_REGISTERS();

    while(1){
        NVIC_EN0_R = 0x40000000; // 30th bit controls PORTF
        GPIO_PORTF_IM_R = 0x11; // unmasking both switches
//        GPIO_PORTF_CR_R = 0x00;
        TIMER1_TBMATCHR_R = duty;
    }

    return 0;
}

void INIT_SYS_CTRL_REGISTERS(){
    SYSCTL_RCGC2_R |= 0x00000020;       /* enable clock to GPIOF */
    SYSCTL_RCGCTIMER_R = 0x02;
    SYSCTL_RCGCGPIO_R = 0x20;
}

void INIT_GPIO_PORTF_REGISTERS(){
    GPIO_PORTF_LOCK_R = 0x4C4F434B;     /* unlock commit register */
    GPIO_PORTF_CR_R = 0x1F;             /* make PORTF configurable */
    GPIO_PORTF_DEN_R = 0x01F;            /* set PORTF pins 4 : 0 pins */
    GPIO_PORTF_DIR_R = 0x0E;            /*  */
    GPIO_PORTF_PUR_R = 0x11;            /* PORTF0 and PORTF4 are pulled up */
    GPIO_PORTF_AFSEL_R = 0x08;
    GPIO_PORTF_PCTL_R = 0x00007000;

    NVIC_EN0_R = 0x40000000; // 30th bit controls PORTF
    GPIO_PORTF_IS_R = 0x00; // interrupt sensitivity - edge
    GPIO_PORTF_IEV_R = 0x00;
    GPIO_PORTF_IM_R = 0x11; // unmasking both switches
}

void INIT_TIMER1_REGISTERS(){
    TIMER1_CTL_R = 0x00;
    TIMER1_CFG_R = 0x04;
    TIMER1_TBMR_R = 0x0A;
    TIMER1_TBILR_R= 160;
    TIMER1_TBMATCHR_R = duty;
    TIMER1_CTL_R = 0x0100;
}

void GPIOInterrupt(){
    PORTF_Interrupt = GPIO_PORTF_RIS_R & 0x11;

    NVIC_EN0_R = 0x00000000; // 30th bit controls PORTF
    GPIO_PORTF_IM_R = 0x00; // masking both switches
    if (PORTF_Interrupt == 0x01){
        GPIO_PORTF_ICR_R = 0x01;
        duty += 8;
        if (duty >= 152){ // saturating
            duty = 152;
        }
    }

    else if (PORTF_Interrupt == 0x10){
        GPIO_PORTF_ICR_R = 0x10;
            duty -= 8;
            if (duty <= 0){ // saturating
                duty = 0;
            }
        }


//    GPIO_PORTF_DATA_R ^= 0x02;


}
