#ifndef AUDIO_STRING_FORMAT_UTILS_H
#define AUDIO_STRING_FORMAT_UTILS_H

#include "media/audio/audio_format.h"
#include "media/audio/channels/audio_channel_params.h"

#include <ostream>

std::ostream& operator<<(std::ostream& ostream, const core::media::audio::audio_format_t& audio_format);
std::ostream& operator<<(std::ostream& ostream, const core::media::audio::audio_format_t::sample_format_t& sample_format);
std::ostream& operator<<(std::ostream& ostream, const core::media::audio::channels::audio_channel_params_t& channel_params);
std::ostream& operator<<(std::ostream& ostream, const core::media::audio::channels::channel_direction_t& direction);

#endif // AUDIO_STRING_FORMAT_UTILS_H
