#pragma once

// ---------- Includes ------------
#include <time.h>
#include "../Utils/Utils.h"

// ------ Struct declaration -------

typedef struct _Event
{
	int ms_min, ms_max;

	clock_t start;
	int ticks;
	int elapsed;

	bool state;

}	Event;


// --------- Constructors ---------

Event *
event_new (int min, int max);

Event *event_alloc();

// ----------- Functions ------------
void event_init (Event *event, int min, int max);
void event_start (Event *event, clock_t now);
void event_start_now (Event *event);
bool event_is_started (Event *event);
bool event_pulse (Event *event);
bool event_update (Event *event);
void event_stop (Event *event);
void event_restart (Event *this, int ms_min, int ms_max, clock_t now);
void event_restart_now (Event *this, int ms_min, int ms_max);
bool event_done (Event *event);
void event_set_done (Event *event);
clock_t event_get_now ();
int event_elapsed (Event *this, clock_t now);
int event_elapsed_now (Event *this);

// --------- Destructors ----------

void
event_free (Event *event);


