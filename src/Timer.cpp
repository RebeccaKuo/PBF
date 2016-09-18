#include"Timer.h"

Timer* Timer::m_current_time = 0;

Timer::Timer()
{
  m_time = 0;
  m_timeStep = 0.025;
  m_minStep = 0.03;
  m_maxStep = 0.05;
}

Timer::~Timer()
{
  m_current_time = 0;
}

float Timer::getTime()
{
  return m_time;
}

void Timer::setTime(float _time)
{
  m_time = _time;
}

float Timer::getStepSize()
{
  return m_timeStep;
}

float Timer::getMaxStepSize()
{
  return m_maxStep;
}

float Timer::getMinStepSize()
{
  return m_minStep;
}

void Timer::setStepSize(float _step)
{
  m_timeStep = _step;
}

Timer* Timer::getCurrent()
{
  if( m_current_time == 0 )
  {
    m_current_time = new Timer();
  }
  return m_current_time;
}

void Timer::setCurrent(Timer* _tm)
{
  m_current_time = _tm;
}

bool Timer::existCurrent()
{
  return (m_current_time != 0);
}
