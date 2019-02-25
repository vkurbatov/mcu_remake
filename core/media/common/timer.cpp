#include "timer.h"

#include <thread>

namespace core
{

namespace media
{

using clock = std::chrono::high_resolution_clock;

void Timer::Sleep(std::int32_t period_ms)
{
	std::this_thread::sleep_for(std::chrono::milliseconds(period_ms));
}

Timer::Timer()
	: m_start(false)
{

}

void Timer::Start(std::int32_t period_ms)
{
	m_start = true;
	m_target_point = clock::now() + std::chrono::milliseconds(period_ms);
}

void Timer::Stop()
{
	m_start = false;
}

bool Timer::IsStart() const
{
	return m_start;
}

bool Timer::IsEnable() const
{
	return m_start && clock::now() < m_target_point;
}

std::int32_t Timer::Elapsed() const
{
	return m_start
			? std::chrono::duration_cast<std::chrono::milliseconds>(m_target_point - clock::now()).count()
			: 0;
}

void Timer::Delay(std::int32_t delay_ms, bool is_wait)
{
	if (!m_start)
	{
		Start(delay_ms);
	}
	else
	{
		m_target_point += std::chrono::milliseconds(delay_ms);
	}

	if (is_wait)
	{
		std::this_thread::sleep_for(m_target_point - clock::now());
	}
}

void Timer::operator()(int32_t delay_ms, bool is_wait)
{
	Delay(delay_ms, is_wait);
}

} // media

} // core
