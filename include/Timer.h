/*
 * InteractiveComputerGraphics - TimeManager
*/

#ifndef _TIMER_H_
#define _TIMER_H_

class Timer
{
public:
  Timer();
  ~Timer();

  float getTime();
  void setTime(float _time);
  float getStepSize();
  float getMaxStepSize();
  float getMinStepSize();
  void setStepSize(float _step);

  static Timer*  getCurrent();
  static void setCurrent(Timer* _tm);
  static bool existCurrent();

private:
  float m_timeStep;
  float m_time;
  float m_minStep;
  float m_maxStep;
  static Timer* m_current_time;

};

#endif
