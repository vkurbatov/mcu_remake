#ifndef I_SYNC_POINT_H
#define I_SYNC_POINT_H

namespace core
{

namespace media
{

class ISyncPoint
{
public:
	virtual ~ISyncPoint() = default;

	virtual void Lock() const = 0;
	virtual void Unlock() const = 0;
};

} // media

} // core

#endif // I_SYNC_POINT_H
