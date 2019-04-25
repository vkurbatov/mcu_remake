#include "libav_wrapper.h"
#include <cstring>

extern "C"
{
#include <libavcodec/avcodec.h>
}


namespace largo
{

namespace codec
{

namespace audio
{

struct libav_context_t
{
	AVCodec *			codec;
	AVCodecContext *	codec_context;
	AVFrame *			frame;
	AVPacket *			packet;

};

namespace utils
{

template<typename T1, typename T2>
void set_av_param(T1& param, const T2& default_store_value, const T2& new_value)
{
	if (new_value != default_store_value)
	{
		param = static_cast<T1>(new_value);
	}
}

AVCodecID get_av_codec_id(const audio_codec_id_t& codec_id)
{
	AVCodecID result = AV_CODEC_ID_NONE;

	switch(codec_id)
	{
		case audio_codec_id_t::audio_codec_g723_1:
			result = AV_CODEC_ID_G723_1;
		break;
		case audio_codec_id_t::audio_codec_aac_ld:
			result = AV_CODEC_ID_AAC;
		break;
	}

	return result;
}

AVSampleFormat get_av_sample_format(sample_format_t sample_format)
{
	AVSampleFormat result = AV_SAMPLE_FMT_NONE;

	switch(sample_format)
	{
		case sample_format_t::pcm_8:
			result = AV_SAMPLE_FMT_U8;
		break;
		case sample_format_t::pcm_16:
			result = AV_SAMPLE_FMT_S16;
		break;
		case sample_format_t::pcm_32:
			result = AV_SAMPLE_FMT_S32;
		break;
		case sample_format_t::float_32:
			result = AV_SAMPLE_FMT_FLT;
		break;
		case sample_format_t::float_64:
			result = AV_SAMPLE_FMT_DBL;
		break;
	}

	return result;
}

void init_context(AVCodecID av_codec_id, libav_context_t& context, const libav_codec_config_t& libav_codec_config)
{
	context.codec_context->bit_rate = libav_codec_config.bit_rate;
	context.codec_context->sample_fmt = get_av_sample_format(libav_codec_config.sample_format);
	context.codec_context->sample_rate = libav_codec_config.sample_rate;
	context.codec_context->channel_layout = AV_CH_LAYOUT_MONO;
	context.codec_context->channels = 1;
	context.codec_context->frame_size = libav_codec_config.frame_size;
	context.codec_context->profile = libav_codec_config.profile;
	context.codec_context->flags |= CODEC_FLAG_GLOBAL_HEADER | AV_CODEC_FLAG_QSCALE | AV_CODEC_FLAG_LOW_DELAY;

	context.frame->channels = context.codec_context->channels;
	context.frame->channel_layout = context.codec_context->channel_layout;
	context.frame->format = context.codec_context->sample_fmt;
	context.frame->sample_rate = context.codec_context->sample_rate;
	context.frame->nb_samples = context.codec_context->frame_size;
}


libav_context_t* create_context(AVCodecID codec_id, bool is_encoder, const libav_codec_config_t& libav_codec_config)
{

	static bool libav_register_flag = false;

	if (libav_register_flag == false)
	{
		avcodec_register_all();
		libav_register_flag = true;
	}


	libav_context_t* context = nullptr;

	AVCodec* codec = is_encoder ? avcodec_find_encoder(codec_id) : avcodec_find_decoder(codec_id);
	if (codec != nullptr)
	{
		AVCodecContext * codec_context = avcodec_alloc_context3(codec);

		if (codec_context != nullptr)
		{
			AVFrame* frame = av_frame_alloc();

			if (frame != nullptr)
			{
				AVPacket* packet = av_packet_alloc();

				if (packet != nullptr)
				{
					av_init_packet(packet);

					context = new libav_context_t;

					if (context != nullptr)
					{
						context->codec = codec;
						context->codec_context = codec_context;
						context->frame = frame;
						context->packet = packet;

						init_context(codec_id, *context, libav_codec_config);

						return context;
					}

					av_packet_free(&packet);
				}

				av_frame_free(&frame);
			}

			av_free(codec_context);
		}
	}

	return context;
}

void destroy_context(libav_context_t* context)
{
	if (context != nullptr)
	{
		avcodec_free_context(&context->codec_context);
		av_frame_free(&context->frame);
		av_packet_free(&context->packet);
		delete context;
	}
}

std::int32_t encoder(libav_context_t * context,
						 void* input_data,
						 std::size_t input_size,
						 void* output_data,
						 std::size_t output_size)
{
	std::int32_t result = -1;
	std::int32_t got_packet = 0;

	if (context != NULL)
	{
		if (input_data != NULL && output_data != NULL)
		{
			context->frame->nb_samples = input_size / av_get_bytes_per_sample(context->codec_context->sample_fmt);

			/*auto buff_size = av_samples_get_buffer_size(NULL, context->codec_context->channels, context->codec_context->frame_size,
																				 context->codec_context->sample_fmt, 1);*/

			result = avcodec_fill_audio_frame(context->frame,
											  context->codec_context->channels,
											  context->codec_context->sample_fmt,
											  static_cast<const std::uint8_t*>(input_data),
											  input_size,
											  1);

			if (result >= 0)
			{
				av_init_packet(context->packet);

				context->packet->data = static_cast<std::uint8_t*>(output_data);
				context->packet->size = static_cast<std::int32_t>(output_size);

				result = avcodec_encode_audio2(context->codec_context,
											   context->packet,
											   context->frame,
											   &got_packet);

				if (result >= 0)
				{

					result = got_packet != 0 ? context->packet->size : 0;

				}

			}
		}

	}

	return result;
}

std::int32_t decoder(libav_context_t* context,
						 void* input_data,
						 std::size_t input_size,
						 void* output_data,
						 std::size_t output_size)
{
	std::int32_t result = -1;
	std::int32_t got_packet = 0;

	if (context != NULL)
	{
		if (input_data != NULL && output_data != NULL)
		{
			// avcodec_get_frame_defaults(context->frame);

			av_init_packet(context->packet);

			context->packet->data = static_cast<std::uint8_t*>(input_data);
			context->packet->size = static_cast<std::int32_t>(input_size);

			result = avcodec_decode_audio4(context->codec_context,
										   context->frame,
										   &got_packet,
										   context->packet);

			if (result >= 0)
			{
				if (got_packet)
				{

					result = (context->frame->nb_samples * av_get_bytes_per_sample(context->codec_context->sample_fmt));

					if (result > output_size)
					{
						result = output_size;
					}

					std::memcpy(output_data, context->frame->data[0], result);
				}
				else
				{
					result = 0;
				}

			}
		}

	}

	return result;
}

std::int32_t codec_open(libav_context_t* context)
{
	std::int32_t result = -EINVAL;

	if (context != nullptr)
	{
		if (avcodec_is_open(context->codec_context) > 0)
		{
			result = 0;
		}
		else
		{
			result = avcodec_open2(context->codec_context, context->codec, nullptr);
		}

		// result = avcodec_is_open
	}

	return result;
}


std::int32_t codec_close(libav_context_t* context)
{
	std::int32_t result = -EINVAL;

	if (context != nullptr)
	{
		if (avcodec_is_open(context->codec_context) > 0)
		{
			result = avcodec_close(context->codec_context);
		}
		else
		{
			result = 0;
		}
	}

	return result;
}

} // libav_utils

LibavWrapper::LibavWrapper(audio_codec_id_t codec_id
				 , const libav_codec_config_t& config
				 , bool is_encoder)
	: m_codec_id(codec_id)
	, m_is_encoder(is_encoder)
	, m_codec_config(config)
	, m_context(
		  utils::create_context(
			  utils::get_av_codec_id(codec_id)
			  , is_encoder
			  ,  config),
		  [](libav_context_t *ctx) { utils::destroy_context(ctx); })
{

}

LibavWrapper::~LibavWrapper()
{
	Close();
}

bool LibavWrapper::Open()
{

	Close();

	m_is_open = utils::codec_open(m_context.get()) >= 0;

	return m_is_open;

}

bool LibavWrapper::Close()
{

	m_is_open = false;

	return utils::codec_close(m_context.get()) >= 0;

}

bool LibavWrapper::IsOpen() const
{
	return m_is_open;
}

bool LibavWrapper::IsEncoder() const
{
	return m_is_encoder;
}

int32_t LibavWrapper::Transcode(const void* input_data, std::size_t input_size, void* output_data, std::size_t output_size)
{
	std::int32_t result = -EINVAL;

	if (IsOpen())
	{
		if (m_buffer.size() < input_size)
		{
			m_buffer.resize(input_size);
		}

		std::memcpy(m_buffer.data(), input_data, input_size);

		auto transcode_proc = IsEncoder() ? utils::encoder : utils::decoder;

		result = transcode_proc(m_context.get(), m_buffer.data(), input_size, output_data, output_size);

	}

	return result;
}

void LibavWrapper::SetConfig(const libav_codec_config_t& config)
{
	bool need_open = IsOpen();

	Close();

	m_codec_config = config;

	utils::init_context(utils::get_av_codec_id(m_codec_id), *m_context.get(), m_codec_config);

	if (need_open)
	{
		Open();
	}
}

const libav_codec_config_t& LibavWrapper::GetConfig() const
{
	return m_codec_config;
}

audio_codec_id_t LibavWrapper::GetCodecId() const
{
	return m_codec_id;
}

} // audio

} // codec

} // largo
