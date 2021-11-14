#include "mbed.h"  
#include "FastPWM.h"  
  
#define ERROR 100000    //Required variation to previous power before slope is complemented.  
#define AVERAGES 1000   //number of samples for averaging  
#define DEBUG 0         //Turn debug on/off  
  
Serial pc(USBTX, USBRX, 115200);          //USB serial communication  
  
AnalogIn voltage(A0);                     //Voltage input on pin A0  
AnalogIn current(A1);                     //Current input on pin A1  
FastPWM pwm(D6);                          //PWM output on pin D6  
  
unsigned int prev_power, current_power;  //previous and current power variables  
float dutycycle = 0.1f;                  //starting duty cycle  
float alpha = 0.004f;                    //perturb step size  
int slope = 1;                           //Initial slope  
  
/*Function prototypes*/  
void getValues(void);  
void sweep(void);  
  
int main()  
{      
    //Initialise the switching frequency to 100KHz  
    pwm.period_us(10);  
      
    //Initialise the duty cycle  
    pwm.write(dutycycle);  
      
    //Initialise current power to 0  
    current_power = 0;  
      
    //If debug is on, call the duty cycle sweep function  
    if(DEBUG) sweep();  
      
    //---Program loop---  
    while(1)  
    {             
        //Store previous results          
        prev_power = current_power;  
          
        //Calcualte current power  
        getValues();              
          
        //Determine the difference  
        int difference = current_power - prev_power;       
          
        if ((difference > 0) && (abs(difference) > ERROR)) //current power>prev power  
        {  
            if(slope > 0) //Maintain climbing  
            {  
                dutycycle += alpha;  
                slope = 1;  
            }  
            else //Reverse direction  
            {  
                dutycycle -= alpha;  
                slope = -1;  
            }  
        }   
        else if ((difference < 0) && (abs(difference) > ERROR))//prev power>current power  
        {  
            if(slope < 0) //Reverse direction  
            {  
                dutycycle += (alpha);  
                slope = 1;  
            }  
            else //Maintain climbing  
            {  
                dutycycle -= alpha;  
                slope = -1;  
            }      
        }   
        //Limit the duty cycle within 2-98%  
        if(dutycycle>0.98) dutycycle = 0.98f;  
        if(dutycycle<0.02) dutycycle = 0.02f;        
          
        //Update the duty cycle  
        pwm.write(dutycycle);  
          
        //wait for stability  
        wait_ms(50);  
    }  
    //---Program loop end---      
}   
  
void getValues(void)  
{      
    //Read new adc values and average     
    unsigned int avg_data[2]={0};  
    for(int i=0;i<AVERAGES;i++)  
    {  
        avg_data[0] += voltage.read_u16();  
        avg_data[1] += current.read_u16();  
    }     
    unsigned int PA1_DATA = (avg_data[0] / AVERAGES);  
    unsigned int PA2_DATA = (avg_data[1] / AVERAGES);  
      
    //Calculate power of new sample      
    current_power = (PA1_DATA*PA2_DATA);  
      
    //Output data if debugging  
    if(DEBUG)  
    {  
        pc.printf("avg_data[0]: %d\n\r", avg_data[0]);  
        pc.printf("avg_data[1]: %d\n\r", avg_data[1]);  
        pc.printf("PA1_DATA: %d\n\r", PA1_DATA);  
        pc.printf("PA2_DATA: %d\n\r", PA2_DATA);  
        pc.printf("Power: %d\n\r", current_power);  
    }  
}  
  
//Sweep the duty cycle. Useful for finding which duty cycle -> MPP  
void sweep(void)  
{      
    //Loop from 1-99% in steps of 1%  
    pc.printf("Duty Cycle,Power\n\r");  
    for(double i=0.01; i<1; i=i+0.01)  
    {  
        //Set the varying duty cycle  
        pwm.write(i);  
          
        //Get averages current power  
        getValues();  
          
        //Print duty cycle and power to terminal  
        pc.printf("%f,%d\n\r", i, current_power);    
        wait_ms(100);           
    }      
}  
