#include <cstdio>
#include <iostream>
#include <fstream>
#include <allegro5/allegro5.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>

// GLOBAL.
// =============================================================================
long frames;
long score;
long hiscore;

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

#define CANNON_EXPLOSION_W 16
#define CANNON_EXPLOSION_H 8
#define CANNON_EXPLOSION_FRAMES 8;

#define CANNON_SHOT_EXPLOSION_W 6
#define CANNON_SHOT_EXPLOSION_H 7

#define EXPLOSION_FRAMES 8

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
typedef enum INVADER_TYPE
{
	INVADER_TYPE_SQUID = 0,
	INVADER_TYPE_CRAB,
	INVADER_TYPE_OCTOPUS,
	INVADER_TYPE_N
} INVADER_TYPE;

const int invadersW[]{ 8, 11, 12 };
const int invadersH[]{ 8, 8, 8 };

#define INVADER_SHOT_W 3
#define INVADER_SHOT_H 7

#define INVADER_EXPLOSION_W 13
#define INVADER_EXPLOSION_H 9
#define INVADER_EXPLOSION_FRAMES 8

#define INVADER_SHOT_EXPLOSION_W 6
#define INVADER_SHOT_EXPLOSION_H 7

typedef struct SPRITES
{
	ALLEGRO_BITMAP* _sheet;

	ALLEGRO_BITMAP* cannon;
	ALLEGRO_BITMAP* cannonExplosion;
	ALLEGRO_BITMAP* life;

	ALLEGRO_BITMAP* invaderSquid[2];
	ALLEGRO_BITMAP* invaderCrab[2];
	ALLEGRO_BITMAP* invaderOctopus[2];

	ALLEGRO_BITMAP* invaderExplosion;

	ALLEGRO_BITMAP* cannonShot;
	ALLEGRO_BITMAP* invaderShot1;
	ALLEGRO_BITMAP* invaderShot2;

	ALLEGRO_BITMAP* cannonShotExplosion;
	ALLEGRO_BITMAP* invaderShotExplosion;
} Sprite;
SPRITES sprites;

ALLEGRO_BITMAP* GetSprite(int x, int y, int w, int h)
{
	ALLEGRO_BITMAP* sprite = al_create_sub_bitmap(sprites._sheet, x, y, w, h);
	MustInit(sprite, "get sprite.");
	return sprite;
}

void SpriteInit()
{
	sprites._sheet = al_load_bitmap("spritesheet.png");

	sprites.cannon = GetSprite(16, 0, CANNON_W, CANNON_H);
	sprites.cannonExplosion = GetSprite(0, 0, CANNON_EXPLOSION_W, CANNON_EXPLOSION_H);
	sprites.life = GetSprite(16, 0, CANNON_W, CANNON_H);

	sprites.cannonShot = GetSprite(46, 7, CANNON_SHOT_W, CANNON_SHOT_H);
	sprites.invaderShot1 = GetSprite(28, 16, INVADER_SHOT_W, INVADER_SHOT_H);
	sprites.invaderShot2 = GetSprite(31, 16, INVADER_SHOT_W, INVADER_SHOT_H);

	sprites.cannonShotExplosion = GetSprite(42, 0, CANNON_SHOT_EXPLOSION_W, CANNON_SHOT_EXPLOSION_H);
	sprites.invaderShotExplosion = GetSprite(16, 16, INVADER_SHOT_EXPLOSION_W, INVADER_SHOT_EXPLOSION_H);

	sprites.invaderSquid[0] = GetSprite(0, 16, invadersW[INVADER_TYPE_SQUID], invadersH[INVADER_TYPE_SQUID]);
	sprites.invaderSquid[1] = GetSprite(8, 16, invadersW[INVADER_TYPE_SQUID], invadersH[INVADER_TYPE_SQUID]);

	sprites.invaderCrab[0] = GetSprite(24, 8, invadersW[INVADER_TYPE_CRAB], invadersH[INVADER_TYPE_CRAB]);
	sprites.invaderCrab[1] = GetSprite(35, 8, invadersW[INVADER_TYPE_CRAB], invadersH[INVADER_TYPE_CRAB]);

	sprites.invaderOctopus[0] = GetSprite(0, 8, invadersW[INVADER_TYPE_OCTOPUS], invadersH[INVADER_TYPE_OCTOPUS]);
	sprites.invaderOctopus[1] = GetSprite(12, 8, invadersW[INVADER_TYPE_OCTOPUS], invadersH[INVADER_TYPE_OCTOPUS]);

	sprites.invaderExplosion = GetSprite(29, 0, INVADER_EXPLOSION_W, INVADER_EXPLOSION_H);
}

