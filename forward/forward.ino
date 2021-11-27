#include <EEPROM.h>            // 带电可擦可编程只读存储器控制代码库（迫真）
#include <FlexiTimer2.h>       // 定时触发库，用作函数循环
#include <JMDlib.h>            // 硬件相关驱动库（所以为什么一半都是音符频率相关。。）
#include <LiquidCrystal_I2C.h> // 液晶屏控制库（真的有用吗？）
#include <PID_v1.h>            // PID 算法库

int up1Pin = 4, // 第一个舵机引脚号
    up2Pin = 5, // 中间的舵机引脚号
    gpPin = 6;  // 爪子舵机引脚号
unsigned int up1 = 90, up2 = 90, gp = 90;

unsigned int button0_Up1 = 0, button0_Up2 = 0, button0_Gp = 0;
unsigned int button11_Up1 = 0, button11_Up2 = 0, button11_Gp = 0;

int joyStickData[10]; // 接收的遥控手柄数据

// ----------------- 四个电机的旋转、平移、前后速度值
int mafor, mago, maturn;
int mbfor, mbgo, mbturn;
int mcfor, mcgo, mcturn;
int mdfor, mdgo, mdturn;

const int L = 5, R = -5;
double M1PWMOUT, M2PWMOUT, M3PWMOUT, M4PWMOUT; // 4个电机输出PWM
double ref1, ref2, ref3, ref4;                 // 四个电机的参考转速
double M1S, M2S, M3S, M4S;                     // in1-4是脉冲的个数，M1-4S是转换成转速（没有单位）后的值

int maxspeed = 100; // 最高限速
int pidwm = 30;     // PID采样频率

double Kp = 5.0, Ki = 4.8, Kd = 0.0; // PID系数
unsigned long t;
PID M1PID(&M1S, &M1PWMOUT, &ref1, Kp, Ki, Kd, DIRECT); // 将四个电机绑定到 PID 控制上
PID M2PID(&M2S, &M2PWMOUT, &ref2, Kp, Ki, Kd, DIRECT);
PID M3PID(&M3S, &M3PWMOUT, &ref3, Kp, Ki, Kd, DIRECT);
PID M4PID(&M4S, &M4PWMOUT, &ref4, Kp, Ki, Kd, DIRECT);
// ***********************************************************************

// 计算速度
void Speed()
{
  long code = getCodePin(m1);
  if (abs(code) < 255)
    M1S = code;
  code = getCodePin(m2);
  if (abs(code) < 255)
    M2S = -code;
  code = getCodePin(m3);
  if (abs(code) < 255)
    M3S = -code;
  code = getCodePin(m4);
  if (abs(code) < 255)
    M4S = code;
  setClearCodePin(m1);
  setClearCodePin(m2);
  setClearCodePin(m3);
  setClearCodePin(m4);
}

// 移动
void moving()
{
  setMotorPin(M1, M1PWMOUT);
  setMotorPin(M2, -M2PWMOUT);
  setMotorPin(M3, -M3PWMOUT);
  setMotorPin(M4, M4PWMOUT);
}

// 停止
inline void stop_all()
{
  M1PWMOUT = M2PWMOUT = M3PWMOUT = M4PWMOUT = 0;
  moving();
}

/***********************************/

// PID计算，得出输出PWM
void setspeed()
{
  M1PID.Compute();
  M2PID.Compute();
  M3PID.Compute();
  M4PID.Compute();
}

void buzzer(int t) { tone(9, 440, t); }

void Event() { tone(9, 66, 30); }

