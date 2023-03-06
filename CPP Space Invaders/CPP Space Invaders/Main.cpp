#include <cstdio>
#include <allegro5/allegro5.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>

// GLOBAL.
// =============================================================================
long frames;
long score;

#define OFFSET 16
#define LAND_POSITION 239

typedef enum GAME_STATE
{
	GAME_STATE_RUNNING = 0,
	GAME_STATE_PLAYER_WIN,
	GAME_STATE_PLAYER_RESPAWNING,
	GAME_STATE_INVADERS_WIN
} GAME_STATE;
GAME_STATE gameState;

// Utilities.
// =============================================================================
void MustInit(bool test, const char* description)
{
	if (test) return;

	printf("ERROR: Couldn't initialize %s\n", description);
	exit(1);
}

int Between(int lo, int hi)
{
	return lo + (rand() % (hi - lo));
}

float BetweenFloat(float lo, float hi)
{
	return lo + ((float)rand() / (float)RAND_MAX) * (hi - lo);
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

#define INVADER_SHOT_W 3
#define INVADER_SHOT_H 7

const int invadersW[]{ 8, 11, 12 };
const int invadersH[]{ 8, 8, 8 };

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
#define INVADER_SHOT_SPEED 1

// Cannon - 1 | Aliens - 3
#define SHOTS_N_CANNON 1
#define SHOTS_N_INVADERS 3
#define SHOTS_N (SHOTS_N_CANNON + SHOTS_N_INVADERS)
SHOT shots[SHOTS_N];

void ShotsInit()
{
	for (int i = 0; i < SHOTS_N; i++)
	{
		shots[i].isAvailable = true;
	}
}

bool ShotsAdd(bool isFromCannon, int x, int y)
{
	if (isFromCannon)
	{
		for (int i = 0; i < SHOTS_N_CANNON; i++)
		{
			if (!shots[i].isAvailable) continue;

			shots[i].x = x + (CANNON_W / 2) - (CANNON_SHOT_W / 2);
			shots[i].y = y + (CANNON_H / 2);
			shots[i].isAvailable = false;
			shots[i].isFromCannon = true;

			return true;
		}

		return false;
	}

	for (int i = SHOTS_N_CANNON; i < SHOTS_N; i++)
	{
		if (!shots[i].isAvailable) continue;

		shots[i].x = x - (INVADER_SHOT_W / 2);
		shots[i].y = y;
		shots[i].isAvailable = false;
		shots[i].isFromCannon = false;

		return true;
	}

	return false;
}

bool ShotsCollide(bool isFromCannon, int x, int y, int  w, int h)
{
	for (int i = 0; i < SHOTS_N; i++)
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
			sw = INVADER_SHOT_W;
			sh = INVADER_SHOT_H;
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
	// Cannon shots.
	for (int i = 0; i < SHOTS_N_CANNON; i++)
	{
		if (shots[i].isAvailable) continue;

		shots[i].y -= CANNON_SHOT_SPEED;

		if (shots[i].y < -CANNON_SHOT_H)
		{
			shots[i].isAvailable = true;
			continue;
		}

		if (ShotsCollide(true, shots[i].x, shots[i].y, INVADER_SHOT_W, INVADER_SHOT_H))
		{
			if (Between(0, 100) <= 80) shots[i].isAvailable = true;
		}
	}

	// Invaders shots.
	for (int i = SHOTS_N_CANNON; i < SHOTS_N; i++)
	{
		if (shots[i].isAvailable) continue;

		shots[i].y += INVADER_SHOT_SPEED;

		if (shots[i].y + INVADER_SHOT_H > LAND_POSITION)
		{
			shots[i].isAvailable = true;
			continue;
		}
	}
}

void ShotsDraw()
{
	//	Cannon shots
	for (int i = 0; i < SHOTS_N_CANNON; i++)
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
	for (int i = SHOTS_N_CANNON; i < SHOTS_N; i++)
	{
		if (shots[i].isAvailable) continue;

		al_draw_filled_rectangle(
			shots[i].x,
			shots[i].y,
			shots[i].x + INVADER_SHOT_W,
			shots[i].y + INVADER_SHOT_H,
			al_map_rgb(255, 255, 0));
	}
}

// Cannon.
// =============================================================================
#define CANNON_SPEED 2

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

	Cannon.lives = 3;
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
		Cannon.lives--;
		if (Cannon.lives <= 0)
		{
			gameState = GAME_STATE_INVADERS_WIN;
		}
	}
}

