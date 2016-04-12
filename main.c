#include <mc30p011.h>




#define EnWatchdog()            WDTEN = 1
#define DisWatchdog()   WDTEN = 0

#define key_interrupt_enable()  KBIE = 1
#define key_interrupt_disable() KBIE = 1

#define pwm_stop()    PWMOUT = 0  //关闭PWM
#define pwm_start()   PWMOUT = 1  //启动PWM

#define Stop()  __asm__("stop")
#define ClrWdt()        __asm__("clrwdt")
#define NOP()   __asm__("nop")




// p11 data
//p12 clk


#define SDA_H()                 (P11 = 1)
#define SDA_L()                 (P11 = 0)

#define SCL_H()                 (P12 = 1)
#define SCL_L()                 (P12 = 0)

#define GET_SDA()               (P11)

#define CHG_SDA_OUT()   (DDR11 = 0)
#define CHG_SDA_IN()    (DDR11 = 1)

/*
#define SDA_H()                 (P10 = 1)
#define SDA_L()                 (P10 = 0)

#define SCL_H()                 (P11 = 1)
#define SCL_L()                 (P11 = 0)

#define GET_SDA()               (P10)

#define CHG_SDA_OUT()   (DDR10 = 0)
#define CHG_SDA_IN()    (DDR10 = 1)
*/

#define IIC_ADDR  0xEA

#define _nop_() __asm__("nop")
#define swait_uSec(n) Wait_uSec(n)

const unsigned char markbit[8] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};


unsigned char temp_char_4;
unsigned char temp_char_1;
unsigned char temp_char_2;
unsigned char temp_char_3;
unsigned char temp_char_delay;

unsigned char   StatusBuf,ABuf;
void isr(void) __interrupt
{
         __asm
                movra   _ABuf
                swapar  _STATUS
                movra   _StatusBuf
        __endasm;

        if(T0IF)
        {
                T0IF = 0;

                //g3STick++;
        }
        __asm
                swapar  _StatusBuf
                movra   _STATUS
                swapr   _ABuf
                swapar  _ABuf
        __endasm;     
}

void    Wait_uSec(unsigned char DELAY)
{
     

         temp_char_4 = DELAY<<1;                          // by 16MHz IRC OSC
      
         while(temp_char_4>0)  temp_char_4--;                    //
}
void delay_ms(unsigned int count)
{

        while(count>0)
        {
                count--;
                for(temp_char_delay =0 ;temp_char_delay < 250; temp_char_delay++);
                ClrWdt();
        }
        ClrWdt();
}

static void IIC_START()
{
        SDA_H();
        SCL_H();
        _nop_();
        SDA_L();
        Wait_uSec(1);   
        SCL_L();
        //_nop_();
}       


static void IIC_STOP()
{
        SCL_L();
        SDA_L();
        CHG_SDA_OUT();
        Wait_uSec(1);
        SCL_H();
        Wait_uSec(1);
        SDA_H();
        //Wait_uSec(2);
}

static void IIC_SEND_ACK()
{
        SCL_L();
        CHG_SDA_OUT();
        SDA_H();
        swait_uSec(4);  
        SCL_H();
        Wait_uSec(1);
        //SCL_L();
}

static char IIC_CHECK_ACK()
{
        //char ret=0;

        SCL_L();
        CHG_SDA_IN();
        swait_uSec(3);  
        SCL_H();
        swait_uSec(3);
        if(GET_SDA() == 0)
        {
                temp_char_1= 1;
        }
        else
        {
                temp_char_1 = 0;
        }
        //SCL_L();
        //SDA_H();
        //CHG_SDA_OUT();
        
        return temp_char_1;
}


static char IIC_SEND_BYTE(unsigned char val)
{
        //signed char i; temp_char_1
        CHG_SDA_OUT();
        for (temp_char_1 = 8; temp_char_1 > 0; temp_char_1--)
        {
                SCL_L();
                if(val & markbit[temp_char_1-1])
                {
                        SDA_H();
                }
                else
                {
                        SDA_L();
                }
                /*
                if(i == 7)
                {
                        CHG_SDA_OUT();
                }
                else
                {
                        _nop_();
                }
                */
                _nop_();
                _nop_();
                _nop_();
                SCL_H();
                swait_uSec(2);          
                //SCL_L();
                //Wait_uSec(2);
        }
        return IIC_CHECK_ACK();
}

static unsigned char IIC_GET_BYTE()
{
        //signed char i;   temp_char_1
        //unsigned char rdata = 0;   temp_char_2

        temp_char_2 =0;
        
        SCL_L();
        CHG_SDA_IN();   //xyl change
        for(temp_char_1=8; temp_char_1>0; temp_char_1--)
        {
                SCL_L();
                swait_uSec(3);
                SCL_H();
                _nop_();
                _nop_();
                _nop_();
                if(GET_SDA())
                {
                   temp_char_2 = temp_char_2 | markbit[temp_char_1-1];
                }
        }

        return temp_char_2;
}


void  I2C_write(unsigned char reg, unsigned char val)
{
        //signed char ret = 0;

        
        IIC_START();

        #if 0
        if(!IIC_SEND_BYTE(IIC_ADDR))
                ret = -1;
        if(!IIC_SEND_BYTE(reg))
                ret = -2;
        if(!IIC_SEND_BYTE(val))
                ret = -3;
        #else
        IIC_SEND_BYTE(IIC_ADDR);
        IIC_SEND_BYTE(reg);
        IIC_SEND_BYTE(val);
        #endif
        IIC_STOP();
        Wait_uSec(2);
}

short I2C_read(unsigned char reg)
{
        //unsigned char val;    temp_char_3
        //signed short ret = 0; temp_char_2

        temp_char_2 = 0;

        IIC_START();
        #if 0
        if(!IIC_SEND_BYTE(IIC_ADDR)) ret = -1;
        if(!IIC_SEND_BYTE(reg)) ret = -2;
        IIC_STOP();
        Wait_uSec(1);
        IIC_START();
        if(!IIC_SEND_BYTE(IIC_ADDR+1)) ret = -3;
        val =IIC_GET_BYTE();
        IIC_SEND_ACK();
        IIC_STOP();
        Wait_uSec(10);
        #else
        IIC_SEND_BYTE(IIC_ADDR);
        IIC_SEND_BYTE(reg);
        IIC_STOP();
        Wait_uSec(1);
        IIC_START();    
        IIC_SEND_BYTE(IIC_ADDR+1);
        temp_char_3 =IIC_GET_BYTE();
        IIC_SEND_ACK();
        IIC_STOP();
        #endif
        Wait_uSec(10);

        return (short)temp_char_3;
}



void InitConfig()
{
        /*
        *       MCU config
        */
        
       // DDR0 = 0;
        DDR1 = 0xE9;   // 11101001

         P1 = 0;

        T0CR = 0x07;
        T0IE = 0;
        
        GIE = 1;                    //使能全局中断
}




void main()
{
        InitConfig();    //初始化配置
        DisWatchdog();

        EnWatchdog();
        
   
        

        while(1)
        {
                ClrWdt();
        }
     
}


