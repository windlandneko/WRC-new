// #include "NewPing.h"
#include "pid.h"
#define RED 0
#define BLUE 1
#define END -1
#define near 2
#define far 3
int S1, S2, S3, S4, S5;
int g_linestate = 0;
int ADC_TD[5] = {0, 0, 0, 0, 0};
// NewPing dis(43, 42); // Trigger pin; Echo pin
int ccount = 0;
int isbeep = 0;
int beepcount = 0;

inline void memwrite(int value, int index)
{
  EEPROM.write(index * 2 + 1000, value % 256);
  EEPROM.write(index * 2 + 1001, value / 256);
}
inline long memread(int index)
{
  return EEPROM.read(index * 2 + 1000) + 256 * EEPROM.read(index * 2 + 1001);
}
void newtone(int frequency, int duration)
{
  FlexiTimer2::stop();
  int period = 1000000L / frequency;
  int pulse = period / 2;
  for (long i = 0; i < duration * 1000L; i += period)
  {
    digitalWrite(9, HIGH);
    delayMicroseconds(pulse);
    digitalWrite(9, LOW);
    delayMicroseconds(pulse);
  }
  inittimer();
}
void beep()
{
  isbeep = 1;
}
void setservo(int pin, int angle)
{
  FlexiTimer2::stop();
  servo(pin, angle);
  inittimer();
}
//巡线过程中，不可调用tone控制发声
void pid()
{
  ccount++;
  if (isbeep)
  {
    beepcount++;
    setDigitalPin(9, beepcount % 2);
    if (beepcount > 30)
    {
      beepcount = 0;
      isbeep = 0;
      setDigitalPin(9, 0);
    }
  }

  if (ccount >= pidwm / ms)
  {
    Speed();
    // Serial.print(M3S);
    // Serial.print(",");
    // Serial.print(M4S);
    // Serial.print(",");
    // Serial.print(getMotor3Code());
    // Serial.print(",");
    // Serial.print(getMotor4Code());
    // Serial.print(",");
    // Serial.print(M3count2);
    // Serial.print(",");
    // Serial.println(M4count2);

    ccount = 0;
  }
  setspeed();
  moving();
}
void inittimer()
{
  FlexiTimer2::set(ms, pid); //  ms代表ms次计数 ， 1.0/1000表示分辨率是1ms  ，pid代表的是你的回调函数
  FlexiTimer2::start();      //开启定时器
}

//获得5个光电的状态值
void getState()
{
  S1 = analogRead(A1) < ADC_TD[0];
  S2 = analogRead(A2) < ADC_TD[1];
  S3 = analogRead(A3) < ADC_TD[2];
  S4 = analogRead(A4) < ADC_TD[3];
  S5 = analogRead(A5) < ADC_TD[4];
}
void line(int speed)
{
  getState();
  if ((S3 && !S2 && !S4) || (S3 && S2 && S4))
  {
    set_2Motor(speed, speed);
  }
  else if (S3 && S2)
  {
    set_2Motor(speed - 6, speed + 2);
    g_linestate = 2;
  }
  else if (S3 && S4)
  {
    set_2Motor(speed + 2, speed - 6);
    g_linestate = 4;
  }
  else if (S2)
  {
    set_2Motor(speed - 18, speed + 3);
    g_linestate = 2;
  }
  else if (S4)
  {
    set_2Motor(speed + 3, speed - 18);
    g_linestate = 4;
  }
  else
  {
    if (g_linestate == 2)
    {
      set_2Motor(-2, speed);
    }
    else
    {
      set_2Motor(speed, -2);
    }
  }
  if (S1)
    g_linestate = 2;
  if (S5)
    g_linestate = 4;
}

