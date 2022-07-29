#include "win_rapper.h"
#include "dummysys/dummyhard.h"
void sample_main(void);
int OpenSPI(int rate);


uint32_t get_syscount(void)
{
	return TimeCount();
}
void uart_init(void)
{

}
int uart_getc(void)
{
	return commGetc();
}
void uart_putc(uint8_t c)
{
	commPutc((char)c);
}

void main()
{
	OpenSPI(2000000);
	sample_main();
}