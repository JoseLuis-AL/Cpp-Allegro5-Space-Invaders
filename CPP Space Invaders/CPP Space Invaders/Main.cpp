#include <cstdio>
#include <allegro5/allegro5.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>

// GLOBAL.
// =============================================================================
long frames;
long score;

// Utilities.
// =============================================================================
void MustInit(bool test, const char* description)
{
	if (test) return;

	printf("ERROR: Couldn't initialize %s\n", description);
	exit(1);
}

// Display.
// =============================================================================
#define BUFFER_W 256
#define BUFFER_H 224

#define DISPLAY_SCALE 3
#define DISPLAY_W (BUFFER_W * DISPLAY_SCALE)
#define DISPLAY_H (BUFFER_H * DISPLAY_SCALE)

ALLEGRO_DISPLAY* display;
ALLEGRO_BITMAP* buffer;

void DisplayInit()
{
	al_set_new_display_option(ALLEGRO_SAMPLE_BUFFERS, 1, ALLEGRO_SUGGEST);
	al_set_new_display_option(ALLEGRO_SAMPLES, 8, ALLEGRO_SUGGEST);

	display = al_create_display(DISPLAY_W, DISPLAY_H);
	MustInit(display, "display");

	buffer = al_create_bitmap(BUFFER_W, BUFFER_H);
	MustInit(buffer, "buffer");
}

void DisplayDeinit()
{
	al_destroy_display(display);
	al_destroy_bitmap(buffer);
}

void DisplayPreDraw()
{
	al_set_target_bitmap(buffer);
}

void DisplayPostDraw()
{
	al_set_target_backbuffer(display);
	al_draw_scaled_bitmap(buffer, 0, 0, BUFFER_W, BUFFER_H, 0, 0, DISPLAY_W, DISPLAY_H, 0);
	al_flip_display();
}

// Keyboard.
// =============================================================================
#define KEY_SEEN 1
#define KEY_RELEASED 2
unsigned char key[ALLEGRO_KEY_MAX];

void KeyboardInit()
{
	memset(key, 0, sizeof(key));
}

void KeyboardUpdate(ALLEGRO_EVENT* event)
{
	if (event->type == ALLEGRO_EVENT_TIMER)
	{
		for (size_t i = 0; i < ALLEGRO_KEY_MAX; i++)
		{
			key[i] &= KEY_SEEN;
		}
	}
	else if (event->type == ALLEGRO_EVENT_KEY_DOWN)
	{
		key[event->keyboard.keycode] = KEY_SEEN | KEY_RELEASED;
	}
	else if (event->type == ALLEGRO_EVENT_KEY_UP)
	{
		key[event->keyboard.keycode] &= KEY_RELEASED;
	}
}

// Main.
// =============================================================================
int main()
{
	// Initialize.
	// -------------------------------------------------------------------------
	MustInit(al_init(), "allegro");
	MustInit(al_install_keyboard(), "keyboard");
	MustInit(al_init_primitives_addon(), "primitives");

	ALLEGRO_TIMER* timer = al_create_timer(1.0 / 60.0);
	MustInit(timer, "timer");

	ALLEGRO_EVENT_QUEUE* queue = al_create_event_queue();
	MustInit(queue, "queue");

	DisplayInit();

	// Queue.
	// -------------------------------------------------------------------------
	al_register_event_source(queue, al_get_keyboard_event_source());
	al_register_event_source(queue, al_get_display_event_source(display));
	al_register_event_source(queue, al_get_timer_event_source(timer));

	KeyboardInit();

	// Game Loop.
	// -------------------------------------------------------------------------
	frames = 0;
	score = 0;

	bool isDone = false;
	bool needRedraw = true;
	ALLEGRO_EVENT event;

	al_start_timer(timer);
	while (true)
	{
		al_wait_for_event(queue, &event);

		if (event.type == ALLEGRO_EVENT_TIMER)
		{
			// Update logic.
			//------------------------------------------------------------------

			if (key[ALLEGRO_KEY_ESCAPE]) isDone = true;

			needRedraw = true;
			frames++;
		}
		else if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
		{
			isDone = true;
		}

		if (isDone) break;

		KeyboardUpdate(&event);

		if (needRedraw && al_event_queue_is_empty(queue))
		{
			DisplayPreDraw();
			al_clear_to_color(al_map_rgb(0, 0, 0));

			// Update draw logic.
			//------------------------------------------------------------------

			DisplayPostDraw();
			needRedraw = false;
		}
	}

	// Deinitialize.
	// -------------------------------------------------------------------------
	DisplayDeinit();
	al_destroy_timer(timer);
	al_destroy_event_queue(queue);

	return 0;
}