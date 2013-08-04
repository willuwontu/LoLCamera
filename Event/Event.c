#include "Event.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

Event *event_new (int ms_min, int ms_max)
{
	Event *this;

	if ((this = event_alloc()) == NULL)
		return NULL;

	event_init(this, ms_min, ms_max);

	return this;
}

Event *event_alloc ()
{
	return calloc (1, sizeof(Event));
}

bool event_is_started (Event *this)
{
    return (this->start != 0 && this->ticks != 0);
}

void event_init (Event *this, int ms_min, int ms_max)
{
	this->ms_min = ms_min;
	this->ms_max = ms_max;

	this->state = FALSE;
}

void event_restart (Event *this, int ms_min, int ms_max, clock_t now)
{
    event_init(this, ms_min, ms_max);
    event_start(this, now);
}

void event_restart_now (Event *this, int ms_min, int ms_max)
{
    clock_t now = event_get_now();
    event_restart(this, ms_min, ms_max, now);
}

void event_start_now (Event *this)
{
    clock_t now = event_get_now();
    event_start(this, now);
}

void event_start (Event *this, clock_t now)
{
    int time_distance = this->ms_max - this->ms_min;
    int rand_time = ((float)rand() / RAND_MAX) * time_distance;

    this->start = now;
    this->ticks = this->ms_min + rand_time;
}

void event_stop (Event *this)
{
    this->start = 0;
    this->ticks = 0;
}

clock_t event_get_now ()
{
	clock_t res;

	#ifdef WIN32
		res = clock() * 1000 / CLOCKS_PER_SEC;
	#else
    #include <sys/times.h>
		static struct tms buf;
		res = (times(&buf) * 10);
    #endif

    return res;
}

static void event_tick (Event *this, clock_t now)
{
	event_start(this, now);

	// Signal = reverse the state
	this->state = !this->state;
}

int event_elapsed (Event *this, clock_t now)
{
	return now - this->start;
}

int event_elapsed_now (Event *this)
{
	clock_t now = event_get_now();
	return event_elapsed(this, now);
}

bool event_update (Event *this)
{
	clock_t now = event_get_now();

    this->elapsed = event_elapsed(this, now);

    if (this->elapsed > this->ms_max)
    {
    	event_tick(this, now);
        return TRUE;
    }

    if (this->elapsed < this->ms_min)
        return FALSE;

    // If the event emits a signal, reset the start time for the next pulse
    bool res;

    if ((res = event_pulse(this)))
        event_tick(this, now);

    return res;
}

void event_set_done (Event *this)
{
    this->state = TRUE;
}

bool event_done (Event *this)
{
    return this->state;
}

bool event_pulse (Event *this)
{
    return (this->elapsed > this->ticks);
}

void event_free (Event *event)
{
	if (event != NULL)
	{
		free (event);
	}
}
