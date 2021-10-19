/*
 * pinmux.h
 */

#ifndef __MX60_PINMUX_H__
#define __MX60_PINMUX_H__

/* uart  */
void setup_iomux_uart1(void);
void setup_iomux_uart2(void);

/* usdhc */
void setup_iomux_usdhc1(void);
void setup_iomux_usdhc2(void);
void setup_iomux_usdhc3(void);

/* i2c   */
void setup_iomux_i2c1(void);
void setup_iomux_i2c2(void);

/* gpio  */
void setup_iomux_gpio1(void);
void setup_iomux_gpio2(void);
void setup_iomux_gpio3(void);
void setup_iomux_gpio4(void);
void setup_iomux_gpio5(void);

/* board */
void setup_iomux(void);
void setup_iomux_lpm(void);

#endif /* __MX60_PINMUX_H__ */
