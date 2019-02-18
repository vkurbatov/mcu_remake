#include "media_slot.h"

namespace core
{

namespace media
{

MediaSlot::MediaSlot(media_slot_id_t media_slot_id,
					 IMultipointDataQueue& multipoint_data_queue,
					 cursor_t cursor)
	: m_media_slot_id(media_slot_id)
	, m_multipoint_data_queue(multipoint_data_queue)
	, m_cursor(cursor)
{

}

media_slot_id_t MediaSlot::GetSlotId() const
{
	return m_media_slot_id;
}

} //media

} //core