//时间循迹，单位ms
void golineTime(long time, int speed = 35)
{
  unsigned long T1 = millis();
  while (millis() - T1 < time)
  {
    getState();
    line(speed);
  }
}
//走编码，code为要走的编码脉冲数，spL左马达速度，spR右马达速度，LR=0左马达编码到达code退出
void gocode(int code, int spL, int spR, int LR = 0)
{
  // resetPid();
  long m3c = getMotor3Code();
  long m4c = getMotor4Code();
  set_2Motor(spL, spR);
  if (LR == 0)
  {
    while (abs(getMotor3Code() - m3c) < code)
    {
      delay(10);
    }
  }
  else
  {
    while (abs(getMotor4Code() - m4c) < code)
    {
      delay(10);
    }
  }
  set_2Motor(0, 0);
  resetPid();
}
//走时间（不巡线），time为要走的毫秒数，spL左马达速度，spR右马达速度
void gotime(int time, int spL, int spR)
{
  unsigned long T1 = millis();
  set_2Motor(spL, spR);
  while (millis() - T1 <= time)
  {
    delay(1);
  }
  set_2Motor(0, 0);
}
//编码巡线
void golinecode(long code, int speed = 35, int LR = 0)
{
  // resetPid();
  long m3c = getMotor3Code();
  long m4c = getMotor4Code();
  while (1)
  {
    getState();
    line(speed);
    if (LR == 0)
    {
      if (abs(getMotor3Code() - m3c) >= code)
      {
        set_2Motor(0, 0);
        break;
      }
    }
    else
    {
      if (abs(getMotor4Code() - m4c) >= code)
      {
        set_2Motor(0, 0);
        break;
      }
    }
  }
  // stop_all();
}
//数线循迹，默认sp速度40，选15号光电数线goline(2)：向前走两条横线，goline(2,30)用30速度走两条线
void goline(int total_line, int speed = 60, int endtime = -1)
{
  unsigned long T1 = 0, T5 = 0, T = millis();
  int count = 0;
  while (endtime == -1 || micros() - T < endtime)
  {
    getState();
    T1 = (S1 ? millis() : T1);
    T5 = (S5 ? millis() : T5);
    if (T1 > 0 && T5 > 0 && abs(T1 - T5) < 500) // 到达一根线了~
    {
      T1 = T5 = 0; // 重置定时
      count++;     // 走完一根线了
      if (count >= total_line)
        break;
      while ((S1 || S5) && (endtime == -1 || micros() - T < endtime))
        line(speed);
      golineTime(300, speed);
    }
    line(speed);
  }
  set_2Motor(0, 0);
}
//光电转弯，spL，spR左右马速度，sensor为遇线停的光电
void Turn(int spL, int spR, int sensor)
{
  set_2Motor(spL, spR);
  delay(330);
  while (1)
  {
    getState();
    if (sensor == 1 && S1)
    {
      set_2Motor(0, 0);
      break;
    }

    if (sensor == 2 && S2)
    {
      set_2Motor(0, 0);
      break;
    }

    if (sensor == 3 && S3)
    {
      set_2Motor(0, 0);
      break;
    }

    if (sensor == 4 && S4)
    {
      set_2Motor(0, 0);
      break;
    }

    if (sensor == 5 && S5)
    {
      set_2Motor(0, 0);
      break;
    }
  }
}

void Event()
{
  tone(9, 66, 30); //巡线过程中，不可调用tone控制发声
}
void waitStart()
{
  tone(9, TONE_CH3, 40); //巡线过程中，不可调用tone控制发声
}

void checkw()
{
  while (!getKey())
    ;
  unsigned long turntime;
  set_2Motor(50, -50);
  getState();
  while (!S3)
  {
    getState();
  }
  turntime = micros();
  delay(100);
  getState();
  while (!S3)
  {
    getState();
  }
  delay(100);
  getState();
  while (!S3)
  {
    getState();
  }
  turntime = micros() - turntime;
  set_2Motor(0, 0);
  Serial.println(turntime);
  unsigned long time = turntime / 1000; //旋转180°时间；
  newtone(1200, 100);
  EEPROM.write(20, time);
  delay(700);
  gotime(time, 50, -50);
  newtone(1500, 100);
  delay(700);
  gotime(time, -50, 50);
  newtone(1500, 100);
}

