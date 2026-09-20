/*******************************************************************************
* File Name:   main.c
*
* Description: This is the main file for the PPCA CPU core 0. This file contains
* the main function for the core, interrupt service routine, and global variables
* used by these functions.
*
* Related Document: See README.md
*
*
********************************************************************************
* (c) 2026, Infineon Technologies AG, or an affiliate of Infineon
* Technologies AG. All rights reserved.
* This software, associated documentation and materials ("Software") is
* owned by Infineon Technologies AG or one of its affiliates ("Infineon")
* and is protected by and subject to worldwide patent protection, worldwide
* copyright laws, and international treaty provisions. Therefore, you may use
* this Software only as provided in the license agreement accompanying the
* software package from which you obtained this Software. If no license
* agreement applies, then any use, reproduction, modification, translation, or
* compilation of this Software is prohibited without the express written
* permission of Infineon.
*
* Disclaimer: UNLESS OTHERWISE EXPRESSLY AGREED WITH INFINEON, THIS SOFTWARE
* IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
* INCLUDING, BUT NOT LIMITED TO, ALL WARRANTIES OF NON-INFRINGEMENT OF
* THIRD-PARTY RIGHTS AND IMPLIED WARRANTIES SUCH AS WARRANTIES OF FITNESS FOR A
* SPECIFIC USE/PURPOSE OR MERCHANTABILITY.
* Infineon reserves the right to make changes to the Software without notice.
* You are responsible for properly designing, programming, and testing the
* functionality and safety of your intended application of the Software, as
* well as complying with any legal requirements related to its use. Infineon
* does not guarantee that the Software will be free from intrusion, data theft
* or loss, or other breaches ("Security Breaches"), and Infineon shall have
* no liability arising out of any Security Breaches. Unless otherwise
* explicitly approved by Infineon, the Software may not be used in any
* application where a failure of the Product or any consequences of the use
* thereof can reasonably be expected to result in personal injury.
*******************************************************************************/

/*******************************************************************************
* Header Files
*******************************************************************************/
#include "cy_pdl.h"
#include "cycfg.h"
    
/******************************************************************************
* Macros
*******************************************************************************/
/* Shared memory addresses in M4 shared memory space (0x20040000-0x20043FFF) */
/* All cores can access M4 shared memory for inter-core communication */
#define PPCA_CPU0_M4_VAR_ADDRESS 0x20040400  /* Used by this core (CPU0) */


#define TIMER_CLK_FREQ_MHz 200

#define CONV_CYCLES_TO_NS(x) (((x) * 1000) / TIMER_CLK_FREQ_MHz)

/*******************************************************************************
* Global Variables
*******************************************************************************/
uint32_t *adc_ch0_read_data = (uint32_t *)PPCA_CPU0_M4_VAR_ADDRESS;
uint32_t *adc_ch1_read_data = (uint32_t *)PPCA_CPU0_M4_VAR_ADDRESS + 1;
uint32_t *adc_ch2_read_data = (uint32_t *)PPCA_CPU0_M4_VAR_ADDRESS + 2;
uint32_t *adc_ch3_read_data = (uint32_t *)PPCA_CPU0_M4_VAR_ADDRESS + 3;
uint32_t *adc_ch4_read_data = (uint32_t *)PPCA_CPU0_M4_VAR_ADDRESS + 4;
uint32_t *adc_ch5_read_data = (uint32_t *)PPCA_CPU0_M4_VAR_ADDRESS + 5;
uint32_t *adc_ch6_read_data = (uint32_t *)PPCA_CPU0_M4_VAR_ADDRESS + 6;
uint32_t *adc_ch7_read_data = (uint32_t *)PPCA_CPU0_M4_VAR_ADDRESS + 7;
uint32_t *adc_conv_time_ns = (uint32_t *)PPCA_CPU0_M4_VAR_ADDRESS + 8;
uint32_t *adc_conv_status = (uint32_t *)PPCA_CPU0_M4_VAR_ADDRESS + 9;

