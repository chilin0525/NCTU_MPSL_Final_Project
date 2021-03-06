#include "stm32l476xx.h"
#include <math.h>
#include <assert.h>

extern void GPIO_init();
extern void max7219_init();
extern void MAX7219Send(int address,int data);
extern void MUTIDISPLAY();
extern void DELAY();

void DISPLAY_init(){
    int i=1;
    for(;i<=8;i++) MAX7219Send(i<<8,0xF);
    MAX7219Send(3<<8,1<<7);
    MAX7219Send(2<<8,0);
    MAX7219Send(1<<8,0);
}

void DISPLAY_TIME(int TIM_INT,int TIM_FLO){
    int len = 0;
    while(TIM_INT != 0){
        if(len==0) MAX7219Send((len+3)<<8,(1<<7) + TIM_INT%10);
        else MAX7219Send((len+3)<<8,TIM_INT%10);
        TIM_INT /= 10;
        ++len;
    }
    MAX7219Send(2<<8,TIM_FLO/10);
    MAX7219Send(1<<8,TIM_FLO%10);
}

void Timer_init(){
    //RCC->APB1ENR1 |= RCC_APB1ENR1_TIM1EN;    // enable TIM1
    RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;    // enable TIM1
    TIM1->CR1 = 0;              // upcounter, disable counter, enable UEV(update event)
    TIM1->CNT = 0;
    TIM1->PSC = (uint32_t)9;  // 4M == 4000000/10 = 400000
    TIM1->ARR = (uint32_t)399999;
}

void Timer_start(int TIME){
    TIM1->ARR = TIME;
    TIM1->EGR = 0x0001;    // re init cnt
    TIM1->SR &= 0xFFFFFFFE;     // set to 0
    TIM1->CR1 |= 0x00000001;    // enable counter
}

void INIT_DHT(){
    GPIOC->MODER |= 0b01;       // set as output mode
    GPIOC->PUPDR |= 0b01;
    GPIOC->ODR &= 0xfffffffe;

    Timer_start(7200);          // delay for 18ms <==> 7200
    while(!(TIM1->SR & 0x00000001)){}
    TIM1->SR &= 0xFFFFFFFE;

    GPIOC->MODER &= 0xfffffffC; // set as input mode
}

int DHT_RESPONCSCE(){
    int flag1=0,flag2=0;
    Timer_start(16);
    while(!(TIM1->SR & 0x00000001)){}
    TIM1->SR &= 0xFFFFFFFE;
    flag1 = !(GPIOC->IDR & 1);

    if(flag1){
        Timer_start(32);
        while(!(TIM1->SR & 0x00000001)){}
        TIM1->SR &= 0xFFFFFFFE;
        flag2 = (GPIOC->IDR & 1);
    }
    while(GPIOC->IDR&1){}

    return flag2;
}

void DELAY_TIME(int TIME){
    int i=TIME;
    while(i){i-=5;}
}

int DHT_READ(void){
    int i=0,j;
    for(j=0;j<8;j++){
        while(!(GPIOC->IDR & 1)){}          // if pc0 ==0 waiting

        Timer_start(16);
        while(!(TIM1->SR & 0x00000001)){}
        TIM1->SR &= 0xFFFFFFFE;

        if(!(GPIOC->IDR & 1)){
			i&= ~(1<<(7-j));   // write 0
        } else {
            i|= (1<<(7-j));
        }

        while(GPIOC->IDR & 1){}             // if pc0 ==1 waiting
    }
    return i;
}

void DHT_DISPLAY(int RH_Int,int RH_Dec,int Temp_Int,int Temp_Dec){
    int sum = RH_Int;
    sum *= 100;
    sum += RH_Dec;
    sum *= 100;
    sum += Temp_Int;
    sum *= 100;
    sum += Temp_Dec;
    MUTIDISPLAY(sum);
}

int main(){
    GPIO_init();
    max7219_init();
    DISPLAY_init();

    Timer_init();

    while(1){
        INIT_DHT();
        int flag = DHT_RESPONCSCE();

        if(flag){
            int RH_Int_Data = DHT_READ();
            int RH_Dec_Data = DHT_READ();
            int Temp_Int_Data = DHT_READ();
            int Temp_Dec_Data = DHT_READ();
            int CheckSum = DHT_READ();
            //DHT_DISPLAY(RH_Int_Data,RH_Dec_Data,Temp_Int_Data,Temp_Dec_Data);
            int sum = RH_Dec_Data+RH_Int_Data+Temp_Dec_Data+Temp_Int_Data;
            sum = sum & 0xff;

            //DHT_DISPLAY(RH_Int_Data,RH_Dec_Data,Temp_Int_Data,Temp_Dec_Data);

            if(sum!=CheckSum) {
                //MUTIDISPLAY(7414);
                //DHT_DISPLAY(RH_Int_Data,RH_Dec_Data,Temp_Int_Data,Temp_Dec_Data);
            } else {
                DHT_DISPLAY(RH_Int_Data,RH_Dec_Data,Temp_Int_Data,Temp_Dec_Data);
            }

            Timer_start(20000);
            while(!(TIM1->SR & 0x00000001)){}
            TIM1->SR &= 0xFFFFFFFE;

            /*
            MUTIDISPLAY(Temp_Int_Data);


            MUTIDISPLAY(RH_Dec_Data);
            Timer_start(400000);
            while(!(TIM2->SR & 0x00000001)){}
            TIM2->SR &= 0xFFFFFFFE;
            MUTIDISPLAY(Temp_Int_Data);
            Timer_start(400000);
            while(!(TIM2->SR & 0x00000001)){}
            TIM2->SR &= 0xFFFFFFFE;
            MUTIDISPLAY(Temp_Dec_Data);
            Timer_start(400000);
            while(!(TIM2->SR & 0x00000001)){}
            TIM2->SR &= 0xFFFFFFFE;
            MUTIDISPLAY(CheckSum);
            Timer_start(400000);
            while(!(TIM2->SR & 0x00000001)){}
            TIM2->SR &= 0xFFFFFFFE;
            */
        }
    }

/*
    while(1){
        Timer_start(399999*2);
        MUTIDISPLAY(tmp);
        while(!(TIM2->SR & 0x00000001)){}
        TIM2->SR &= 0xFFFFFFFE;
        tmp+=1;
    }
*/
    return 0;
}
