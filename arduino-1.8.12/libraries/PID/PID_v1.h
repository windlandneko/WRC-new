/*** Translated by Charlie ***/
#ifndef PID_v1_h
#define PID_v1_h
#define LIBRARY_VERSION 1.1.1

class PID
{
public:
// 这些常量在下面的一些函数里有使用
#define AUTOMATIC 1
#define MANUAL 0
#define DIRECT 0
#define REVERSE 1
#define P_ON_M 0
#define P_ON_E 1
  //常用函数 **************************************************************************

  /** @p 初始化 PID 控制
   * @param *Input 输入参数，待控制的量
   * @param *Output 输出参数，指经过 PID 得到的输出
   * @param *Setpoint 目标值，希望达到的数值
   * @param Kp 比例因子 (P)roportional
   * @param Ki 积分因子 (I)ntegral
   * @param Kd 微分因子 (D)erivative
   * @param POn 默认为普通模式，输入非 0 值可防止超调，但调节时间会延长
   * @param ControllerDirection 是否将输出符号反转，正常置 0 即可
   */
  PID(double *, double *, double *, double, double, double, int, int);
  /** @p 初始化 PID 控制
   * @param *Input 输入参数，待控制的量
   * @param *Output 输出参数，指经过 PID 得到的输出
   * @param *Setpoint 目标值，希望达到的数值
   * @param Kp 比例因子 (P)roportional
   * @param Ki 积分因子 (I)ntegral
   * @param Kd 微分因子 (D)erivative
   * @param ControllerDirection 是否将输出符号反转，正常置 0 即可
   */
  PID(double *, double *, double *, double, double, double, int);
  /**
   * @p 切换控制模式
   * @param Mode 停止（手动模式） (0) ；自动控制 (非0)
   */
  void SetMode(int Mode);
  /*
  * 这，就是魔法（
  * - 此函数应在每次 loop() 循环时被调用，若无需计算，返回值为 false ；若计算了新的值，返回值为 true
  * - 开关状态与计算周期可以分别使用 SetMode 和 SetSampleTimeperforms 进行设置
  */
  bool Compute();
  /**
   * @p 设置输出值范围，默认为 0-255
   * @param Min 最小值
   * @param Max 最大值
   */
  void SetOutputLimits(double, double);

  //并不怎么常用的函数 ******************************************************************

  /**
   * @p 调整 PID 控制参数。虽然正常人都会在构造函数中给参数，但还是有人要中途调参...可以用这个函数
   * @param Kp 比例因子
   * @param Ki 积分因子
   * @param Kd 微分因子
   */
  void SetTunings(double, double, double); // * 虽然正常人都会在构造函数中给参数，但还是有人要中途调参...可以用这个函数

  /**
   * @p 调整 PID 控制参数，如果要改变 POn 的值就用这个
   * @param Kp 比例因子
   * @param Ki 积分因子
   * @param Kd 微分因子
   * @param POn 控制模式
   */
  void SetTunings(double, double, double, int);

  void SetControllerDirection(int); // * Sets the Direction, or "Action" of the controller. DIRECT
                                    //   means the output will increase when error is positive. REVERSE
                                    //   means the opposite.  it's very unlikely that this will be needed
                                    //   once it is set in the constructor.
  /**
   * @p 调整采样时间
   * @param NewSampleTime 以毫秒为单位的采样时间
   */
  void SetSampleTime(int);

  //Display functions
  double GetKp();     // These functions query the pid for interal values.
  double GetKi();     //  they were created mainly for the pid front-end,
  double GetKd();     // where it's important to know what is actually
  int GetMode();      //  inside the PID.
  int GetDirection(); //

private:
  void Initialize();

  double dispKp; // * we'll hold on to the tuning parameters in user-entered
  double dispKi; //   format for display purposes
  double dispKd; //

  double kp; // * (P)roportional Tuning Parameter
  double ki; // * (I)ntegral Tuning Parameter
  double kd; // * (D)erivative Tuning Parameter

  int controllerDirection;
  int pOn;

  double *myInput;    // * Pointers to the Input, Output, and Setpoint variables
  double *myOutput;   //   This creates a hard link between the variables and the
  double *mySetpoint; //   PID, freeing the user from having to constantly tell us
                      //   what these values are.  with pointers we'll just know.

  unsigned long lastTime;
  double outputSum, lastInput;

  unsigned long SampleTime;
  double outMin, outMax;
  bool inAuto, pOnE;
};
#endif