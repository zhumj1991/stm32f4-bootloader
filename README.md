# stm32f4-bootloader
Download a **.bin to spi flash with the command 'loady', and copy it to external sram and run with the command 'bootm'.
This bootload just boot a applicant form external sram, with a lot of commands like uboot, you can add you own commmand if you know how to use uboot command.
It can not boot a operation system like Linux, but if you use system like freertos which do not need kind of 'env'parament, the bootload is OK.
