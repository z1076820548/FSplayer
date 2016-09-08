
#ifndef FRAME_QUEUE_H
#define FRAME_QUEUE_H

#include <SDL_thread.h>
#include <queue>

extern "C"{

#include <libavcodec\avcodec.h>

}

struct FrameQueue
{
	std::queue<AVFrame*> queue;

	uint32_t nb_frames;

	SDL_mutex* mutex;
	SDL_cond * cond;

	FrameQueue();
	bool enQueue(const AVFrame* frame);
	bool deQueue(AVFrame *frame);
};



#endif