void CannonDraw()
{
	if (Cannon.lives < 0) return;

	al_draw_filled_rectangle(
		Cannon.x,
		Cannon.y,
		Cannon.x + CANNON_W,
		Cannon.y + CANNON_H,
		al_map_rgb(29, 255, 29));
}

// Aliens.
// =============================================================================
typedef enum INVADER_TYPE
{
	INVADER_TYPE_SQUID = 0,
	INVADER_TYPE_CRAB,
	INVADER_TYPE_OCTOPUS,
	INVADER_TYPE_N
} INVADER_TYPE;

typedef struct INVADER
{
	int x, y;
	int points;
	bool isAlive;
	INVADER_TYPE type;
} ALIEN;

#define INVADER_FLEET_ROWS 5
#define INVADER_FLEET_COLUMNS 11
#define INVADER_FLEET_N (INVADER_FLEET_ROWS * INVADER_FLEET_COLUMNS)

#define INVADER_FLEET_CELL_W 12
#define INVADER_FLEET_CELL_H 8
#define INVADER_FLEET_CELL_SPACE 4

INVADER invaders[INVADER_FLEET_ROWS][INVADER_FLEET_COLUMNS];
int invaderFleetMoveTime;
int invaderFleetMoveTimer;
int invaderFleetSpeedX;
int invaderFleetSpeedY;
bool needMoveInvadersY;
int invadersAlive;

#define INVADER_SHOT_MAX_TIME 240
#define INVADER_SHOT_MIN_TIME 120
int invaderFleetShotTimer;

void InvadersInit()
{
	int x = OFFSET + 32;
	int y = 46;

	int cellW = INVADER_FLEET_CELL_W + INVADER_FLEET_CELL_SPACE;
	int cellH = INVADER_FLEET_CELL_H + INVADER_FLEET_CELL_SPACE;

	// Squid invader.
	int row = 0;
	for (; row < 1; row++)
	{
		for (int column = 0; column < INVADER_FLEET_COLUMNS; column++)
		{
			invaders[row][column].type = INVADER_TYPE_SQUID;
			invaders[row][column].isAlive = true;
			invaders[row][column].points = 30;
			invaders[row][column].x = x + 2 + (column * cellW);
			invaders[row][column].y = y + (row * cellH);
		}
	}

	// Crab invader.
	for (; row < 3; row++)
	{
		for (size_t column = 0; column < INVADER_FLEET_COLUMNS; column++)
		{
			invaders[row][column].type = INVADER_TYPE_CRAB;
			invaders[row][column].isAlive = true;
			invaders[row][column].points = 20;
			invaders[row][column].x = x + (column * cellW);
			invaders[row][column].y = y + (row * cellH);
		}
	}

	// Crab invader.
	for (; row < INVADER_FLEET_ROWS; row++)
	{
		for (int column = 0; column < INVADER_FLEET_COLUMNS; column++)
		{
			invaders[row][column].type = INVADER_TYPE_OCTOPUS;
			invaders[row][column].isAlive = true;
			invaders[row][column].points = 10;
			invaders[row][column].x = x + (column * cellW);
			invaders[row][column].y = y + (row * cellH);
		}
	}

	invaderFleetMoveTime = (INVADER_FLEET_N / 3) * 2;
	invaderFleetMoveTimer = invaderFleetMoveTime;

	invadersAlive = INVADER_FLEET_N;

	invaderFleetSpeedX = 3;
	invaderFleetSpeedY = 8;

	needMoveInvadersY = false;

	invaderFleetShotTimer = Between(INVADER_SHOT_MIN_TIME, INVADER_SHOT_MAX_TIME);
}

