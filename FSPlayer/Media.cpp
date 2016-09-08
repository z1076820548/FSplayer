
# include "Media.h"


extern bool quit;

MediaState::MediaState(char* input_file)
	:filename(input_file)
{
	pFormatCtx = nullptr;
	audio = new AudioState();

	//quit = false;
}

MediaState::~MediaState()
{
	if(audio)
		delete audio;
}

bool MediaState::openInput()
{
	// Open input file
	if (avformat_open_input(&pFormatCtx, filename, nullptr, nullptr) < 0)
		return false;

	if (avformat_find_stream_info(pFormatCtx, nullptr) < 0)
		return false;

	// Output the stream info to standard 
	av_dump_format(pFormatCtx, 0, filename, 0);

	for (uint32_t i = 0; i < pFormatCtx->nb_streams; i++)
	{
		if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO)
		{
			audio->audio_stream = i;
			break;
		}
	}

	if (audio->audio_stream < 0)
		return false;

	// Find the decoder
	AVCodec *pCodec = avcodec_find_decoder(pFormatCtx->streams[audio->audio_stream]->codec->codec_id);
	if (!pCodec)
		return false;

	audio->audio_ctx = avcodec_alloc_context3(pCodec);
	if (avcodec_copy_context(audio->audio_ctx, pFormatCtx->streams[audio->audio_stream]->codec) != 0)
		return false;

	avcodec_open2(audio->audio_ctx, pCodec, nullptr);

	return true;
}

int decode_thread(void *data)
{
	MediaState *media = (MediaState*)data;

	AVPacket packet;
	while (true)
	{
		if (av_read_frame(media->pFormatCtx, &packet) < 0)
		{
			if (media->pFormatCtx->pb->error == 0) // No error,wait for user input
			{
				SDL_Delay(100);
				continue;
			}
			else
				break;
		}

		if (packet.stream_index == media->audio->audio_stream)
			media->audio->audioq.enQueue(&packet);
		else
			av_packet_unref(&packet);
	}

	// All done,wait for it
	while (!quit)
		SDL_Delay(100);

	return 0;
}