void SpriteDeinit()
{
	al_destroy_bitmap(sprites.cannon);
	al_destroy_bitmap(sprites.cannonExplosion);
	al_destroy_bitmap(sprites.life);

	al_destroy_bitmap(sprites.invaderSquid[0]);
	al_destroy_bitmap(sprites.invaderSquid[1]);

	al_destroy_bitmap(sprites.invaderCrab[0]);
	al_destroy_bitmap(sprites.invaderCrab[1]);

	al_destroy_bitmap(sprites.invaderOctopus[0]);
	al_destroy_bitmap(sprites.invaderOctopus[1]);

	al_destroy_bitmap(sprites.invaderExplosion);

	al_destroy_bitmap(sprites.cannonShot);
	al_destroy_bitmap(sprites.invaderShot1);
	al_destroy_bitmap(sprites.invaderShot2);

	al_destroy_bitmap(sprites.cannonShotExplosion);
	al_destroy_bitmap(sprites.invaderShotExplosion);

	al_destroy_bitmap(sprites._sheet);
}

// Sound.
// =============================================================================
#define SAMPLES_N
ALLEGRO_SAMPLE* sampleCannonShoot;
ALLEGRO_SAMPLE* sampleCannonExplosion;
ALLEGRO_SAMPLE* sampleInvaderExplosion;

ALLEGRO_SAMPLE* samplesInvadersMove[4];

void AudioInit()
{
	al_install_audio();
	al_init_acodec_addon();
	al_reserve_samples(256);

	sampleCannonShoot = al_load_sample("shoot.wav");
	sampleCannonExplosion = al_load_sample("explosion.wav");
	MustInit(sampleCannonShoot, "shoot.wav");

	sampleInvaderExplosion = al_load_sample("invaderkilled.wav");
	MustInit(sampleInvaderExplosion, "invaderkilled.wav");

	samplesInvadersMove[0] = al_load_sample("fastinvader1.wav");
	MustInit(samplesInvadersMove[0], "fastinvader1.wav");

	samplesInvadersMove[1] = al_load_sample("fastinvader2.wav");
	MustInit(samplesInvadersMove[1], "fastinvader2.wav");

	samplesInvadersMove[2] = al_load_sample("fastinvader3.wav");
	MustInit(samplesInvadersMove[2], "fastinvader3.wav");

	samplesInvadersMove[3] = al_load_sample("fastinvader4.wav");
	MustInit(samplesInvadersMove[3], "fastinvader4.wav");
}

void AudioDeinit()
{
	al_destroy_sample(sampleCannonShoot);
	al_destroy_sample(sampleCannonExplosion);
	al_destroy_sample(sampleInvaderExplosion);

	al_destroy_sample(samplesInvadersMove[0]);
	al_destroy_sample(samplesInvadersMove[1]);
	al_destroy_sample(samplesInvadersMove[2]);
	al_destroy_sample(samplesInvadersMove[3]);
}

// FX.
// =============================================================================
typedef enum FX_TYPE
{
	FX_TYPE_CANNON_EXPLOSION = 0,
	FX_TYPE_CANNON_SHOT_EXPLOSION,
	FX_TYPE_INVADER_EXPLOSION,
	FX_TYPE_INVADER_SHOT_EXPLOSION
} FX_TYPE;

typedef struct FX
{
	int x, y;
	int w, h;
	int currentFrame;
	int frames;
	bool isAvailable;
	ALLEGRO_BITMAP* sprite;
} FX;

