#include <cstdio>
#include <allegro5/allegro5.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>

// GLOBAL.
// =============================================================================
long frames;
long score;

#define OFFSET 16

// Utilities.
// =============================================================================
void MustInit(bool test, const char* description)
{
	if (test) return;

	printf("ERROR: Couldn't initialize %s\n", description);
	exit(1);
}

bool Collide(int ax1, int ay1, int ax2, int ay2, int bx1, int by1, int bx2, int by2)
{
	if (ax1 > bx2) return false;
	if (ax2 < bx1) return false;
	if (ay1 > by2) return false;
	if (ay2 < by1) return false;

	return true;
}

// Display.
// =============================================================================
#define BUFFER_W 256
#define BUFFER_H 256

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

// Sprites.
// =============================================================================
#define CANNON_W 13
#define CANNON_H 8

#define CANNON_SHOT_W 2
#define CANNON_SHOT_H 6

#define ALIEN_SHOT_W 3
#define ALIEN_SHOT_H 7

// Cannon.
// =============================================================================
typedef struct SHOT
{
	int x, y;
	int dx, dy;
	int frame;
	bool isAvailable;
	bool isFromCannon;
} SHOT;

#define CANNON_SHOT_SPEED 5

// Cannon - 1 | Aliens - 3
#define SHOTS_N_CANNON 1
#define SHOTS_N_ALIENS 3
#define SHOTS_N (SHOTS_N_CANNON + SHOTS_N_ALIENS)
SHOT shots[SHOTS_N];

void ShotsInit()
{
	for (size_t i = 0; i < SHOTS_N; i++)
	{
		shots[i].isAvailable = true;
	}
}

bool ShotsAdd(bool isFromCannon, int x, int y)
{
	if (isFromCannon)
	{
		for (size_t i = 0; i < SHOTS_N_CANNON; i++)
		{
			if (!shots[i].isAvailable) continue;

			shots[i].x = x + (CANNON_W / 2) - (CANNON_SHOT_W / 2);
			shots[i].y = y + (CANNON_H / 2);
			shots[i].isAvailable = false;
			shots[i].isFromCannon = true;
		}

		return false;
	}

	return false;
}

bool ShotsCollide(bool isFromCannon, int x, int y, int  w, int h)
{
	for (size_t i = 0; i < SHOTS_N_CANNON; i++)
	{
		if (shots[i].isAvailable) continue;
		if (shots[i].isFromCannon == isFromCannon) continue;

		int sw = 0, sh = 0;
		if (isFromCannon)
		{
			sw = CANNON_SHOT_W;
			sh = CANNON_SHOT_H;
		}
		else
		{
			sw = ALIEN_SHOT_W;
			sh = ALIEN_SHOT_H;
		}

		if (Collide(x, y, x + w, y + h, shots[i].x, shots[i].y, shots[i].x + sw, shots[i].y + sh))
		{
			shots[i].isAvailable = true;
			return true;
		}
	}
	return false;
}

void ShotsUpdate()
{
	for (size_t i = 0; i < SHOTS_N_CANNON; i++)
	{
		if (shots[i].isAvailable) continue;

		shots[i].y -= CANNON_SHOT_SPEED;

		if (shots[i].y < -CANNON_SHOT_H)
		{
			shots[i].isAvailable = true;
			continue;
		}
	}
}

void ShotsDraw()
{
	//	Cannon shots
	for (size_t i = 0; i < SHOTS_N_CANNON; i++)
	{
		if (shots[i].isAvailable) continue;

		al_draw_filled_rectangle(
			shots[i].x,
			shots[i].y,
			shots[i].x + CANNON_SHOT_W,
			shots[i].y + CANNON_SHOT_H,
			al_map_rgb(255, 255, 255));
	}

	// Alien shots
	for (size_t i = SHOTS_N_CANNON; i < SHOTS_N; i++)
	{
		if (shots[i].isAvailable) continue;

		al_draw_filled_rectangle(
			shots[i].x,
			shots[i].y,
			shots[i].x + ALIEN_SHOT_W,
			shots[i].y + ALIEN_SHOT_H,
			al_map_rgb(255, 0, 0));
	}
}

// Cannon.
// =============================================================================
#define CANNON_SPEED 3