void InvadersUpdate()
{
	// Collision.
	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	for (int row = 0; row < INVADER_FLEET_ROWS; row++)
	{
		for (int column = 0; column < INVADER_FLEET_COLUMNS; column++)
		{
			if (!invaders[row][column].isAlive) continue;

			// Shots collision.
			if (ShotsCollide(
				false,
				invaders[row][column].x,
				invaders[row][column].y,
				invadersW[invaders[row][column].type],
				invadersH[invaders[row][column].type]))
			{
				invaders[row][column].isAlive = false;
				invadersAlive--;
				if (invadersAlive == 0)
				{
					gameState = GAME_STATE_PLAYER_WIN;
					return;
				}

				score += invaders[row][column].points;

				invaderFleetMoveTime -= 1;
				if (invaderFleetMoveTime <= 0) invaderFleetMoveTime = 1;
			}

			// Cannon collision.
			if (Collide(
				invaders[row][column].x,
				invaders[row][column].y,
				invaders[row][column].x + invadersW[invaders[row][column].type],
				invaders[row][column].y + invadersH[invaders[row][column].type],
				Cannon.x,
				Cannon.y,
				Cannon.x + CANNON_W,
				Cannon.y + CANNON_H))
			{
				gameState = GAME_STATE_INVADERS_WIN;
				return;
			}
		}
	}

	// Timer (Shoting).
	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	if (invaderFleetShotTimer > 0)
	{
		invaderFleetShotTimer--;
	}
	else
	{
		bool invaderShot = false;
		for (int row = 0; row < INVADER_FLEET_ROWS; row++)
		{
			for (int column = 0; column < INVADER_FLEET_COLUMNS; column++)
			{
				if (!invaders[row][column].isAlive) continue;

				if (Between(0, 100) >= 90)
				{
					invaderShot = ShotsAdd(
						false,
						invaders[row][column].x + (invadersW[invaders[row][column].type] / 2),
						invaders[row][column].y);

					if (invaderShot) break;
				}
			}

			if (invaderShot) break;
		}

		invaderFleetShotTimer = Between(INVADER_SHOT_MIN_TIME, INVADER_SHOT_MAX_TIME);
	}

	// Timer (Movement).
	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	if (invaderFleetMoveTimer > 0)
	{
		invaderFleetMoveTimer--;
	}
	else
	{
		// Move in X.
		if (!needMoveInvadersY)
		{
			for (int row = 0; row < INVADER_FLEET_ROWS; row++)
			{
				for (int column = 0; column < INVADER_FLEET_COLUMNS; column++)
				{
					invaders[row][column].x += invaderFleetSpeedX;
				}
			}

			// Check positions in X.
			for (int row = 0; row < INVADER_FLEET_ROWS; row++)
			{
				for (int column = 0; column < INVADER_FLEET_COLUMNS; column++)
				{
					if (!invaders[row][column].isAlive) continue;

					if (invaders[row][column].x <= OFFSET)
					{
						invaderFleetSpeedX *= -1;
						needMoveInvadersY = true;
						break;
					}
					else if ((invaders[row][column].x + invadersW[invaders[row][column].type]) >= (BUFFER_W - OFFSET))
					{
						invaderFleetSpeedX *= -1;
						needMoveInvadersY = true;
						break;
					}
				}

				if (needMoveInvadersY)
				{
					invaderFleetMoveTimer = invaderFleetMoveTime;
					return;
				}
			}
		}

		// Move in Y.
		if (needMoveInvadersY)
		{
			for (int row = 0; row < INVADER_FLEET_ROWS; row++)
			{
				for (int column = 0; column < INVADER_FLEET_COLUMNS; column++)
				{
					invaders[row][column].y += invaderFleetSpeedY;
				}
			}

			// Check land position.
			for (int row = 0; row < INVADER_FLEET_ROWS; row++)
			{
				for (int column = 0; column < INVADER_FLEET_COLUMNS; column++)
				{
					if (!invaders[row][column].isAlive) continue;

					int invaderPositon = invaders[row][column].y + invadersH[invaders[row][column].type] + 3;
					if (invaderPositon >= LAND_POSITION)
					{
						gameState = GAME_STATE_INVADERS_WIN;
						return;
					}
				}
			}

			needMoveInvadersY = false;
		}

		// Reset timer.
		invaderFleetMoveTimer = invaderFleetMoveTime;
	}
}