#define CANNON_EXPLOSION_INDEX 0
#define INVADER_EXPLOSION_INDEX 1
#define SHOT_EXPLOSION_INDEX 2
#define FX_N 6
FX fx[FX_N];

bool displayInvaderExplosion;

void FxInit()
{
	// Cannon explosion.
	fx[CANNON_EXPLOSION_INDEX].x = 0;
	fx[CANNON_EXPLOSION_INDEX].y = 0;
	fx[CANNON_EXPLOSION_INDEX].w = CANNON_EXPLOSION_W;
	fx[CANNON_EXPLOSION_INDEX].h = CANNON_EXPLOSION_H;
	fx[CANNON_EXPLOSION_INDEX].frames = CANNON_EXPLOSION_FRAMES;
	fx[CANNON_EXPLOSION_INDEX].currentFrame = 0;
	fx[CANNON_EXPLOSION_INDEX].isAvailable = true;
	fx[CANNON_EXPLOSION_INDEX].sprite = sprites.cannonExplosion;

	// Invader explosion.
	fx[INVADER_EXPLOSION_INDEX].x = 0;
	fx[INVADER_EXPLOSION_INDEX].y = 0;
	fx[INVADER_EXPLOSION_INDEX].w = INVADER_EXPLOSION_W;
	fx[INVADER_EXPLOSION_INDEX].h = INVADER_EXPLOSION_H;
	fx[INVADER_EXPLOSION_INDEX].frames = INVADER_EXPLOSION_FRAMES;
	fx[INVADER_EXPLOSION_INDEX].currentFrame = 0;
	fx[INVADER_EXPLOSION_INDEX].isAvailable = true;
	fx[INVADER_EXPLOSION_INDEX].sprite = sprites.invaderExplosion;
	displayInvaderExplosion = false;

	// Shots explosions.
	for (int i = SHOT_EXPLOSION_INDEX; i < FX_N; i++)
	{
		fx[i].isAvailable = true;
	}
}

bool FxAdd(int x, int y, FX_TYPE type)
{
	if (type == FX_TYPE_CANNON_EXPLOSION)
	{
		al_play_sample(sampleCannonExplosion, 1.f, 0.f, 1.f, ALLEGRO_PLAYMODE_ONCE, NULL);

		if (!fx[CANNON_EXPLOSION_INDEX].isAvailable) return false;
		fx[CANNON_EXPLOSION_INDEX].x = x;
		fx[CANNON_EXPLOSION_INDEX].y = y;
		fx[CANNON_EXPLOSION_INDEX].currentFrame = 0;
		fx[CANNON_EXPLOSION_INDEX].isAvailable = false;

		return true;
	}

	if (type == FX_TYPE_INVADER_EXPLOSION)
	{
		al_play_sample(sampleInvaderExplosion, 1.f, 0.f, 1.f, ALLEGRO_PLAYMODE_ONCE, NULL);

		if (!fx[INVADER_EXPLOSION_INDEX].isAvailable) return false;
		fx[INVADER_EXPLOSION_INDEX].x = x;
		fx[INVADER_EXPLOSION_INDEX].y = y;
		fx[INVADER_EXPLOSION_INDEX].currentFrame = 0;
		fx[INVADER_EXPLOSION_INDEX].isAvailable = false;
		displayInvaderExplosion = true;
		return true;
	}

	for (int i = SHOT_EXPLOSION_INDEX; i < FX_N; i++)
	{
		if (!fx[i].isAvailable) continue;

		int w = 1, h = 1, frames = 1;
		ALLEGRO_BITMAP* sprite = NULL;
		if (type == FX_TYPE_CANNON_SHOT_EXPLOSION)
		{
			w = CANNON_SHOT_EXPLOSION_W;
			h = CANNON_SHOT_EXPLOSION_H;
			frames = EXPLOSION_FRAMES;
			sprite = sprites.cannonShotExplosion;
		}
		else if (type == FX_TYPE_INVADER_SHOT_EXPLOSION)
		{
			w = CANNON_SHOT_EXPLOSION_W;
			h = CANNON_SHOT_EXPLOSION_H;
			frames = EXPLOSION_FRAMES;
			sprite = sprites.invaderShotExplosion;
		}
		fx[i].x = x;
		fx[i].y = y;
		fx[i].w = w;
		fx[i].h = h;
		fx[i].frames = frames;
		fx[i].currentFrame = 0;
		fx[i].isAvailable = false;
		fx[i].sprite = sprite;

		return true;
	}
	return false;
}

