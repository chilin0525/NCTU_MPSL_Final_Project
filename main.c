#include "stm32l476xx.h"
#include <math.h>
#include <assert.h>
int pwm;
int time;
int DELAY=200;
int DELAY1=50;
int previous;
int press=0;
int pulse[168];
int alarm_mode=0;
int alarm_on=0;

int alarm_time=0;

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

    Timer_start(7300);          // delay for 18ms <==> 7200
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




}
//#########################################################
void GPIO_init()
{
	RCC->AHB2ENR=0b111;

	GPIOA->MODER&=0b11111111111111111111001111111100;
	GPIOA->MODER|=0b00000000000000000000100000000010;
	GPIOA->AFR[0]&=0xFF0FFFF0;
	GPIOA->AFR[0]|=0x00100002;


	GPIOC->MODER=0b0101010100010100;
	GPIOC->ODR=0;

	GPIOB->MODER &=0b00000000000000000000000000000000;
	GPIOB->MODER |=0b01010101010101010101010101010101;
	GPIOB->OTYPER =0b0000000000000000;

	GPIOB->OSPEEDR=0;
	GPIOB->ODR    =0;
}
void NVIC_config()
{
	NVIC_EnableIRQ(EXTI3_IRQn);
}

void EXTI_config()
{
    RCC->APB2ENR|=1;
    SYSCFG->EXTICR[0]=0b0010000000000000;
    EXTI->IMR1=0b1000;
    EXTI->FTSR1=0b1000;
}
//#########################################################
void SysTick_Handler(void)
{
	//if(alarm_mode==0)
	alarm_mode++;

	if(alarm_mode==4){alarm_mode=0;SysTick->LOAD = 500000;}
	else {SysTick->LOAD = 100000;}

	if(alarm_on==0){TIM2->ARR=0;}
	else if(alarm_mode==1){TIM2->ARR=0;}
	else if(TIM2->ARR==0){TIM2->ARR=400000;}
	else {TIM2->ARR=0;}
}
void EXTI3_IRQHandler()
{
	if(TIM5->CNT>=alarm_time&&TIM5->CNT<=alarm_time+100&&alarm_on==1)SysTick->CTRL = 3;

GPIOB->ODR=0;
EXTI->IMR1=0b0000;
EXTI->FTSR1=0b0000;
TIM4->CNT=0;
int time;
int count1=0;
int count=0;
int flag=0;
int now=0;
int pre=0;
int i;

for(i=0;i<70;i++){pulse[i]=0;}

while(TIM4->CNT<10000)
{
	now=GPIOC->IDR&0b1000;
	if(now!=pre){pulse[count]=TIM4->CNT;count=count+1;}

	pre=now;

}



int t;
int n=0;

if     ((pulse[35]-pulse[34])>120&&(pulse[37]-pulse[36])<120&&(pulse[39]-pulse[38])>120&&(pulse[41]-pulse[40])>120
      &&(pulse[43]-pulse[42])<120&&(pulse[45]-pulse[44])<120&&(pulse[47]-pulse[46])>120&&(pulse[49]-pulse[48])<120)
{n=1;if(DELAY!=0)previous=DELAY; DELAY=DELAY==0?previous:0;}

else if((pulse[35]-pulse[34])<120&&(pulse[37]-pulse[36])<120&&(pulse[39]-pulse[38])>120&&(pulse[41]-pulse[40])>120
	  &&(pulse[43]-pulse[42])<120&&(pulse[45]-pulse[44])<120&&(pulse[47]-pulse[46])>120&&(pulse[49]-pulse[48])<120){n=2;if(DELAY!=0)DELAY=DELAY==50?200:50;}

else if((pulse[35]-pulse[34])>120&&(pulse[37]-pulse[36])>120&&(pulse[39]-pulse[38])<120&&(pulse[41]-pulse[40])>120
	  &&(pulse[43]-pulse[42])<120&&(pulse[45]-pulse[44])<120&&(pulse[47]-pulse[46])<120&&(pulse[49]-pulse[48])<120){n=3;}

else if((pulse[35]-pulse[34])<120&&(pulse[37]-pulse[36])<120&&(pulse[39]-pulse[38])<120&&(pulse[41]-pulse[40])<120
	  &&(pulse[43]-pulse[42])>120&&(pulse[45]-pulse[44])<120&&(pulse[47]-pulse[46])<120&&(pulse[49]-pulse[48])<120){n=4;}

else if((pulse[35]-pulse[34])>120&&(pulse[37]-pulse[36])<120&&(pulse[39]-pulse[38])>120&&(pulse[41]-pulse[40])>120
	  &&(pulse[43]-pulse[42])<120&&(pulse[45]-pulse[44])<120&&(pulse[47]-pulse[46])<120&&(pulse[49]-pulse[48])<120){n=5;}

else if((pulse[35]-pulse[34])>120&&(pulse[37]-pulse[36])<120&&(pulse[39]-pulse[38])<120&&(pulse[41]-pulse[40])<120
	  &&(pulse[43]-pulse[42])>120&&(pulse[45]-pulse[44])<120&&(pulse[47]-pulse[46])<120&&(pulse[49]-pulse[48])<120){n=6;}

else if((pulse[35]-pulse[34])<120&&(pulse[37]-pulse[36])>120&&(pulse[39]-pulse[38])>120&&(pulse[41]-pulse[40])>120
	  &&(pulse[43]-pulse[42])<120&&(pulse[45]-pulse[44])<120&&(pulse[47]-pulse[46])<120&&(pulse[49]-pulse[48])<120){n=7;}

else if((pulse[35]-pulse[34])<120&&(pulse[37]-pulse[36])>120&&(pulse[39]-pulse[38])<120&&(pulse[41]-pulse[40])>120
	  &&(pulse[43]-pulse[42])>120&&(pulse[45]-pulse[44])<120&&(pulse[47]-pulse[46])<120&&(pulse[49]-pulse[48])<120)
{SysTick->CTRL =0;alarm_on^=1;GPIOC->ODR&=0b10111111;if(alarm_on==1)GPIOC->ODR|=0b1000000;}
//else if((pulse[35]-pulse[34])<120&&(pulse[37]-pulse[36])>120&&(pulse[39]-pulse[38])>120&&(pulse[41]-pulse[40])<120
	  //&&(pulse[43]-pulse[42])<120&&(pulse[45]-pulse[44])>120&&(pulse[47]-pulse[46])<120&&(pulse[49]-pulse[48])<120){DELAY=150;}

if(n>0&&n<8){press=n;}


t=20000;
while(t--);

    EXTI->IMR1=0b1000;
    EXTI->FTSR1=0b1000;

	EXTI->PR1|=0b1000;
	NVIC_ClearPendingIRQ(EXTI3_IRQn);

	return;
}
//############################################################################
void set_time(int ttt)
{
	int time1,time2,time3,time4,time5,time6;
    int tt;
    int set=ttt/100;
	int c=0,t;
	int mode=0;
	int count=0;
    int flag=0;
    GPIOC->ODR=0b0000010;
    if(alarm_on==1)GPIOC->ODR|=0b1000000;

    while(1)
		{
		    tt=set;
		    if(TIM5->CNT>=alarm_time&&TIM5->CNT<=alarm_time+100&&alarm_on==1)SysTick->CTRL = 3;


		    //if(press==6){press=0;if(flag==1)TIM5->CNT=set*100;show_time();}
		    //if(press==4){press=0;if(flag==1)TIM5->CNT=set*100;set_alarm();}
		    if(press==5){press=0;if(flag==1)TIM5->CNT=set*100;set_alarm();}

            if(press==4){flag=1;press=0;count=0;mode=mode==0?2:mode-1;}
            if(press==6){flag=1;press=0;count=0;mode=mode==2?0:mode+1;}

	        time1=tt%10;
	        time2=(tt%60)/10;
	        tt=tt/60;

	        time3=tt%10;
	        time4=(tt%60)/10;
	        tt=tt/60;

	        time5=tt%10;
	        time6=(tt%60)/10;

	        switch(mode){
	        case 2:if(press==3){flag=1;press=0;count=0;if((set)%60==59){set-=59;}else{set+=1;}}
	               if(press==7){flag=1;press=0;count=0;if((set)%60==0){set+=59;}else{set-=1;}}
	               break;
	        case 1:if(press==3){flag=1;press=0;count=0;if((set/60)%60==59){set-=3540;}else{set+=60;}}
	               if(press==7){flag=1;press=0;count=0;if((set/60)%60==0){set+=3540;}else{set-=60;}}
	               break;
	        case 0:if(press==3){flag=1;press=0;count=0;if((set/3600)%24==23){set-=82800;}else{set+=3600;}}
	               if(press==7){flag=1;press=0;count=0;if((set/3600)%24==0){set+=82800;}else{set-=3600;}}
	               break;
	        }

	        if     (c==0&&!(mode==2&&count>1500))GPIOB->ODR=((0b0000010000000000)|(1<<time1));
	        else if(c==1&&!(mode==2&&count>1500))GPIOB->ODR=((0b0000100000000000)|(1<<time2));
            else if(c==2&&!(mode==1&&count>1500))GPIOB->ODR=((0b0001000000000000)|(1<<time3));
            else if(c==3&&!(mode==1&&count>1500))GPIOB->ODR=((0b0010000000000000)|(1<<time4));
            else if(c==4&&!(mode==0&&count>1500))GPIOB->ODR=((0b0100000000000000)|(1<<time5));
            else if(c==5&&!(mode==0&&count>1500))GPIOB->ODR=((0b1000000000000000)|(1<<time6));

			t=DELAY;
			while(t--);
	        GPIOB->ODR=0;

	        t=200-DELAY;
	        while(t--);

			t=DELAY1;
            while(t--);

	        c=c==5?0:c+1;
	        count=count==3000?0:count+1;
		}
	return;
}
void set_alarm()
{
	int time1,time2,time3,time4,time5,time6;
    int tt;
    int set=alarm_time/100;
	int c=0,t;
	int mode=0;
	int count=0;
    int flag=0;
    GPIOC->ODR=0b0000100;
    if(alarm_on==1)GPIOC->ODR|=0b1000000;

    while(1)
		{
		    tt=set;
		    if(TIM5->CNT>=alarm_time&&TIM5->CNT<=alarm_time+100&&alarm_on==1)SysTick->CTRL = 3;


		    //if(press==6){press=0;if(flag==1){alarm_on=1;alarm_time=set*100;}set_time(TIM5->CNT);}
		    //if(press==4){press=0;if(flag==1){alarm_on=1;alarm_time=set*100;}timer();}
		    if(press==5){press=0;if(flag==1){alarm_on=1;alarm_time=set*100;GPIOC->ODR|=0b1000000;}timer();}

            if(press==4){flag=1;press=0;count=0;mode=mode==0?2:mode-1;}
            if(press==6){flag=1;press=0;count=0;mode=mode==2?0:mode+1;}

	        time1=tt%10;
	        time2=(tt%60)/10;
	        tt=tt/60;

	        time3=tt%10;
	        time4=(tt%60)/10;
	        tt=tt/60;

	        time5=tt%10;
	        time6=(tt%60)/10;

	        switch(mode){
	        case 2:if(press==3){flag=1;press=0;count=0;if((set)%60==59){set-=59;}else{set+=1;}}
	               if(press==7){flag=1;press=0;count=0;if((set)%60==0){set+=59;}else{set-=1;}}
	               break;
	        case 1:if(press==3){flag=1;press=0;count=0;if((set/60)%60==59){set-=3540;}else{set+=60;}}
	               if(press==7){flag=1;press=0;count=0;if((set/60)%60==0){set+=3540;}else{set-=60;}}
	               break;
	        case 0:if(press==3){flag=1;press=0;count=0;if((set/3600)%24==23){set-=82800;}else{set+=3600;}}
	               if(press==7){flag=1;press=0;count=0;if((set/3600)%24==0){set+=82800;}else{set-=3600;}}
	               break;
	        }

	        if     (c==0&&!(mode==2&&count>1500))GPIOB->ODR=((0b0000010000000000)|(1<<time1));
	        else if(c==1&&!(mode==2&&count>1500))GPIOB->ODR=((0b0000100000000000)|(1<<time2));
            else if(c==2&&!(mode==1&&count>1500))GPIOB->ODR=((0b0001000000000000)|(1<<time3));
            else if(c==3&&!(mode==1&&count>1500))GPIOB->ODR=((0b0010000000000000)|(1<<time4));
            else if(c==4&&!(mode==0&&count>1500))GPIOB->ODR=((0b0100000000000000)|(1<<time5));
            else if(c==5&&!(mode==0&&count>1500))GPIOB->ODR=((0b1000000000000000)|(1<<time6));

			t=DELAY;
			while(t--);
	        GPIOB->ODR=0;

	        t=200-DELAY;
	        while(t--);

			t=DELAY1;
            while(t--);

	        c=c==5?0:c+1;
	        count=count==3000?0:count+1;
		}
	return;
}
void timer()
{
	int time1,time2,time3,time4,time5,time6,time_now,hour_now;
	GPIOC->ODR=0b0010000;
	if(alarm_on==1)GPIOC->ODR|=0b1000000;

	int c=0;
	TIM3->CNT=0;
	TIM3->CR1 =0;
	int mode=0;
	//int count=0;
	int t;
	int hour=0;
	int count_flag=0;
	int flag=0;
	while(1)
	{
		//time_now=TIM6->CNT;
		//if(press==4){press=0;set_time(TIM5->CNT);}
		if(TIM5->CNT>=alarm_time&&TIM5->CNT<=alarm_time+100&&alarm_on==1)SysTick->CTRL = 3;


        if(press==3&&mode==0){flag=0;mode=1;press=0;TIM3->CR1 |= TIM_CR1_CEN;}
        if(press==3&&mode==1){flag=0;mode=0;press=0;TIM3->CR1 =0;}

        if(press==6){flag=1;press=0;time_now=TIM3->CNT;hour_now=hour;}
        if(press==7){flag=0;mode=0;press=0;TIM3->CR1 =0;TIM3->CNT=0;hour=0;}

        if(press==5){flag=0;press=0;TIM3->CR1=0;TIM3->CNT=0;humid();}

		time=TIM3->CNT;
        if(time==5999&&count_flag==0){hour++;count_flag=1;}
        if(hour==60)hour=0;

        if(time==3000)count_flag=0;

        if(flag==1)time=time_now;
        time1=time%10;
        time=time/10;

        time2=time%10;
        time=time/10;

        time3=time%10;
        time4=(time%60)/10;
        time=time/60;

        if(flag==0){
        time5=hour%10;
        time6=(hour%60)/10;}

        if(flag==1){
        time5=hour_now%10;
        time6=(hour_now%60)/10;}

        if(c==0)     {GPIOB->ODR=((0b0000010000000000)|(1<<time1));}
        else if(c==1){GPIOB->ODR=((0b0000100000000000)|(1<<time2));}
        else if(c==2){GPIOB->ODR=((0b0001000000000000)|(1<<time3));}
        else if(c==3){GPIOB->ODR=((0b0010000000000000)|(1<<time4));}
        else if(c==4){GPIOB->ODR=((0b0100000000000000)|(1<<time5));}
        else if(c==5){GPIOB->ODR=((0b1000000000000000)|(1<<time6));}

		t=DELAY;
		while(t--);
        GPIOB->ODR=0;

        t=200-DELAY;
        while(t--);

		t=DELAY1;
		while(t--);

        c=c==5?0:c+1;
	}
	return;
}
void show_time()
{
	int time1,time2,time3,time4,time5,time6,time_now;

	int c=0;
	GPIOC->ODR=0b10000000;
	if(alarm_on==1)GPIOC->ODR|=0b1000000;
	//int count=0;
	int t;
	while(1)
	{
		time_now=TIM5->CNT;
		if(press==5){press=0;set_time(TIM5->CNT);}

		if(TIM5->CNT>=alarm_time&&TIM5->CNT<=alarm_time+100&&alarm_on==1)SysTick->CTRL = 3;

		time=TIM5->CNT/100;

        time1=time%10;
        time2=(time%60)/10;
        time=time/60;

        time3=time%10;
        time4=(time%60)/10;
        time=time/60;

        time5=time%10;
        time6=(time%60)/10;

        if(c==0)     {GPIOB->ODR=((0b0000010000000000)|(1<<time1));}
        else if(c==1){GPIOB->ODR=((0b0000100000000000)|(1<<time2));}
        else if(c==2){GPIOB->ODR=((0b0001000000000000)|(1<<time3));}
        else if(c==3){GPIOB->ODR=((0b0010000000000000)|(1<<time4));}
        else if(c==4){GPIOB->ODR=((0b0100000000000000)|(1<<time5));}
        else if(c==5){GPIOB->ODR=((0b1000000000000000)|(1<<time6));}

		t=DELAY;
		while(t--);
        GPIOB->ODR=0;

        t=200-DELAY;
        while(t--);

		t=DELAY1;
		while(t--);

        c=c==5?0:c+1;
	}
	return;
}
void humid()
{

	Timer_init();
	int time1,time2,time3,time4,time5,time6;
	GPIOC->ODR=0b100000;
	if(alarm_on==1)GPIOC->ODR|=0b1000000;

    int RH_Int_Data=0;
     int RH_Dec_Data =0;
     int Temp_Int_Data=0;
     int Temp_Dec_Data =0;
     int t;
     int c=0;
    //while(1){
        INIT_DHT();
        int flag = DHT_RESPONCSCE();

        if(flag){
             RH_Int_Data = DHT_READ();
             RH_Dec_Data = DHT_READ();
             Temp_Int_Data = DHT_READ();
             Temp_Dec_Data = DHT_READ();
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
        //}
            while(1)
            {
        		//time_now=TIM5->CNT;
            	if(TIM5->CNT>=alarm_time&&TIM5->CNT<=alarm_time+100&&alarm_on==1)SysTick->CTRL = 3;
        		if(press==5){press=0;show_time();}

        		//if(TIM5->CNT>=alarm_time&&TIM5->CNT<=alarm_time+100&&alarm_on==1)SysTick->CTRL = 3;

        		//time=TIM5->CNT/100;

                time6=RH_Int_Data/10;
                time5=RH_Int_Data%10;
                time2=Temp_Int_Data/10;
                time1=Temp_Int_Data%10;


                if(c==0)     {GPIOB->ODR=((0b0000010000000000)|(1<<time1));}
                else if(c==1){GPIOB->ODR=((0b0000100000000000)|(1<<time2));}
                else if(c==2){GPIOB->ODR=((0b0000000000000000)|(1<<time3));}
                else if(c==3){GPIOB->ODR=((0b0000000000000000)|(1<<time4));}
                else if(c==4){GPIOB->ODR=((0b0100000000000000)|(1<<time5));}
                else if(c==5){GPIOB->ODR=((0b1000000000000000)|(1<<time6));}

        		t=DELAY;
        		while(t--);
                GPIOB->ODR=0;

                t=200-DELAY;
                while(t--);

        		t=DELAY1;
        		while(t--);

                c=c==5?0:c+1;
            }

    }
	return ;

}
int main()
{
	GPIO_init();
	NVIC_config();
	EXTI_config();

	SysTick->LOAD = 1;
	SysTick->CTRL = 0;

	RCC->APB1ENR1|=RCC_APB1ENR1_TIM5EN;
	TIM5->ARR = 8640000-1;
	TIM5->PSC = 40376;
	TIM5->EGR = TIM_EGR_UG;
	TIM5->CR1 |= TIM_CR1_CEN;

	RCC->APB1ENR1|=RCC_APB1ENR1_TIM3EN;
	TIM3->ARR = 6000-1;
	TIM3->PSC = 40376;
	TIM3->EGR = TIM_EGR_UG;
	TIM3->CR1 |= TIM_CR1_CEN;

	RCC->APB1ENR1|=RCC_APB1ENR1_TIM4EN;
	TIM4->ARR = 86400;
	TIM4->PSC = 39;
	TIM4->EGR = TIM_EGR_UG;
	TIM4->CR1 |= TIM_CR1_CEN;

	RCC->APB1ENR1 |= RCC_APB1ENR1_TIM2EN;
	TIM2->CCMR1=0b000000000000000000000000001100000;
	TIM2->CCER=1;
	TIM2->ARR=0;
	TIM2->CCR1=2000;
	TIM2->EGR = TIM_EGR_UG;
	TIM2->CR1 |= TIM_CR1_CEN;
    press=0;

    show_time();

	return 0;
}
