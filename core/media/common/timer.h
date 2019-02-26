#ifndef TIMER_H
#define TIMER_H

#include <chrono>

namespace core
{

namespace media
{

class Timer
{
	std::chrono::time_point<std::chrono::system_clock>		m_target_point;
	bool													m_startad;

public:

	static void Sleep(std::int32_t period_ms);

	Timer();

	void Start(std::int32_t period_ms);
	void Stop();

	bool IsStart() const;
	bool IsEnable() const;
	std::int32_t Elapsed() const;

	void Delay(std::int32_t delay_ms, bool is_wait = true);
	void operator()(std::int32_t delay_ms, bool is_wait = true);
};

} // media

} // core

#endif // TIMER_H
