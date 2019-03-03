#ifndef AUDIO_MUX_H
#define AUDIO_MUX_H

#include "media/common/i_data_collection.h"
#include "media/audio/i_audio_point.h"
#include "media/audio/i_audio_formatter.h"

#include <memory>
#include <list>

namespace core
{

namespace media
{

namespace audio
{

class AudioMux: public IAudioReader, public IDataCollection
{
	using audio_reader_t = std::shared_ptr<IAudioReader>;
	using audio_reader_list_t = std::list<audio_reader_t>;

	IAudioReader&		m_audio_reader;

public:
	AudioMux(IAudioReader& audio_reader);

	bool AddReader(IAudioReader* audio_reader);
	bool RemoveReader(IAudioReader* audio_reader);

};

} // audio

} // media

} // core

#endif // AUDIO_MUX_H
