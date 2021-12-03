#include <EEPROM.h>
#include <FlexiTimer2.h>
#include <PID_v1.h>

unsigned long t;
double M3PWMOUT, M4PWMOUT; //电机输出pwm
double ref3 = 0, ref4 = 0; //四个电机的参考转速
double M3S = 0, M4S = 0;   //M1-4S是转换成转速后的值（没有单位）
double M3count = 0, M4count = 0;
double M3count2 = 0, M4count2 = 0;
int maxspeed = 100; //最高限速
int pidwm = 20;     //pid频率
int ms = 2;
double Kp = 12, Ki = 0.6, Kd = 0.02; //PID系数
PID M3PID(&M3S, &M3PWMOUT, &ref3, Kp, Ki, Kd, DIRECT);
PID M4PID(&M4S, &M4PWMOUT, &ref4, Kp, Ki, Kd, DIRECT);
//***********************************************************************
extern void inittimer();
extern void setspeed();
extern void moving();
//pid马达速度控制
void set_2Motor(int sp1, int sp2)
{
  ref3 = sp1;
  ref4 = sp2;
  if (sp1 == 0)
    M3PWMOUT = 0;
  if (sp2 == 0)
    M4PWMOUT = 0;
  setspeed();
  moving();
}

long getMotor3Code()
{
  return getCodePin(m3);
}
long getMotor4Code()
{
  return -getCodePin(m4);
}
//--------计算速度
void Speed()
{
  M3count = getMotor3Code();
  if (abs(M3count2 - M3count) < 255)
  {
    M3S = M3count - M3count2;
  }
  M3count2 = M3count;

  M4count = getMotor4Code();
  if (abs(M4count2 - M4count) < 255)
  {
    M4S = M4count - M4count2;
  }
  M4count2 = M4count;
}
/***********************************/
void setspeed() //PID计算，得出输出PWM
{
  M3PID.Compute();
  M4PID.Compute();
}

void moving()
{
  setMotorPin(M3, M3PWMOUT);
  setMotorPin(M4, -M4PWMOUT);
}
void resetPid()
{
  M3S = M4S = ref3 = ref4 = M3PWMOUT = M4PWMOUT = 0;
  setClearCodePin(M3);
  setClearCodePin(M4);

  //    M3PWMOUT=0;
  //    M4PWMOUT=0;
  Speed();
  setspeed();
  //    moving();
}
void stop_all() //停止
{
  //   FlexiTimer2::stop();

  M3PWMOUT = 0;
  M4PWMOUT = 0;
  setspeed();
  moving();
  setClearCodePin(m3);
  setClearCodePin(m4);
  M3count2 = 0;
  M3count = 0;
  M4count2 = 0;
  M4count = 0;
  setMotorPin(M3, 0);
  setMotorPin(M4, 0);
  //   inittimer();
}
void fail_safe()
{
  stop_all();
  Speed();
  setspeed();
  moving();
}