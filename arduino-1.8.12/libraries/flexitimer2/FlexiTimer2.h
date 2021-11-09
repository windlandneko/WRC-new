/*** Translated by Charlie ***/
#ifndef FlexiTimer2_h
#define FlexiTimer2_h

#ifdef __AVR__
#include <avr/interrupt.h>
#else
#error FlexiTimer2 library only works on AVR architecture
#endif

namespace FlexiTimer2
{
	extern unsigned long time_units;
	extern void (*func)();
	extern volatile unsigned long count;
	extern volatile char overflowing;
	extern volatile unsigned int tcnt2;

	/**********************************
	 * @p 使用此函数初始化定时器
	 * @param ms
	 *   多少毫秒执行一次回调函数
	 * @param f
	 *   回调函数
 	 **********************************/
	void set(unsigned long ms, void (*f)());

	/**********************************
	 * @p 使用此函数初始化定时器
	 * @param units
	 *   多少周期执行一次回调函数
 	 * @param resolution
 	 *   周期大小，0.001 代表周期为 1 ms（默认值），0.0005 代表周期为 0.5 ms ，以此类推
	 * @param f
	 *   回调函数
 	 **********************************/
	void set(unsigned long units, double resolution, void (*f)());
	void start(); // 开始定时触发
	void stop(); // 停止定时触发
	void _overflow(); // 内部函数，每周期执行一次
}

#endif
