//pid bldc spindle
#include <16F1618.h>
#fuses NOWDT
#device ADC=10
#use delay(internal=32MHz)



#use timer(timer=2, tick=1ms, bits=16, ISR)

//Set the sample frequency
#define SAMPLE_FREQ 1000 //50ms sample period

//Set the PID Kp, Ki and Kd values
#define KP_VALUE 5
#define KI_VALUE 10
#define KD_VALUE 0

#define  SCK pin_b4
#define  SDI pin_b5
#define  LOAD pin_b7
#DEFINE  CS   PIN_b6
//#define sw_manual pin_c6/////////////////////select mode//////////////

int8 trisc;
#locate trisc = 0x08e
#bit trisc6   = trisc.6

int8 portc;
#locate portc=0x00e
#bit sw_mode = portc.6


//unsigned int16 a,b,c;
   pid_struct_t PIDOutput;
   
   unsigned int16 ADCReading;
   unsigned int16 CurrentTick, PIDTick;
   unsigned int16 SetPoint;
   signed int16 K1, K2, K3;
   unsigned int16 PWMDuty;
  
   unsigned int16 DATA_A;
   unsigned int16 value;
   unsigned  INT16 VALUEA;
   /////////////////////////////////var select mode///////////////////
   int1 flg_manual=0;
   //unsigned int8 loop_chk=0;


unsigned int16 GetTickDifference(unsigned int16 Current, unsigned int16 Previous)
{
   return(Current-Previous);
}

void CalculateKValues(int16 Kp,int16 Ki,int16 Kd,int16 &K1,int16 &K2,int16 &K3)
{
   K1 = Kp + Ki/SAMPLE_FREQ + Kd*SAMPLE_FREQ;
   K2 = -(Kp + (2*Kd)*SAMPLE_FREQ);
   K3 = Kd*SAMPLE_FREQ;
}
void writ_DAC_A(VOID);
VOID CALL_DAC(VOID);
void manual_mode(void);
#include "manual_auto.c"
void main()
{
   //char c;

   
   //100 ms delay for PLL to stablize
   delay_ms(100);
   
   enable_interrupts(GLOBAL);
      
   //Calculate PID Module K1, K2 and K3 values
   CalculateKValues(KP_VALUE, KI_VALUE, KD_VALUE, K1, K2, K3);
   
   //printf("\r\nK1=%ld, K2=%ld, K3=%ld", K1, K2, K3);
   
   //Setup ADC 
   setup_adc_ports(sAN2|san5,vss_vdd);
   setup_adc(ADC_CLOCK_INTERNAL);
   set_adc_channel(0);
   
   //Setup the PID module for PID mode and set the K1, K2 and K3 values
   setup_pid(PID_MODE_PID, K1, K2, K3);
   
   //Set the initial PID set point
   SetPoint = 70;   //~2.5 V
   setup_timer_5(T5_EXTERNAL | T5_DIV_BY_1);// get process variable
   //Setup PWM 3 to generate a 1 kHz signal
   setup_timer_4(T4_CLK_INTERNAL | T4_DIV_BY_32, 249, 1);   //1ms period
   set_pwm3_duty(0); //0% duty
   setup_pwm3(PWM_ENABLED | PWM_OUTPUT | PWM_TIMER4);
   

    trisc6=1;

   
   while(TRUE)
   {  //SetPoint = 30; 
      CurrentTick = get_ticks();
      set_adc_channel( 2 );
      value = read_adc();
      
      //if(value<100)value=100;
      if(value>900)value=900; 
      
       SetPoint=value;   
      
      
      if(GetTickDifference(CurrentTick, PIDTick) >= ((unsigned int16)TICKS_PER_SECOND / SAMPLE_FREQ))
      {
          
          if(sw_mode==0)manual_mode();
          
          output_toggle(pin_c7);          
          set_adc_channel( 5 );
          delay_us(5);
          ADCReading = read_adc();
        

         //ADCReading = get_timer5();
         //set_timer5(0);
         pid_get_result(SetPoint, ADCReading, &PIDOutput);
         
         PIDOutput.u &= 0x07;
         
         if(PIDOutput.u >= 4)          //PIDOutput is negative, set PWMDuty to Minimum
            PWMDuty = 0;         
         else if(PIDOutput.u != 0)     //PIDOutput > Maximum, set PWMDuty to Maximum
           PWMDuty = 3500;
        else if(PIDOutput.l > 3500)   //PIDOutput > Maximum, set PWMDuty to Maximum
            PWMDuty = 3500;
         else
            PWMDuty = PIDOutput.l;
       
         //set_pwm3_duty(PWMDuty);
         VALUEA= PWMDuty;
        // VALUEA= 2500;
         CALL_DAC();
         
         PIDTick = CurrentTick;
      }
      
      /*if(kbhit())
      {
         c = toupper(getc());
         
         if(c == 'S')
         {
            printf("\r\nNew Set Point (0-5V): ");
            
            SetPoint = (unsigned int16)(get_float() * 1000) / 5;
         }
      }*/
   }
}  
void writ_DAC_A(VOID)
{int8 i;
  output_HIGH(CS);
  
  DATA_A+=VALUEA;
  //DATA_A= 2000;
  
DELAY_US(10);
DATA_A&=0X7FFF;


output_low(CS);
output_low(SCK);
OUTPUT_HIGH(LOAD);


for(i=0;i<16;++i)
   {
     output_bit(SDI,shift_left(&DATA_A,2,0));

     DELAY_US(1);
     
     output_high(SCK);
     delay_us(5);
     output_low(SCK);
     delay_us(5);


   }
     output_LOW(LOAD);
     delay_us(5);
     output_HIGH(LOAD);
     DELAY_US(1);OUTPUT_LOW(CS);DELAY_US(10);
}
VOID CALL_DAC(VOID)
{
DATA_A=0XF000;


 
writ_DAC_A(); 


//DATA_B=0XFEF0;



output_HIGH(CS);
//writ_DAC_B();
}
