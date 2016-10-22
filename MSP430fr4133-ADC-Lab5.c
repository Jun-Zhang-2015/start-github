/**************************************************
 * EE318 Group2 Elevator Demonstrator
 * Jun Zhang
 *
 * Copyright 2015 University of Strathclyde
 **************************************************/
#include <msp430.h>
#include <driverlib.h>
#include "LedDriver.h"
#include "adc.h"


int SW1_interruptFlag_=1;
int SW2_interruptFlag_ ;
int sample;
unsigned char dialValue = 0x01;

//#pragma vector = PORT1_VECTOR                         // Lab 3
//__interrupt void P1_ISR(void)
//{
//   switch(__even_in_range(P1IV,P1IV_P1IFG7))
//  {
//     case P1IV_P1IFG3: //It is SW1
//      	SW1_interruptFlag_ = -SW1_interruptFlag_;
//    	GPIO_clearInterrupt(GPIO_PORT_P1, GPIO_PIN3);
//      break;
//
//      case P1IV_P1IFG4:  //It is SW2
//    	SW2_interruptFlag_ = 1;
//    	GPIO_clearInterrupt(GPIO_PORT_P1, GPIO_PIN4);
//      break;
//  }
//}                                                      //

/**
Initialise Timer , setting GPIO parameters
*/
#define PWMprd 20   //period of timer interrupt = 5.12 ms
#define PWMduty 1    //duty period =75%    (20-5)/20 = 1-23%


/*General Timer Interrupt Routine.*/
 #pragma vector = TIMER0_A0_VECTOR  //On the compare of TA0CCTL0
 __interrupt void TIMERA0_ISR0(void) //Flag cleared automatically
 {
  // detect new event(new buttons)
   P1SEL0 |= GPIO_PIN7; //Connect to timer     `
   P1DIR |= GPIO_PIN7 +GPIO_PIN1+GPIO_PIN3+GPIO_PIN4 +GPIO_PIN5; //Set as an output
   GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN3 | GPIO_PIN4 );
   GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN5);
 }



//// ADC interrupt service routine
//#pragma vector=ADC_VECTOR           // these two lines are used by IAR and CCC
//__interrupt void ADC_ISR(void)
//{
//  switch(__even_in_range(ADCIV,ADCIV_ADCIFG))
//  {
//    case ADCIV_ADCIFG:              // conversion complete
//        {
//         sample = ADCMEM0;
//         break;
//        }
//  }
//  ADC_clearInterrupt(ADC_BASE, ADC_COMPLETED_INTERRUPT);
//}
//
//
//
//  //On the compare of TA0CCTL0
// #pragma vector = TIMER0_A0_VECTOR
// __interrupt void TIMERA0_ISR0(void) //Flag cleared automatically
// {
////    ADCCTL0 |= ADCENC|ADCSC;   //ADC enable conversion;ADC start conversion. Software-controlled sample-and-conversion start
//   ADC_startConversion(ADC_BASE, ADC_REPEATED_SINGLECHANNEL);
//
//    if (SW1_interruptFlag_==1)          //anticlockwise
//    {
//      dialValue = dialValue * 0x02;
//
//      if(0x00 == dialValue)
//         dialValue = 0x01;
//    }
//      //if(SW1_interruptFlag_==-1)
//    else                                //clockwise
//    {
//      dialValue = dialValue / 0x02;
//
//      if(0x00 == dialValue)
//         dialValue = 0x80;
//    }
//
//     //Set value
//     setLedDial(dialValue);
//
//     //Refresh display (10 times for each position)
//       refreshLedDial();
//
//     TA0CCR0= 0x0400 + 0x000f * sample;
// }
//
//
////Timer general interrupt
// #pragma vector = TIMER0_A1_VECTOR      //drive ADC interrupt
//
// __interrupt void TIMERA0_ISR1(void)
// {
//
// switch(__even_in_range(TA0IV,10)) //Clears the flag
//  {
//    case 2:
//      {
//        ADCCTL0 |= ADCENC|ADCSC;   //ADC enable conversion;ADC start conversion. Software-controlled sample-and-conversion start
//        sample = ADCMEM0;
//        break;
//      }
//    Timer_A_clearTimerInterrupt(TIMER_A0_BASE);
//  }
//
// }


