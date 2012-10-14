/*
 * gpio.h
 *
 *  Created on: Sep 8, 2012
 *      Author: xen
 */

#ifndef GPIO_H_
#define GPIO_H_
/*----------------------------------------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>
/*----------------------------------------------------------------------------*/
typedef int16_t gpioKey;
/*----------------------------------------------------------------------------*/
#define GPIO_TO_PIN(port, pin) ((gpioKey)((int8_t)port << 8 | (int8_t)pin))
/*----------------------------------------------------------------------------*/
enum gpioDir {GPIO_INPUT = 0, GPIO_OUTPUT};
enum gpioPull {GPIO_NOPULL = 0, GPIO_PULLUP, GPIO_PULLDOWN};
enum gpioType {GPIO_PUSHPULL = 0, GPIO_OPENDRAIN};
/*----------------------------------------------------------------------------*/
union GpioPin
{
  gpioKey key;
  struct
  {
    int8_t offset;
    int8_t port;
  };
};
/*----------------------------------------------------------------------------*/
struct GpioPinMode
{
  gpioKey key;
  uint8_t mode;
};
/*----------------------------------------------------------------------------*/
struct Gpio
{
  union GpioPin pin;
  LPC_GPIO_TypeDef *control;
};
/*----------------------------------------------------------------------------*/
void gpioInit();
void gpioDeinit();
/*----------------------------------------------------------------------------*/
struct Gpio gpioInitPin(gpioKey, enum gpioDir);
void gpioReleasePin(struct Gpio *);
/*----------------------------------------------------------------------------*/
uint8_t gpioRead(struct Gpio *);
void gpioWrite(struct Gpio *, uint8_t);
/*----------------------------------------------------------------------------*/
uint8_t gpioFindMode(const struct GpioPinMode *, gpioKey);
void gpioSetMode(struct Gpio *, uint8_t);
void gpioSetPull(struct Gpio *, enum gpioPull);
void gpioSetType(struct Gpio *, enum gpioType);
int16_t gpioGetId(struct Gpio *);
/*----------------------------------------------------------------------------*/
#endif /* GPIO_H_ */
