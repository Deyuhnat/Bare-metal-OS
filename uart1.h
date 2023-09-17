#include "gpio.h"

/* Auxilary mini UART (UART1) registers */
#define AUX_ENABLE      (* (volatile unsigned int*)(MMIO_BASE+0x00215004))
#define AUX_MU_IO       (* (volatile unsigned int*)(MMIO_BASE+0x00215040))
#define AUX_MU_IER      (* (volatile unsigned int*)(MMIO_BASE+0x00215044))
#define AUX_MU_IIR      (* (volatile unsigned int*)(MMIO_BASE+0x00215048))
#define AUX_MU_LCR      (* (volatile unsigned int*)(MMIO_BASE+0x0021504C))
#define AUX_MU_MCR      (* (volatile unsigned int*)(MMIO_BASE+0x00215050))
#define AUX_MU_LSR      (* (volatile unsigned int*)(MMIO_BASE+0x00215054))
#define AUX_MU_MSR      (* (volatile unsigned int*)(MMIO_BASE+0x00215058))
#define AUX_MU_SCRATCH  (* (volatile unsigned int*)(MMIO_BASE+0x0021505C))
#define AUX_MU_CNTL     (* (volatile unsigned int*)(MMIO_BASE+0x00215060))
#define AUX_MU_STAT     (* (volatile unsigned int*)(MMIO_BASE+0x00215064))
#define AUX_MU_BAUD     (* (volatile unsigned int*)(MMIO_BASE+0x00215068))

/* Function prototypes */
void uart_init();
void uart_sendc(char c);
char uart_getc();
void uart_puts(char *s);
void uart_hex_byte(unsigned char byte);
void uart_hex(unsigned int d);
void uart_dec(int num);
char uart_get_char();
