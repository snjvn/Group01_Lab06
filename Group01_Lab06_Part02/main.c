
#include <stdint.h>
#include <stdbool.h>
#include "tm4c123gh6pm.h"
/**
 * main.c
 */

void GPIOInterrupt(void);
void SystickInterrupt(void);
void INIT_SYS_CTRL_REGISTERS(void);
void INIT_GPIO_PORTF_REGISTERS(void);
void INIT_TIMER1_REGISTERS(void);

uint32_t PORTF_Interrupt = 0x00;
uint32_t SW_State = 0x00;
int duty = 80; // init, 50% duty cycle
int main(void)
{
    INIT_SYS_CTRL_REGISTERS(); // init system control registers
    INIT_GPIO_PORTF_REGISTERS();
    INIT_TIMER1_REGISTERS();

    while(1){
        NVIC_EN0_R = 0x40000000; // 30th bit controls PORTF
        GPIO_PORTF_IM_R = 0x01; // unmasking one switch
//        GPIO_PORTF_CR_R = 0x00;
        TIMER1_TBMATCHR_R = duty;// update the compar-value of TIMER1-B
    }

    return 0;
}

void INIT_SYS_CTRL_REGISTERS(){
    SYSCTL_RCGC2_R |= 0x00000020;       /* enable clock to GPIOF */
    SYSCTL_RCGCTIMER_R = 0x02; // enable clock to General Purpose Timer 1 Module
    SYSCTL_RCGCGPIO_R = 0x20; // enable clock to PORTF GPIO
}

void INIT_GPIO_PORTF_REGISTERS(){
    GPIO_PORTF_LOCK_R = 0x4C4F434B;     /* unlock commit register */
    GPIO_PORTF_CR_R = 0x1F;             /* make PORTF configurable */
    GPIO_PORTF_DEN_R = 0x0F;            /* set PORTF pins 4 : 0 pins */
    GPIO_PORTF_DIR_R = 0x0E;            /*  */
    GPIO_PORTF_PUR_R = 0x01;            /* PORTF0 and PORTF4 are pulled up */
    GPIO_PORTF_AFSEL_R = 0x08; // Select PORTF3 (Green LED) for Alternate Function:
                               // Green LED not driven by GPIO_PORTF_DATA_R but instead by T1CCP1 (PWM Signal from GPT1)
    GPIO_PORTF_PCTL_R = 0x00007000; // connects the PWM output of GPTM1 to PORTF3

    NVIC_EN0_R = 0x40000000; // 30th bit controls PORTF
    GPIO_PORTF_IS_R = 0x00; // interrupt sensitivity - edge
    GPIO_PORTF_IEV_R = 0x00; // GPIO Interrupt triggered at negative edge from Pulled-Up Switch
    GPIO_PORTF_IM_R = 0x01; // unmasking one switch (SW2)
}

void INIT_TIMER1_REGISTERS(){
    TIMER1_CTL_R = 0x00; // make sure TIMER1 is disabled before configuring
    TIMER1_CFG_R = 0x04; // configures the timer in 16-bit mode
    TIMER1_TBMR_R = 0x0A; // configure the timer in periodic timer mode, with PWM mode enabled
    TIMER1_TBILR_R= 160; // the reload value to achieve 10us (assuming 16MHz clock)
    TIMER1_TBMATCHR_R = duty; // the compare value for the timer
    TIMER1_CTL_R = 0x0100; // enable Timer B (the timer which we're using)
}

void INIT_SYSTICK(){ // initializing systick
    NVIC_ST_RELOAD_R = 16000*500; // 500 ms
    NVIC_ST_CURRENT_R = 0x00;
    NVIC_ST_CTRL_R = 0x00000007;
}

void SystickInterrupt(){
    SW_State = (GPIO_PORTF_DATA_R & 0x01); // read the state of switch
    if (SW_State == 0x00){ // if it is still pressed, brighten
        duty -= 8;
        if (duty <= 0){ // saturating
            duty = 0;
        }
    }

    else{ // if it is released, stop systick- no need to brighten anymore
        NVIC_ST_CURRENT_R = 0x00;
        NVIC_ST_CTRL_R = 0x00000000;
    }
}

void GPIOInterrupt(){
    PORTF_Interrupt = GPIO_PORTF_RIS_R & 0x01; // read which switch caused the interrupt


    INIT_SYSTICK(); // in case this interrupt turns out to be a long-press

    // for debouncing
    NVIC_EN0_R = 0x00000000; // 30th bit controls PORTF
    GPIO_PORTF_IM_R = 0x00; // masking both switches
    if (PORTF_Interrupt == 0x01){ // switch was pressed, reduce brightness
        GPIO_PORTF_ICR_R = 0x01; // for edge-triggered interrupts, necessary to clear the interrupt status
        duty += 8;
        if (duty >= 152){ // saturating
            duty = 152;
        }
    }



}