#define CANNON_START_X (BUFFER_W / 2) - (CANNON_W / 2)
#define CANNON_START_Y 217

#define CANNON_MIN_X (OFFSET)
#define CANNON_MAX_X ((BUFFER_W - OFFSET) - CANNON_W)

typedef struct SHIP
{
	int x, y;
	int lives;
} SHIP;

SHIP Cannon;

void CannonInit()
{
	Cannon.x = CANNON_START_X;
	Cannon.y = CANNON_START_Y;
}

void CannonUpdate()
{
	if (key[ALLEGRO_KEY_RIGHT])
	{
		Cannon.x += CANNON_SPEED;
	}
	if (key[ALLEGRO_KEY_LEFT])
	{
		Cannon.x -= CANNON_SPEED;
	}

	if (Cannon.x <= CANNON_MIN_X) Cannon.x = CANNON_MIN_X;
	if (Cannon.x >= CANNON_MAX_X) Cannon.x = CANNON_MAX_X;

	if (key[ALLEGRO_KEY_SPACE])
	{
		ShotsAdd(true, Cannon.x, Cannon.y);
	}

	if (ShotsCollide(true, Cannon.x, Cannon.y, CANNON_W, CANNON_H))
	{
	}
}

void CannonDraw()
{
	al_draw_filled_rectangle(
		Cannon.x,
		Cannon.y,
		Cannon.x + CANNON_W,
		Cannon.y + CANNON_H,
		al_map_rgb(29, 255, 29));
}

// Aliens.
// =============================================================================

#define INVADER_SQUID_W 8
#define INVADER_SQUID_H 8

#define INVADER_FLEET_ROWS 5
#define INVADER_FLEET_COLUMNS 11
#define INVADER_FLEET_N (INVADER_FLEET_ROWS * INVADER_FLEET_COLUMNS)

typedef enum INVADER_TYPE
{
	INVADER_TYPE_SQUID = 0,
	INVADER_TYPE_N
} INVADER_TYPE;

typedef struct INVADER
{
	int x, y;
	int life;
	int points;
	INVADER_TYPE type;
} ALIEN;
INVADER invaders[INVADER_FLEET_N];

void InvadersInit()
{
	int space = 3;
	int startX = OFFSET + 10;
	int startY = 50;

	for (size_t y = 0; y < INVADER_FLEET_ROWS; y++)
	{
		for (size_t x = 0; x < INVADER_FLEET_COLUMNS; x++)
		{
			int index = y * INVADER_FLEET_COLUMNS + x;
			invaders[index].x = startX + (x * (INVADER_SQUID_W + space));
			invaders[index].y = startY + (y * (INVADER_SQUID_H + space));
			invaders[index].life = 1;
			invaders[index].type = INVADER_TYPE_SQUID;
		}
	}
}

void InvadersUpdate()
{
	for (size_t y = 0; y < INVADER_FLEET_ROWS; y++)
	{
		for (size_t x = 0; x < INVADER_FLEET_COLUMNS; x++)
		{
			int index = y * INVADER_FLEET_COLUMNS + x;

			if (invaders[index].life <= 0) continue;

			if (ShotsCollide(false, invaders[index].x, invaders[index].y, INVADER_SQUID_W, INVADER_SQUID_H))
			{
				invaders[index].life--;
			}
		}
	}
}

void InvadersDraw()
{
	for (size_t y = 0; y < INVADER_FLEET_ROWS; y++)
	{
		for (size_t x = 0; x < INVADER_FLEET_COLUMNS; x++)
		{
			int index = y * INVADER_FLEET_COLUMNS + x;

			if (invaders[index].life <= 0) continue;

			al_draw_filled_rectangle(
				invaders[index].x,
				invaders[index].y,
				invaders[index].x + INVADER_SQUID_W,
				invaders[index].y + INVADER_SQUID_H,
				al_map_rgb(255, 0, 0));
		}
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
	CannonInit();
	ShotsInit();
	InvadersInit();

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
			CannonUpdate();
			ShotsUpdate();
			InvadersUpdate();

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

			al_draw_filled_rectangle(OFFSET, 0, BUFFER_W - OFFSET, BUFFER_H, al_map_rgb(30, 30, 30));

			// Update draw logic.
			//------------------------------------------------------------------
			ShotsDraw();
			InvadersDraw();
			CannonDraw();

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