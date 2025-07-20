#include <iostm8s103f3.h>

char src_buf[] = "Hello world!";
char dst_buf[100] = { 0 };

char * src_ptr;
char * dst_ptr;

int ptr_size;
int size;

void delay(unsigned int n)
{
    while (n-- > 0);
}

int main( void )
{
    PB_ODR = 0; //Turn off all pins
    PB_DDR_DDR5 = 1; //PortB, Bit 5 is output (PB5 - Data Direction Register)
    PB_CR1_C15 = 1; //PortB, Control Register 1, Bit 3 (PB5) set to Push-Pull
    PB_CR2_C25 = 1; //PortB, Control Register 2, Bit 3 (PB5) set to Push-Pull
   
    ptr_size = sizeof(src_ptr);
    
    src_ptr = src_buf;
    dst_ptr = dst_buf;
    
    size = sizeof(src_buf);
    while (size > 0)
    {
      *dst_ptr = *src_ptr;
      dst_ptr++;
      src_ptr++;
      size--;
    }
    
    while (1)
    {
        PB_ODR_bit.ODR5 = !PB_ODR_bit.ODR5;
        delay(30000);
    }
}