int main(void)
{
  //Default MCLK = 1MHz

  WDTCTL = WDTPW | WDTHOLD;               // Stop watchdog timer

  PWM_setUp();

  __enable_interrupt();

  // Disable the GPIO power-on default high-impedance mode
  // to activate previously configured port settings
  PMM_unlockLPM5();

  //ADC P8.1 A9                                                                                         //Lab4,Lab5
  GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P8, GPIO_PIN1, GPIO_PRIMARY_MODULE_FUNCTION);

//     SYSCFG2|=ADCPCTL9;        //ADC input A9 pin select
//
//     ADCMCTL0|=ADCINCH_9;      //Input channel select 1001b=A9
//
//     ADCCTL0|=ADCSHT_2|ADCON;  //0001b = 8 ADCCLK cycles; ADCON modified resetted when ADCENC=0 0b off;1b on
//
//     ADCCTL1|=ADCSHP;  // sample and hold input signal;clock divider;ADC clock source select
//
//     ADCCTL2|=ADCRES;    //ADC resolution; ADC sampling rate(buffering);
//
//     ADC_clearInterrupt(ADC_BASE, ADC_COMPLETED_INTERRUPT);
//     ADC_enableInterrupt(ADC_BASE, ADC_COMPLETED_INTERRUPT);

   ADC_init(ADC_BASE,
            ADC_SAMPLEHOLDSOURCE_SC,
            ADC_CLOCKSOURCE_ADCOSC,
            ADC_CLOCKDIVIDER_1);
   ADC_enable(ADC_BASE);
   ADC_enableInterrupt(ADC_BASE,ADC_COMPLETED_INTERRUPT);
   ADC_clearInterrupt(ADC_BASE,ADC_COMPLETED_INTERRUPT);
   ADC_setupSamplingTimer(ADC_BASE, ADC_CYCLEHOLD_4_CYCLES, ADC_MULTIPLESAMPLESDISABLE);
   ADC_configureMemory(ADC_BASE,
                       ADC_INPUT_A9,
                       ADC_VREFPOS_AVCC,
                       ADC_VREFNEG_AVSS);


  //Switch 1 2                                                                                   //Lab 3
   GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P1, GPIO_PIN3|GPIO_PIN4);
   GPIO_selectInterruptEdge(GPIO_PORT_P1, GPIO_PIN3|GPIO_PIN4, GPIO_LOW_TO_HIGH_TRANSITION);
   GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P1, GPIO_PIN3|GPIO_PIN4);

   GPIO_clearInterrupt(GPIO_PORT_P1, GPIO_PIN3|GPIO_PIN4);
   GPIO_enableInterrupt(GPIO_PORT_P1, GPIO_PIN3|GPIO_PIN4);

//   //TimerA                                                                        //Lab5
//
//     TA0CTL |= TASSEL_1; //32k ACLK clock  Timer_A clock source select 01b = ACLK
//     TA0CTL |= ID_1;     //Divide by 2 Input divider. These bits along with the TAIDEX bits select the divider for the input clock
//     TA0CTL |= MC_1;     //Count up  Mode control
//     TA0CTL |= TACLR;    //Timer_A clear The TACLR bit is automatically reset and always reads as zero
//     TA0CTL |=TAIE;      //Timer_A interrupt enable.
//
//     TA0CCTL1 |= OUTMOD_4; //Toggle  Output mode. Modes 2, 3, 6, and 7 are not useful for TAxCCR0 because EQUx = EQU0.
//
//     TA0CCTL0 |= CCIE;
//     TA0CCTL1 |= CCIE;  //Capture/compare interrupt enable. This bit enables the interrupt request of the corresponding CCIFG flag.
//
//     TA0CCR0 = 0x0900;
//     TA0CCR1 = 0x0800;   //2048  8hz

//     ADCCTL0 |= ADCENC|ADCSC;
//    ADC_startConversion(ADC_BASE, ADC_REPEATED_SINGLECHANNEL);
//
//     P1SEL0 |= GPIO_PIN7; //Connect to timer     `
//     P1DIR |= GPIO_PIN7; //Set as an output
//
  __enable_interrupt();

  while(1)
  {
   // ADC_startConversion(ADC_BASE, ADC_REPEATED_SINGLECHANNEL);

  }
}