// 同时按下遥控上的 1，2，5，8 四个按钮进入设置
// 进入设置后，5、7 控制4号引脚舵机，6、8 控制5号引脚舵机，9、10 控制6号夹取舵机
// 按 0 和 11 按钮保存两个挡位的角度值，按 3 或者 4 按钮退出设置
void setting()
{
  tone(9, 440, 100);
  delay(200);
  tone(9, 440, 100);
  delay(200);
  unsigned long b = 0;
  FlexiTimer2::set(1000, Event); // 每 1000 ms 触发一次 Event 函数
  FlexiTimer2::start();          // 开始计时
  up1 = button0_Up1;
  up2 = button0_Up2;
  gp = button0_Gp;
  servo(up1Pin, up1);
  servo(up2Pin, up2);
  servo(gpPin, gp);
  while (true)
  {
    b = getRFModuleRemoteButtonPin(0);
    switch (b)
    {
    case 7:
      up1 += 1;
      up1 = up1 > 180 ? 180 : up1;
      servo(up1Pin, up1);
      servo(up2Pin, up2);
      break;

    case 5:
      up1 -= 1;
      up1 = up1 < 0 ? 0 : up1;
      servo(up1Pin, up1);
      servo(up2Pin, up2);
      break;

    case 6:
      up2 += 1;
      up1 = up1 > 180 ? 180 : up1;
      servo(up1Pin, up1);
      servo(up2Pin, up2);
      break;

    case 8:
      up2 -= 1;
      up1 = up1 < 0 ? 0 : up1;
      servo(up1Pin, up1);
      servo(up2Pin, up2);
      break;

    case 10:
      gp += 1;
      gp = gp > 180 ? 180 : gp;
      servo(gpPin, gp);
      break;

    case 9:
      gp -= 1;
      gp = gp < 0 ? 0 : gp;
      servo(gpPin, gp);
      break;

    case 0:
      button0_Up1 = up1;
      button0_Up2 = up2;
      button0_Gp = gp;
      servo(up1Pin, button11_Up1);
      servo(up2Pin, button11_Up2);
      servo(gpPin, button11_Gp);
      EEPROM.write(10, button0_Up1);
      EEPROM.write(11, button0_Up2);
      EEPROM.write(12, button0_Gp);
      tone(9, 880, 100);
      delay(500);
      break;

    case 11:
      button11_Up1 = up1;
      button11_Up2 = up2;
      button11_Gp = gp;
      servo(up1Pin, button0_Up1);
      servo(up2Pin, button0_Up2);
      servo(gpPin, button0_Gp);
      EEPROM.write(13, up1);
      EEPROM.write(14, up2);
      EEPROM.write(15, gp);
      tone(9, 880, 100);
      delay(500);
      break;
    }
    Serial.print(up1);
    Serial.print(", ");
    Serial.print(up2);
    Serial.print(", ");
    Serial.print(gp);
    Serial.print(", ");
    Serial.println(b);
    if (b == 3 || b == 4)
      break;
  }
  FlexiTimer2::stop();
  button0_Up1 = EEPROM.read(10);
  button0_Up2 = EEPROM.read(11);
  button0_Gp = EEPROM.read(12);
  button11_Up1 = EEPROM.read(13);
  button11_Up2 = EEPROM.read(14);
  button11_Gp = EEPROM.read(15);
  if (button0_Up1 >= 255)
  {
    button0_Up1 = 90;
    button0_Up2 = 90;
    button0_Gp = 90;
    button11_Up1 = 90;
    button11_Up2 = 90;
    button11_Gp = 90;
  }
  tone(9, 220, 100);
  delay(200);
  tone(9, 220, 100);
  delay(200);
}

void setup()
{
  setRFPassWordPin(0, 1919810);
  Serial.begin(115200);
  Serial.println("start");
  pinMode(9, OUTPUT);
  M1PID.SetMode(AUTOMATIC);         // 设置PID为自动模式
  M1PID.SetSampleTime(pidwm);       // 设置PID采样频率为pidwm ms
  M2PID.SetMode(AUTOMATIC);         // 设置PID为自动模式
  M2PID.SetSampleTime(pidwm);       // 设置PID采样频率为pidwm ms
  M3PID.SetMode(AUTOMATIC);         // 设置PID为自动模式
  M3PID.SetSampleTime(pidwm);       // 设置PID采样频率为pidwm ms
  M4PID.SetMode(AUTOMATIC);         // 设置PID为自动模式
  M4PID.SetSampleTime(pidwm);       // 设置PID采样频率为pidwm ms
  M1PID.SetOutputLimits(-255, 255); // PID输出值设置为-255~255
  M2PID.SetOutputLimits(-255, 255);
  M3PID.SetOutputLimits(-255, 255);
  M4PID.SetOutputLimits(-255, 255);

  button0_Up1 = EEPROM.read(10);
  button0_Up2 = EEPROM.read(11);
  button0_Gp = EEPROM.read(12);
  button11_Up1 = EEPROM.read(13);
  button11_Up2 = EEPROM.read(14);
  button11_Gp = EEPROM.read(15);

  if (button0_Up1 >= 255)
  {
    button0_Up1 = 90;
    button0_Up2 = 90;
    button0_Gp = 90;
    button11_Up1 = 90;
    button11_Up2 = 90;
    button11_Gp = 90;
  }
  up1 = button11_Up1;
  up2 = button11_Up2;
  gp = button11_Gp;

  up1 = up1 > 180 ? 90 : up1;
  up2 = up2 > 180 ? 90 : up2;
  gp = gp > 180 ? 90 : gp;

  joyStickData[1] = 3;
  t = millis();
  pinMode(up1Pin, OUTPUT); // 设定舵机接口为输出接口
  pinMode(up2Pin, OUTPUT); // 设定舵机接口为输出接口
  pinMode(gpPin, OUTPUT);  // 设定舵机接口为输出接口
  Serial.println("init OK");
}

