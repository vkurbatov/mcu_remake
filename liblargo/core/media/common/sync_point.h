#ifndef SYNC_POINT_H
#define SYNC_POINT_H

#include "media/common/i_sync_point.h"

#include <mutex>

namespace core
{

namespace media
{

class SyncPoint : public ISyncPoint
{
	mutable std::mutex	m_mutex;
	const bool			m_is_fake;

public:
	SyncPoint(bool is_fake = false);

	// ISyncPoint interface
public:
	void Lock() const override;
	void Unlock() const override;

public:
	bool IsFake() const;
};

} // media

} // core

#endif // SYNC_POINT_H
