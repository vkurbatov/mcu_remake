#ifndef I_PROCESS_STATE_NOTIFIER_H
#define I_PROCESS_STATE_NOTIFIER_H


namespace core
{

namespace media
{

enum class ProcessState
{
	init,
	start,
	stop,
	pause,
};

class IProcessStateNotifier
{
public:
	virtual void StateChangeNotify(const ProcessState& new_state, const ProcessState& old_state, void* context = nullptr) = 0;
};

} // media

} // core

#endif // I_PROCESS_STATE_NOTIFIER_H