//查看光电状态
void see()
{
  ADC_TD[0] = EEPROM.read(1) * 4;
  ADC_TD[1] = EEPROM.read(2) * 4;
  ADC_TD[2] = EEPROM.read(3) * 4;
  ADC_TD[3] = EEPROM.read(4) * 4;
  ADC_TD[4] = EEPROM.read(5) * 4;

  FlexiTimer2::set(2000, waitStart); //  500代表500次计数 ， 1.0/1000表示分辨率是1ms  ，Event代表的是你的回调函数
  FlexiTimer2::start();              //开启定时器
  while (getKey())
  {
    int avar[5] = {0, 0, 0, 0, 0};
    avar[0] = getAnalogPin(A1);
    avar[1] = getAnalogPin(A2);
    avar[2] = getAnalogPin(A3);
    avar[3] = getAnalogPin(A4);
    avar[4] = getAnalogPin(A5);

    int length = sizeof(ADC_TD) / sizeof(ADC_TD[0]);
    int a_state[5] = {0, 0, 0, 0, 0};
    if ((avar[1] < ADC_TD[1] && avar[2] < ADC_TD[2]) || (avar[2] < ADC_TD[2] && avar[3] < ADC_TD[3]))
    {
      setRGB(4);
    }
    else if ((avar[1] < ADC_TD[1] && avar[2] < ADC_TD[2] && avar[3] < ADC_TD[3]))
    {
      setRGB(5);
    }
    else if (avar[0] < ADC_TD[0] || avar[1] < ADC_TD[1] || avar[2] < ADC_TD[2] || avar[3] < ADC_TD[3] || avar[4] < ADC_TD[4])
    {
      setRGB(3);
    }
    else
    {
      setRGB(7);
    }

    // Serial.print(ADC_TD[0]);
    // Serial.print(",");
    // Serial.print(ADC_TD[1]);
    // Serial.print(",");
    // Serial.print(ADC_TD[2]);
    // Serial.print(",");
    // Serial.print(ADC_TD[3]);
    // Serial.print(",");
    // Serial.print(ADC_TD[4]);
    // Serial.print("---");

    // Serial.print(a_state[0]);
    // Serial.print(",");
    // Serial.print(a_state[1]);
    // Serial.print(",");
    // Serial.print(a_state[2]);
    // Serial.print(",");
    // Serial.print(a_state[3]);
    // Serial.print(",");
    // Serial.println(a_state[4]);

    // Serial.print(avar[0]);
    // Serial.print(",");
    // Serial.print(avar[1]);
    // Serial.print(",");
    // Serial.print(avar[2]);
    // Serial.print(",");
    // Serial.print(avar[3]);
    // Serial.print(",");
    // Serial.println(avar[4]);
    delay(100);
  }
  FlexiTimer2::stop();
  // delay(1000);
  // if(!getKey())
  // {
  //   FlexiTimer2::start();
  //   while(getKey())
  //     ;
  //   checkw();
  //   FlexiTimer2::stop();
  // }
}
//扫描程序，先把机器放到场地上，按着按钮开机，听到提示音进入扫描程序，扫描过程会右短促提示音
//这时让机器5个光电都经过一次黑线和白色区域，然后再按一次按钮跳出扫描程序
void init_light_sensor()
{
  tone(9, 440, 500);
  while (!getKey())
    ;
  delay(500);
  FlexiTimer2::set(200, 1.0 / 1000, Event); //  500代表500次计数 ， 1.0/1000表示分辨率是1ms  ，Event代表的是你的回调函数
  FlexiTimer2::start();                     //开启定时器
  int length = sizeof(ADC_TD) / sizeof(ADC_TD[0]);
  char str[4];
  int max_var[5] = {0, 0, 0, 0, 0};
  int min_ver[5] = {9999, 9999, 9999, 9999, 9999};
  int avar[5] = {0, 0, 0, 0, 0};
  unsigned long dttime = millis();
  while (getKey())
  {

    avar[0] = getAnalogPin(A1);
    avar[1] = getAnalogPin(A2);
    avar[2] = getAnalogPin(A3);
    avar[3] = getAnalogPin(A4);
    avar[4] = getAnalogPin(A5);

    for (int i = 0; i < length; i++)
    {
      if (avar[i] > max_var[i])
        max_var[i] = avar[i];
      if (avar[i] < min_ver[i])
      {
        min_ver[i] = avar[i];
      }
      ADC_TD[i] = (max_var[i] + min_ver[i]) * 1 / 2;

      /* code */
    }
  }
  FlexiTimer2::stop();
  for (int i = 0; i < length; i++)
  {
    ADC_TD[i] = ADC_TD[i] / 4;
    EEPROM.write(i + 1, ADC_TD[i]);
  }
  //巡线过程中，不可调用tone控制发声
  tone(9, 440, 100);
  delay(200);
  tone(9, 440, 100);
  delay(200);
}

//扫描程序，先把机器放到场地上，按着按钮开机，听到提示音进入扫描程序，扫描过程会有短促提示音
//这时让机器5个光电都经过一次黑线和白色区域，然后再按一次按钮跳出扫描程序
void sets()
{
  pinMode(4, OUTPUT);
  pinMode(9, OUTPUT);
  getMotor3Code();
  getMotor4Code();
  if (!getKey()) // 校准光电
    init_light_sensor();

  M3PID.SetMode(AUTOMATIC);   //设置PID为自动模式
  M3PID.SetSampleTime(pidwm); //设置PID采样频率为pidwm ms
  M3PID.SetOutputLimits(-255, 255);

  M4PID.SetMode(AUTOMATIC);   //设置PID为自动模式
  M4PID.SetSampleTime(pidwm); //设置PID采样频率为pidwm ms
  M4PID.SetOutputLimits(-255, 255);

  setRGB(7);
  see();

  inittimer();

  ADC_TD[0] = EEPROM.read(1) * 4;
  ADC_TD[1] = EEPROM.read(2) * 4;
  ADC_TD[2] = EEPROM.read(3) * 4;
  ADC_TD[3] = EEPROM.read(4) * 4;
  ADC_TD[4] = EEPROM.read(5) * 4;
}

void analysis()
{
  Serial.begin(115200);
  Serial.println("Ver 9");
  Serial.println("这辆车是 Charlie 瞎改的说 ~ 请不要玩坏了哦 ~");
  if (memread(0) < memread(1))
    memwrite(memread(0), 1);
  Serial.print("啊这... 这个车上次送货需要 ");
  Serial.print(memread(0) / 1000);
  Serial.print(".");
  Serial.print(memread(0) % 1000);
  Serial.println(" 秒.");
  Serial.print("而历史送货记录为 ");
  Serial.print(memread(1) / 1000);
  Serial.print(".");
  Serial.print(memread(1) % 1000);
  Serial.println(" 秒, 真是太屑了! 就不能再快点吗??");
  memwrite(memread(2) + 1, 2);
  Serial.print("这是自 2021/12/02 16:14 以来第 ");
  Serial.print(memread(2));
  Serial.println(" 次启动这辆车了.");
}
