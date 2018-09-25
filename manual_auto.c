



void manual_mode(void)
{flg_manual=1;
 disable_interrupts(INT_TIMER2);
   while(flg_manual)
   {
      output_toggle(pin_c7);
      //CurrentTick = get_ticks();
      set_adc_channel( 2 );
      delay_us(10);
      value = read_adc();
  
      VALUEA= value;
      valuea*=35;
      valuea/=10;
        
         CALL_DAC();
         
     //output_toggle(pin_c7);
     if(sw_mode==1)flg_manual=0;
     
   }
   enable_interrupts(INT_TIMER2);
}
