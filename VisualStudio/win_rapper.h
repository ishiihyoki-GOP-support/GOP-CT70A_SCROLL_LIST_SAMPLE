typedef signed char         int8_t;
typedef unsigned char       uint8_t;
typedef signed short        int16_t;
typedef unsigned short      uint16_t;
typedef signed long         int32_t;
typedef unsigned long       uint32_t;
typedef unsigned short      MD_STATUS;

typedef int BOOL;
#define NULL 0
#define TRUE 1
#define FALSE 0
uint32_t get_syscount(void);
void uart_init(void);
int uart_getc(void);
void uart_putc(uint8_t c);
#ifdef  __cplusplus
extern "C"
{
#endif //  __cp
	MD_STATUS xfer_spi(uint8_t* _buf, uint16_t tx_num);
	MD_STATUS xfer_spi_R(uint8_t* _buf, uint16_t tx_num);
	MD_STATUS xfer_spi_W(uint8_t* _buf, uint16_t tx_num);
	void reset_out(BOOL sta);
	void spi_cs(BOOL sta);
	uint8_t getio();
#ifdef  __cplusplus
}
#endif //  __cp

#define MD_STATUSBASE       (0x00U)
#define MD_OK               (MD_STATUSBASE + 0x00U) /* register setting OK */
#define MD_SPT              (MD_STATUSBASE + 0x01U) /* IIC stop */
#define MD_NACK             (MD_STATUSBASE + 0x02U) /* IIC no ACK */
#define MD_BUSY1            (MD_STATUSBASE + 0x03U) /* busy 1 */
#define MD_BUSY2            (MD_STATUSBASE + 0x04U) /* busy 2 */
#define MD_OVERRUN          (MD_STATUSBASE + 0x05U) /* IIC OVERRUN occur */