void FxUpdate()
{
	// Update cannon explosion.
	if (!fx[CANNON_EXPLOSION_INDEX].isAvailable)
	{
		if (gameState == GAME_STATE_RUNNING) fx[CANNON_EXPLOSION_INDEX].isAvailable = true;
	}

	// Update invader explosion.
	if (!fx[INVADER_EXPLOSION_INDEX].isAvailable)
	{
		if (!displayInvaderExplosion) fx[INVADER_EXPLOSION_INDEX].isAvailable = true;
	}

	// Update shots explosions.
	for (int i = SHOT_EXPLOSION_INDEX; i < FX_N; i++)
	{
		if (fx[i].isAvailable) continue;

		fx[i].currentFrame++;
		if (fx[i].currentFrame == (fx[i].frames * 2))
		{
			fx[i].isAvailable = true;
		}
	}
}

void FxDraw()
{
	for (int i = 0; i < FX_N; i++)
	{
		if (fx[i].isAvailable) continue;

		al_draw_bitmap(fx[i].sprite, fx[i].x, fx[i].y, NULL);
	}
}

// Cannon.
// =============================================================================
typedef struct SHOT
{
	int x, y;
	int dx, dy;
	int frame;
	bool isAvailable;
	bool isFromCannon;

	ALLEGRO_BITMAP* sprites[2];
} SHOT;

#define SHOT_UPDATE_FRAMES 30

#define CANNON_SHOT_SPEED 4
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

			al_play_sample(sampleCannonShoot, 1.f, 0.f, 1.f, ALLEGRO_PLAYMODE_ONCE, NULL);

			shots[i].x = x + (CANNON_W / 2) - (CANNON_SHOT_W / 2);
			shots[i].y = y + (CANNON_H / 2);
			shots[i].isAvailable = false;
			shots[i].isFromCannon = true;
			shots[i].sprites[0] = sprites.cannonShot;
			shots[i].sprites[1] = sprites.cannonShot;

			return true;
		}

		return false;
	}

	for (int i = SHOTS_N_CANNON; i < SHOTS_N; i++)
	{
		if (!shots[i].isAvailable) continue;

		al_play_sample(sampleCannonShoot, 0.6f, 0.f, 0.7f, ALLEGRO_PLAYMODE_ONCE, NULL);

		shots[i].x = x - (INVADER_SHOT_W / 2);
		shots[i].y = y;
		shots[i].isAvailable = false;
		shots[i].isFromCannon = false;
		shots[i].sprites[0] = sprites.invaderShot1;
		shots[i].sprites[1] = sprites.invaderShot2;

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
			FxAdd(shots[i].x - (CANNON_SHOT_W / 2), 0, FX_TYPE_CANNON_SHOT_EXPLOSION);
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
			FxAdd(shots[i].x - (INVADER_SHOT_W / 2), LAND_POSITION - (INVADER_SHOT_EXPLOSION_H), FX_TYPE_INVADER_SHOT_EXPLOSION);
			shots[i].isAvailable = true;
			continue;
		}

		if (frames % SHOT_UPDATE_FRAMES == 0)
		{
			shots[i].frame++;
			if (shots[i].frame > 1) shots[i].frame = 0;
		}
	}
}

void ShotsDraw()
{
	for (int i = 0; i < SHOTS_N; i++)
	{
		if (shots[i].isAvailable) continue;

		al_draw_bitmap(shots[i].sprites[shots[i].frame], shots[i].x, shots[i].y, NULL);
	}
}

// Cannon.
// =============================================================================
#define CANNON_SPEED 2

#define CANNON_START_X (BUFFER_W / 2) - (CANNON_W / 2)
#define CANNON_START_Y 217