void InvadersDraw()
{
	for (size_t row = 0; row < INVADER_FLEET_ROWS; row++)
	{
		for (size_t column = 0; column < INVADER_FLEET_COLUMNS; column++)
		{
			//if (!invaders[row][column].isAlive) continue;
			// TODO: [InvadersDraw]: Don't draw death invaders.

			if (invaders[row][column].isAlive)
			{
				al_draw_filled_rectangle(
					invaders[row][column].x,
					invaders[row][column].y,
					invaders[row][column].x + invadersW[invaders[row][column].type],
					invaders[row][column].y + invadersH[invaders[row][column].type],
					al_map_rgb(200, 0, 0));
			}
			else
			{
				al_draw_filled_rectangle(
					invaders[row][column].x,
					invaders[row][column].y,
					invaders[row][column].x + invadersW[invaders[row][column].type],
					invaders[row][column].y + invadersH[invaders[row][column].type],
					al_map_rgb(150, 0, 0));
			}
		}
	}
}

// Aliens.
// =============================================================================
ALLEGRO_FONT* font;
long scoreDisplay;

void HUDInit()
{
	font = al_load_font("monogram.ttf", 16, 0);

	//font = al_create_builtin_font();
	MustInit(font, "font");

	scoreDisplay = 0;
}

void HUDDeinit()
{
	al_destroy_font(font);
}

void HUDUpdate()
{
	if (frames % 2) return;
	for (long i = 5; i > 0; i--)
	{
		long diff = 1 << i;
		if (scoreDisplay <= (score - diff))
		{
			scoreDisplay += diff;
		}
	}
}

void HUDDraw()
{
	al_draw_text(font, al_map_rgb(255, 255, 255), 46, 8, ALLEGRO_ALIGN_CENTER, "SCORE");
	al_draw_textf(font, al_map_rgb(255, 255, 255), 46, 20, ALLEGRO_ALIGN_CENTER, "%04ld", scoreDisplay);

	al_draw_text(font, al_map_rgb(255, 255, 255), 100, 8, ALLEGRO_ALIGN_CENTER, "HI-SCORE");
	al_draw_textf(font, al_map_rgb(255, 255, 255), 100, 20, ALLEGRO_ALIGN_CENTER, "%04ld", 0);

	al_draw_line(OFFSET, 239, BUFFER_W - OFFSET, 239, al_map_rgb(29, 255, 29), 1.f);

	al_draw_textf(font, al_map_rgb(255, 255, 255), OFFSET + 10, LAND_POSITION, 0, "%ld", Cannon.lives);

	if (gameState == GAME_STATE_PLAYER_WIN)
	{
		al_draw_text(font, al_map_rgb_f(1, 1, 1), BUFFER_W / 2, BUFFER_H / 2, ALLEGRO_ALIGN_CENTER, "G A M E  O V E R");
		al_draw_text(font, al_map_rgb_f(1, 1, 1), BUFFER_W / 2, BUFFER_H / 2 + 20, ALLEGRO_ALIGN_CENTER, "W I N");
	}
	if (gameState == GAME_STATE_INVADERS_WIN)
	{
		al_draw_text(font, al_map_rgb_f(1, 1, 1), BUFFER_W / 2, BUFFER_H / 2, ALLEGRO_ALIGN_CENTER, "G A M E  O V E R");
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
	MustInit(al_init_font_addon(), "fonts");
	MustInit(al_init_ttf_addon(), "ttf");

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
	HUDInit();

	frames = 0;
	score = 0;
	gameState = GAME_STATE_RUNNING;

	srand(time(NULL));

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
			if (gameState == GAME_STATE_RUNNING)
			{
				CannonUpdate();
				InvadersUpdate();
				ShotsUpdate();
			}
			HUDUpdate();

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
			CannonDraw();
			InvadersDraw();
			ShotsDraw();
			HUDDraw();

			DisplayPostDraw();
			needRedraw = false;
		}
	}

	// Deinitialize.
	// -------------------------------------------------------------------------
	DisplayDeinit();
	HUDDeinit();
	al_destroy_timer(timer);
	al_destroy_event_queue(queue);

	return 0;
}