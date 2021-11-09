/*** Translated by Charlie ***/
/**********************************************************************************************
 * Arduino PID Library - Version 1.1.1
 * by Brett Beauregard <br3ttb@gmail.com> brettbeauregard.com
 *
 * This Library is licensed under a GPLv3 License
 **********************************************************************************************/
#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif
#include <PID_v1.h>

PID::PID(double *Input, double *Output, double *Setpoint,
         double Kp, double Ki, double Kd, int POn, int ControllerDirection)
{
   myOutput = Output;
   myInput = Input;
   mySetpoint = Setpoint;
   inAuto = false;

   PID::SetOutputLimits(0, 255); //default output limit corresponds to
                                 //the arduino pwm limits

   SampleTime = 100; //default Controller Sample Time is 0.1 seconds

   PID::SetControllerDirection(ControllerDirection);
   PID::SetTunings(Kp, Ki, Kd, POn);

   lastTime = millis() - SampleTime;
}


PID::PID(double *Input, double *Output, double *Setpoint,
         double Kp, double Ki, double Kd, int ControllerDirection)
    : PID::PID(Input, Output, Setpoint, Kp, Ki, Kd, P_ON_E, ControllerDirection)
{
}

bool PID::Compute()
{
   if (!inAuto)
      return false;
   unsigned long now = millis();
   unsigned long timeChange = now - lastTime;
   if (timeChange >= SampleTime)
   {
      /* 计算所有误差 */
      double input = *myInput;
      double error = *mySetpoint - input;
      double dInput = (input - lastInput);
      outputSum += (ki * error);

      /* 当 P_ON_M 模式启用时，测量比例大小 */
      if (!pOnE)
         outputSum -= kp * dInput;

      if (outputSum > outMax)
         outputSum = outMax;
      else if (outputSum < outMin)
         outputSum = outMin;

      /* （默认）当 P_ON_E 模式启用时，测量误差大小 */
      double output;
      if (pOnE)
         output = kp * error;
      else
         output = 0;

      /* 对 output 进行调整，准备输出 */
      output += outputSum - kd * dInput;

      if (output > outMax)
         output = outMax;
      else if (output < outMin)
         output = outMin;
      *myOutput = output;

      /* 记录一些变量，准备下次使用 */
      lastInput = input;
      lastTime = now;
      return true;
   }
   else
      return false; // 没有什么要做的
}

void PID::SetTunings(double Kp, double Ki, double Kd, int POn)
{
   if (Kp < 0 || Ki < 0 || Kd < 0)
      return;

   pOn = POn;
   pOnE = POn == P_ON_E;

   dispKp = Kp;
   dispKi = Ki;
   dispKd = Kd;

   double SampleTimeInSec = double(SampleTime) / 1000;
   kp = Kp;
   ki = Ki * SampleTimeInSec;
   kd = Kd / SampleTimeInSec;

   if (controllerDirection == REVERSE)
   {
      kp = -kp;
      ki = -ki;
      kd = -kd;
   }
}

void PID::SetTunings(double Kp, double Ki, double Kd)
{
   SetTunings(Kp, Ki, Kd, pOn);
}

void PID::SetSampleTime(int NewSampleTime)
{
   if (NewSampleTime > 0)
   {
      double ratio = (double)NewSampleTime / (double)SampleTime;
      ki *= ratio;
      kd /= ratio;
      SampleTime = (unsigned long)NewSampleTime;
   }
}

void PID::SetOutputLimits(double Min, double Max)
{
   if (Min >= Max)
      return;
   outMin = Min;
   outMax = Max;

   if (inAuto)
   {
      if (*myOutput > outMax)
         *myOutput = outMax;
      else if (*myOutput < outMin)
         *myOutput = outMin;

      if (outputSum > outMax)
         outputSum = outMax;
      else if (outputSum < outMin)
         outputSum = outMin;
   }
}

void PID::SetMode(int Mode)
{
   bool newAuto = (Mode == AUTOMATIC);
   if (newAuto && !inAuto)
   { /* 只能开始控制，不能停止 2333 */
      PID::Initialize();
   }
   inAuto = newAuto;
}

/* Initialize()****************************************************************
 * 完成所有要做的事，确保从手动模式顺畅切换到自动模式。
 ******************************************************************************/
void PID::Initialize()
{
   outputSum = *myOutput;
   lastInput = *myInput;
   if (outputSum > outMax)
      outputSum = outMax;
   else if (outputSum < outMin)
      outputSum = outMin;
}

/* SetControllerDirection (...) ************************************************
 * The PID will either be connected to a DIRECT acting process (+Output leads
 * to +Input) or a REVERSE acting process(+Output leads to -Input.)  we need to
 * know which one, because otherwise we may increase the output when we should
 * be decreasing.  This is called from the constructor.
 ******************************************************************************/
void PID::SetControllerDirection(int Direction)
{
   if (inAuto && Direction != controllerDirection)
   {
      kp = (0 - kp);
      ki = (0 - ki);
      kd = (0 - kd);
   }
   controllerDirection = Direction;
}

/* Status Funcions *************************************************************
 * 设置 Kp=-1 并不代表它确实变成了 -1，下面的函数可以查询 PID 的内部状态，比如用于展示。
 ******************************************************************************/
double PID::GetKp() { return dispKp; }
double PID::GetKi() { return dispKi; }
double PID::GetKd() { return dispKd; }
int PID::GetMode() { return inAuto ? AUTOMATIC : MANUAL; }
int PID::GetDirection() { return controllerDirection; }