#define CANNON_MIN_X (OFFSET)
#define CANNON_MAX_X ((BUFFER_W - OFFSET) - CANNON_W)

int cannonWaitTimer;
int cannonWaitTime;

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

	cannonWaitTime = 60;
	cannonWaitTimer = cannonWaitTime;
}

void CannonReset()
{
	int lives = Cannon.lives;

	CannonInit();
	Cannon.lives = lives;
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

	if (key[ALLEGRO_KEY_SPACE] && !displayInvaderExplosion)
	{
		ShotsAdd(true, Cannon.x, Cannon.y);
	}

	if (ShotsCollide(true, Cannon.x, Cannon.y, CANNON_W, CANNON_H))
	{
		Cannon.lives--;
		gameState = GAME_STATE_PLAYER_RESPAWNING;

		FxAdd(Cannon.x, Cannon.y, FX_TYPE_CANNON_EXPLOSION);
	}
}

void CannonDraw()
{
	if (Cannon.lives < 0) return;
	if (gameState == GAME_STATE_PLAYER_RESPAWNING) return;

	al_draw_bitmap(sprites.cannon, Cannon.x, Cannon.y, 0);
}

// Aliens.
// =============================================================================
typedef struct INVADER
{
	int x, y;
	int points;
	int frame;
	bool isAlive;
	ALLEGRO_BITMAP* sprites[2];
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
int invadersAlive;
bool needMoveInvadersX;
bool needMoveInvadersY;
bool playMoveSample;

#define INVADER_SHOT_MAX_TIME 240
#define INVADER_SHOT_MIN_TIME 120
int invaderFleetShotTimer;

#define INVADER_DESTROYED_WAIT_TIME 15
int invaderDestroyedTimer;

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
			invaders[row][column].frame = 0;
			invaders[row][column].x = x + 2 + (column * cellW);
			invaders[row][column].y = y + (row * cellH);
			invaders[row][column].sprites[0] = sprites.invaderSquid[0];
			invaders[row][column].sprites[1] = sprites.invaderSquid[1];
		}
	}

	// Crab invader.
	for (; row < 3; row++)
	{
		for (int column = 0; column < INVADER_FLEET_COLUMNS; column++)
		{
			invaders[row][column].type = INVADER_TYPE_CRAB;
			invaders[row][column].isAlive = true;
			invaders[row][column].points = 20;
			invaders[row][column].frame = 0;
			invaders[row][column].x = x + (column * cellW);
			invaders[row][column].y = y + (row * cellH);
			invaders[row][column].sprites[0] = sprites.invaderCrab[0];
			invaders[row][column].sprites[1] = sprites.invaderCrab[1];
		}
	}

	// Octopus invader.
	for (; row < INVADER_FLEET_ROWS; row++)
	{
		for (int column = 0; column < INVADER_FLEET_COLUMNS; column++)
		{
			invaders[row][column].type = INVADER_TYPE_OCTOPUS;
			invaders[row][column].isAlive = true;
			invaders[row][column].points = 10;
			invaders[row][column].frame = 0;
			invaders[row][column].x = x + (column * cellW);
			invaders[row][column].y = y + (row * cellH);
			invaders[row][column].sprites[0] = sprites.invaderOctopus[0];
			invaders[row][column].sprites[1] = sprites.invaderOctopus[1];
		}
	}

	invaderFleetMoveTime = (INVADER_FLEET_N / 3) * 2;
	invaderFleetMoveTimer = invaderFleetMoveTime;

	invadersAlive = INVADER_FLEET_N;

	invaderFleetSpeedX = 3;
	invaderFleetSpeedY = 8;

	needMoveInvadersY = false;

	playMoveSample = false;

	invaderFleetShotTimer = Between(INVADER_SHOT_MIN_TIME, INVADER_SHOT_MAX_TIME);
}

