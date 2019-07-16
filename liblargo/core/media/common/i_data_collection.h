#ifndef I_DATA_COLLECTION_H
#define I_DATA_COLLECTION_H

#include <cstdint>

namespace core
{

namespace media
{

class IDataCollection
{
public:
	virtual ~IDataCollection() {}

	virtual std::size_t Count() const = 0;
};

}

}

#endif // I_DATA_COLLECTION_H
