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
	: m_startad(false)
{

}

void DelayTimer::Start(std::int32_t period_ms)
{
	m_startad = true;
	m_target_point = clock::now() + std::chrono::milliseconds(period_ms);
}

void DelayTimer::Stop()
{
	m_startad = false;
}

bool DelayTimer::IsStart() const
{
	return m_startad;
}

bool DelayTimer::IsEnable() const
{
	return m_startad && clock::now() < m_target_point;
}

std::int32_t DelayTimer::Elapsed() const
{
	return m_startad
			? std::chrono::duration_cast<std::chrono::milliseconds>(m_target_point - clock::now()).count()
			: 0;
}

void DelayTimer::Delay(std::int32_t delay_ms, bool is_wait)
{
	if (!m_startad)
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