void InvadersUpdate()
{
	// Invader destroyed.
	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	if (invaderDestroyedTimer > 0)
	{
		invaderDestroyedTimer--;
		return;
	}
	else
	{
		displayInvaderExplosion = false;
	}

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

				invaderDestroyedTimer = INVADER_DESTROYED_WAIT_TIME;
				invaderFleetMoveTimer = 0;
				FxAdd(invaders[row][column].x, invaders[row][column].y, FX_TYPE_INVADER_EXPLOSION);
				return;
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
		return;
	}

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

				invaders[row][column].frame++;
				if (invaders[row][column].frame > 1) invaders[row][column].frame = 0;
			}

			if (needMoveInvadersY)
			{
				invaderFleetMoveTimer = invaderFleetMoveTime;
				return;
			}
		}

		if (invadersAlive > 30) playMoveSample = true;
		else if (frames % 30 == 0) playMoveSample = true;
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

	// TODO: Add fleet movement audio.
	/*
	if (playMoveSample)
	{
		al_play_sample(samplesInvadersMove[Between(0, 3)], 0.75f, 0.f, 1.f, ALLEGRO_PLAYMODE_ONCE, NULL);
		playMoveSample = false;
	}
	*/

	// Reset timer.
	invaderFleetMoveTimer = invaderFleetMoveTime;
}

void InvadersDraw()
{
	for (size_t row = 0; row < INVADER_FLEET_ROWS; row++)
	{
		for (size_t column = 0; column < INVADER_FLEET_COLUMNS; column++)
		{
			if (!invaders[row][column].isAlive) continue;

			al_draw_bitmap(invaders[row][column].sprites[invaders[row][column].frame], invaders[row][column].x, invaders[row][column].y, NULL);
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
	if (frames % 60 == 0) return;
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
	al_draw_textf(font, al_map_rgb(255, 255, 255), 100, 20, ALLEGRO_ALIGN_CENTER, "%04ld", hiscore);

	al_draw_line(OFFSET, 239, BUFFER_W - OFFSET, 239, al_map_rgb(29, 255, 29), 1.f);

	// Cannon lives.
	al_draw_textf(font, al_map_rgb(255, 255, 255), OFFSET + 10, LAND_POSITION, 0, "%ld", Cannon.lives);
	for (int i = 0; i < Cannon.lives; i++)
	{
		int x = OFFSET + 20 + ((CANNON_W + 3) * i);
		int y = LAND_POSITION + 3;

		al_draw_bitmap(sprites.life, x, y, NULL);
	}

	// Game state.
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

// Score.
// =============================================================================
void ScoreLoad()
{
	std::ifstream infile("score.txt");
	infile >> hiscore;
	infile.close();
}

void ScoreSave()
{
	if (score > hiscore)
	{
		std::ofstream outfile("score.txt");
		outfile << score;
		outfile.close();
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
	MustInit(al_init_image_addon(), "image");

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
	SpriteInit();
	CannonInit();
	ShotsInit();
	InvadersInit();
	AudioInit();
	FxInit();
	HUDInit();

	frames = 0;
	gameState = GAME_STATE_RUNNING;

	ScoreLoad();

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
			else if (gameState == GAME_STATE_PLAYER_RESPAWNING)
			{
				if (Cannon.lives <= 0)
				{
					ShotsInit();
					CannonReset();

					gameState = GAME_STATE_INVADERS_WIN;
				}
				else
				{
					if (cannonWaitTimer > 0)
					{
						cannonWaitTimer--;
					}
					else
					{
						ShotsInit();
						CannonReset();

						cannonWaitTimer = cannonWaitTime;
						gameState = GAME_STATE_RUNNING;
					}
				}
			}

			FxUpdate();
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

			// Update draw logic.
			//------------------------------------------------------------------
			if (gameState == GAME_STATE_RUNNING || gameState == GAME_STATE_PLAYER_RESPAWNING)
			{
				ShotsDraw();
				InvadersDraw();
				FxDraw();
				CannonDraw();
			}
			HUDDraw();

			DisplayPostDraw();
			needRedraw = false;
		}
	}

	// Save score.
	// -------------------------------------------------------------------------
	ScoreSave();

	// Deinitialize.
	// -------------------------------------------------------------------------
	DisplayDeinit();
	HUDDeinit();
	AudioDeinit();
	SpriteDeinit();
	al_destroy_timer(timer);
	al_destroy_event_queue(queue);

	return 0;
}