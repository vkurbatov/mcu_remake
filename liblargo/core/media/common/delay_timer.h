#ifndef TIMER_H
#define TIMER_H

#include <chrono>

namespace core
{

namespace media
{

class DelayTimer
{
	std::chrono::time_point<std::chrono::system_clock>		m_target_point;
	bool													m_started;

public:

	static void Sleep(std::int32_t period_ms);

	DelayTimer();

	void Start(std::int32_t period_ms);
	void Stop();

	bool IsStart() const;
	bool IsEnabled() const;
	bool IsWait() const;
	std::int32_t Elapsed() const;

	void Delay(std::int32_t delay_ms, bool is_wait = true);
	void operator()(std::int32_t delay_ms, bool is_wait = true);
};

} // media

} // core

#endif // TIMER_H