cy_stc_sysint_t pwm_intr_config =
{
    .intrSrc = START_TRIG_PWM_IRQ,
    .intrPriority = 1U,
};

uint32_t adc_conv_cycles = 0;

/*******************************************************************************
* Function Prototypes
*******************************************************************************/
void pwm_compare_isr(void);

/*******************************************************************************
* Function Definitions
*******************************************************************************/

/*******************************************************************************
* Function Name: main
*********************************************************************************
* Summary:
* This is the main function for PPCA CPU 0. It performs the initialization of the
* variables used in the code, and initializes and enables the interrupt from the
* PPCA timer required to interrupt this CPU.
*
* Parameters:
*  void
*
* Return:
*  int
*
*******************************************************************************/
int main(void)
{
    cy_en_sysint_status_t status;

    *adc_ch0_read_data = 0;
    *adc_ch1_read_data = 0;
    *adc_ch2_read_data = 0;
    *adc_ch3_read_data = 0;
    *adc_ch4_read_data = 0;
    *adc_ch5_read_data = 0;
    *adc_ch6_read_data = 0;
    *adc_ch7_read_data = 0;
    *adc_conv_time_ns  = 0;
    *adc_conv_status = 0;

    /* Initializing the timer interrupt */
    status = Cy_SysInt_Init(&pwm_intr_config, &pwm_compare_isr);

    /* Interrupt initialization failed. */
    if(CY_SYSINT_SUCCESS != status)
    {
        CY_ASSERT(0);
    }

    /* Clearing any pending interrupt. */
    NVIC_ClearPendingIRQ(pwm_intr_config.intrSrc);

    /* Enabling timer interrupt. */
    NVIC_EnableIRQ(pwm_intr_config.intrSrc);

    /* Enabling interrupts. */
    __enable_irq();

     for(;;)
     {
     }
}

/*******************************************************************************
* Function Name: pwm_compare_isr
*********************************************************************************
* Summary:
* This is the interrupt service routine for the PPCA timer used for triggering
* the ADC conversion and generate a periodic interrupt. It reads the data from
* the ADC channels, reads the time required for the conversion, and copies the
* data to the shared memory.
*
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/
void pwm_compare_isr(void)
{
    /* Clearing the PWM capture interrupt. */
    Cy_TCPWM_ClearInterrupt(START_TRIG_PWM_HW, START_TRIG_PWM_NUM, CY_TCPWM_INT_ON_TC);

    /* Reading the ADC channel data to the shared memory. */
    *adc_ch0_read_data = Cy_PPCA_ADC_Read_ADC_Data(ADC_HW, 0);
    *adc_ch1_read_data = Cy_PPCA_ADC_Read_ADC_Data(ADC_HW, 1);
    *adc_ch2_read_data = Cy_PPCA_ADC_Read_ADC_Data(ADC_HW, 2);
    *adc_ch3_read_data = Cy_PPCA_ADC_Read_ADC_Data(ADC_HW, 3);
    *adc_ch4_read_data = Cy_PPCA_ADC_Read_ADC_Data(ADC_HW, 4);
    *adc_ch5_read_data = Cy_PPCA_ADC_Read_ADC_Data(ADC_HW, 5);
    *adc_ch6_read_data = Cy_PPCA_ADC_Read_ADC_Data(ADC_HW, 6);
    *adc_ch7_read_data = Cy_PPCA_ADC_Read_ADC_Data(ADC_HW, 7);

    /* Reading the cycle count required for the conversion. */
    adc_conv_cycles = Cy_TCPWM_Counter_GetCapture0Val(ADC_CAPTURE_COUNTER_HW, ADC_CAPTURE_COUNTER_NUM);

    /* Calculating the conversion time in nano seconds. */
    *adc_conv_time_ns = CONV_CYCLES_TO_NS(adc_conv_cycles);

    /* Informing main core arrival new data. */
    *adc_conv_status = 1;
}
