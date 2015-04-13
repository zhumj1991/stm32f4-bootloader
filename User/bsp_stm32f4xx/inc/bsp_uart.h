#ifndef __BSP_UART_H
#define __BSP_UART_H

void	bsp_init_uart(void);
void	bsp_init_nvic(void);


int		serial_putc (int ch);
void	serial_puts (const char *s);
int		serial_getc (void);
int		serial_tstc (void);

#endif