void loop()
{
  unsigned long joyStickInput = getRFModuleRemoteCodePin(0);
  if (joyStickInput == 294) // 294 = 2^1 + 2^2 + 2^5 + 2^8
    setting();              // 当1,2,5,8号按钮同时按下时进入设置模式
  else
  {
    if (joyStickInput == 1) // 0号按钮
    {
      if (button11_Gp > button0_Gp)
        servo(gpPin, button11_Gp + 7);
      else
        servo(gpPin, button11_Gp - 7);
      delay(300);
      servo(up1Pin, button0_Up1);
      servo(up2Pin, button0_Up2);
    }

    if (joyStickInput == 2048) // 11号按钮
    {
      servo(gpPin, button0_Gp);
      delay(300);
      servo(up1Pin, button11_Up1);
      servo(up2Pin, button11_Up2);
    }

    if (joyStickInput & 2) // 1号按钮
    {
      servo(up1Pin, button0_Up1);
      servo(up2Pin, button0_Up2);
    }
    if (joyStickInput & 4) // 2号按钮
    {
      servo(gpPin, button0_Gp);
    }
    if (joyStickInput & 8) // 3号按钮
    {
      servo(up1Pin, button11_Up1);
      servo(up2Pin, button11_Up2);
    }
    if (joyStickInput == 16) // 4号按钮
    {
      if (button11_Gp > button0_Gp)
        servo(gpPin, button11_Gp + 7);
      else
        servo(gpPin, button11_Gp - 7);
      delay(500);
    }
  }
  int x = 0;
  if (millis() > t)
  {
    Speed();
    t = millis() + pidwm;
    x = 1;
  }

  /****************计算四个电机的参考转速********************/
  int godata = getRFModuleRemoteRockerPin(0, ROCKER_LEFT, ROCKER_Y);
  joyStickData[2] = 0;
  if (godata != 999)
    joyStickData[2] = godata / 2;
  if ((joyStickData[2] > L) || (joyStickData[2] < R))
  {
    mafor = map(joyStickData[2], -100, 100, -maxspeed, maxspeed);
    mbfor = map(joyStickData[2], -100, 100, -maxspeed, maxspeed);
    mcfor = map(joyStickData[2], -100, 100, -maxspeed, maxspeed);
    mdfor = map(joyStickData[2], -100, 100, -maxspeed, maxspeed);
  }
  else
    mafor = mbfor = mcfor = mdfor = 0;
  joyStickData[1] = 0;


  int piaoyi = getRFModuleRemoteRockerPin(0, ROCKER_RIGHT, ROCKER_X);
  if (piaoyi != 999)
    joyStickData[1] = piaoyi / 2;
  if ((joyStickData[1] > L) || (joyStickData[1] < R))
  {
    mago = map(joyStickData[1], -100, 100, -maxspeed, maxspeed);
    mbgo = map(joyStickData[1], -100, 100, maxspeed, -maxspeed);
    mcgo = map(joyStickData[1], -100, 100, -maxspeed, maxspeed);
    mdgo = map(joyStickData[1], -100, 100, maxspeed, -maxspeed);
  }
  else
    mago = mbgo = mcgo = mdgo = 0;

  joyStickData[4] = 0;

  int turndata = getRFModuleRemoteRockerPin(0, ROCKER_LEFT, ROCKER_X);
  if (turndata != 999)
    joyStickData[4] = -turndata / 2;

  if (L < joyStickData[4] || joyStickData[4] < R)
  {
    maturn = map(joyStickData[4], -100, 100, 70, -70);
    mbturn = map(joyStickData[4], -100, 100, -70, 70);
    mcturn = map(joyStickData[4], -100, 100, -70, 70);
    mdturn = map(joyStickData[4], -100, 100, 70, -70);
  }
  else
    maturn = mbturn = mcturn = mdturn = 0;

  ref1 = max(min(mafor + mago + maturn, maxspeed), -maxspeed);
  ref2 = max(min(mbfor + mbgo + mbturn, maxspeed), -maxspeed);
  ref3 = max(min(mcfor + mcgo + mcturn, maxspeed), -maxspeed);
  ref4 = max(min(mdfor + mdgo + mdturn, maxspeed), -maxspeed);
  /*************计算4个电机的参考转速*******************/
  setspeed();
  moving();
  Serial.println(joyStickInput);
  if (x)
  {
    Serial.print(ref1);
    Serial.print(", ");
    Serial.print(ref2);
    Serial.print(", ");
    Serial.print(ref3);
    Serial.print(", ");
    Serial.print(ref4);

    Serial.print(", m1s:");
    Serial.print(M1S);
    Serial.print(", ");
    Serial.print(M2S);
    Serial.print(", ");
    Serial.print(M3S);
    Serial.print(", ");
    Serial.print(M4S);
    Serial.print(", ");
    Serial.println(joyStickInput);

    if ((joyStickData[1] < L) && (joyStickData[1] > R) && (joyStickData[2] < L) && (joyStickData[2] > R) && (joyStickData[4] < L) && (joyStickData[4] > R))
      stop_all();
  }
}
