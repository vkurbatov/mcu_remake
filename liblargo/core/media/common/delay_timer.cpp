#include "delay_timer.h"

#include <thread>

namespace core
{

namespace media
{

using clock = std::chrono::high_resolution_clock;

void DelayTimer::Sleep(std::int32_t period_ms)
{
	std::this_thread::sleep_for(std::chrono::milliseconds(period_ms));
}

DelayTimer::DelayTimer()
	: m_started(false)
{

}

void DelayTimer::Start(std::int32_t period_ms)
{
	m_started = true;
	m_target_point = clock::now() + std::chrono::milliseconds(period_ms);
}

void DelayTimer::Stop()
{
	m_started = false;
}

bool DelayTimer::IsStart() const
{
	return m_started;
}

bool DelayTimer::IsEnabled() const
{
	return m_started && clock::now() >= m_target_point;
}

bool DelayTimer::IsWait() const
{
	return IsStart() && !IsEnabled();
}

std::int32_t DelayTimer::Elapsed() const
{
	return m_started
	       ? std::chrono::duration_cast<std::chrono::milliseconds>(m_target_point - clock::now()).count()
	       : 0;
}

void DelayTimer::Delay(std::int32_t delay_ms, bool is_wait)
{
	if (!m_started)
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

void DelayTimer::operator()(int32_t delay_ms, bool is_wait)
{
	Delay(delay_ms, is_wait);
}

} // media

} // core
