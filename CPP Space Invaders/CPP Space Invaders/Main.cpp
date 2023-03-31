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

#include "ThirdParty/effolkronium/random.hpp"

#include "Utilities.h"

using Random = effolkronium::random_static;

// GAME =============================================================================
enum class GameState
{
	MainMenu = 0,
	Gameplay,
	Respawning,
	GameOver
};
GameState currentState = GameState::MainMenu;
GameState newState = GameState::MainMenu;

long frames;
int score;
int hiscore;

const int HorizontalPadding = 16;
const int GroundPosition = 239;

bool showText;

bool initState = true;

void SwitchToState(GameState state, bool callInit = true);
void SwitchState();

void ScoreSave();
void ScoreLoad();

// DISPLAY ==========================================================================
const int BufferW = 256;
const int BufferH = 256;

const int DisplayScale = 3;
const int DisplayW = BufferW * DisplayScale;
const int DisplayH = BufferH * DisplayScale;

#define COLOR_BACKGROUND al_map_rgb(33, 33, 35)
ALLEGRO_DISPLAY* displayPtr;
ALLEGRO_BITMAP* bufferPtr;

void DisplayInit();
void DisplayDeinit();
void DisplayPreDraw();
void DisplayPostDraw();

// KEYBOARD =========================================================================
const int KeySeen = 1; // 0001
const int KeyReleased = 2; // 0010

unsigned char key[ALLEGRO_KEY_MAX];

void KeyboardInit();
void KeyboardUpdate(ALLEGRO_EVENT* event);

// SPRITES ==========================================================================
struct Sprites
{
	ALLEGRO_BITMAP* _spriteSheet;

	// Logo.
	ALLEGRO_BITMAP* _logo;

	// Cannon.
	ALLEGRO_BITMAP* cannon;
	ALLEGRO_BITMAP* cannonExplosion;
	ALLEGRO_BITMAP* cannonLaser;

	// Invaders.
	ALLEGRO_BITMAP* invaderSquid[2];
	ALLEGRO_BITMAP* invaderCrab[2];
	ALLEGRO_BITMAP* invaderOctopus[2];
	ALLEGRO_BITMAP* invaderLaser;
	ALLEGRO_BITMAP* invaderLaserWiggly;
	ALLEGRO_BITMAP* invaderExplosion;

	// Lasers.
	ALLEGRO_BITMAP* laserExplosionUp;
	ALLEGRO_BITMAP* laserExplosionDown;

	// Shields.
	ALLEGRO_BITMAP* shieldComplete[11];
	ALLEGRO_BITMAP* shieldDestroyed[11];
};
Sprites sprites;

const int CannonW = 13;
const int CannonH = 8;
const int CannonExplosionW = 16;
const int CannonExplosionH = 8;
const int CannonExplosionFrames = 32;

const int InvaderW[]{ 8, 11, 12 }; // SQUID - CRAB - OCTOPUS.
const int InvaderH[]{ 8, 8, 8 }; // SQUID - CRAB - OCTOPUS.
const int InvaderExplosionW = 13;
const int InvaderExplosionH = 8;

const int LaserW[]{ 2, 3, 3 }; // CANNON - INVADER - INVADER_WIGGLY.
const int LaserH[]{ 6, 7, 7 }; // CANNON - INVADER - INVADER_WIGGLY.
const int LaserExplosionW = 6;
const int LaserExplosionH = 7;
const int LaserExplosionFrames = 8;

const int ShieldW = 7;
const int ShieldH = 4;

void SpritesInit();
void SpritesDeinit();
ALLEGRO_BITMAP* SpriteGet(int x, int y, int w, int h);

// AUDIO ============================================================================
ALLEGRO_SAMPLE* sampleCannonExplosion;
ALLEGRO_SAMPLE* sampleInvaderExplosion;
ALLEGRO_SAMPLE* sampleLaser;

void AudioInit();
void AudioDeinit();

// FONTS ============================================================================
ALLEGRO_FONT* mainFont = { 0 };

void FontsInit();
void FontsDeinit();

// FX ===============================================================================
enum FxType
{
	FX_TYPE_CANNON_EXPLOSION = 0,
	FX_TYPE_CANNON_LASER_EXPLOSION,
	FX_TYPE_INVADER_EXPLOSION,
	FX_TYPE_INVADER_LASER_EXPLOSION
};

struct FX
{
	int x, y;
	int w, h;
	int timer;
	bool isAvalilable;
	ALLEGRO_BITMAP* sprite;
};

const int FxN = 30;
const int FxCannonExplosionIndex = 0;
const int FxInvaderExplosionIndex = 1;
const int FxLaserExplosionIndex = 2;

FX fx[FxN];

void FxInit();
void FxUpdate();
void FxDraw();
void FxAdd(int x, int y, FxType type);

// LASER ============================================================================
enum LaserType
{
	LASER_TYPE_CANNON = 0,
	LASER_TYPE_INVADER,
	LASER_TYPE_INVADER_WIGGLY
};

struct Laser
{
	int x, y;
	int dx, dy;
	bool isAvailable;
	LaserType type;
	ALLEGRO_BITMAP* sprite;
};

constexpr int LaserCannonSpeed = 4;
constexpr int LaserinvaderSpeed = 1;

constexpr int LaserCannonN = 1;
constexpr int LaserInvaderN = 3;
constexpr int LaserN = LaserCannonN + LaserInvaderN;

const int LaserFlipFrames = 15;
bool LaserFlipBitmap;

Laser lasers[LaserN];

void LaserInit();
void LaserUpdate();
void LaserDraw();
void LaserClear();
bool LaserAdd(int x, int y, LaserType type);
bool LaserCollide(bool checkCannonLaser, int x, int y, int w, int h);

// CANNON ===========================================================================
struct Cannon
{
	int x, y;
	int lives;
	ALLEGRO_BITMAP* sprite;
	ALLEGRO_BITMAP* spriteExplosion;
};
Cannon cannon = { 0 };

constexpr int CannonSpeed = 2;

constexpr int CannonMinXPosition = HorizontalPadding;
constexpr int CannonMaxXPosition = BufferW - HorizontalPadding - CannonW;

bool canShoot;

void CannonInit();
void CannonUpdate();
void CannonDraw();
void CannonReset();

// INVADER FLEET ====================================================================
enum InvaderType
{
	INVADER_TYPE_SQUID = 0,
	INVADER_TYPE_CRAB,
	INVADER_TYPE_OCTOPUS
};

struct Invader
{
	int x, y;
	bool isAlive;
	InvaderType type;
	ALLEGRO_BITMAP* sprite[2];
};

const int InvaderFleetRows = 5;
const int InvaderFleetColumns = 11;
const int InvaderFleetN = InvaderFleetRows * InvaderFleetColumns;

Invader invaderFleet[InvaderFleetRows][InvaderFleetColumns];

const int InvaderPoints[]{ 30, 20, 10 };

int invaderSpriteIndex;
int invadersAlive;

int invaderFleetMoveTime;
int invaderFleetMoveTimer;
int invaderFleetMoveSpeedY;
int invaderFleetMoveSpeedX;
bool invaderFleetNeedMoveY;

int invaderFleetShotTime;
int invaderFleetShotTimer;
const int InvaderFleetShotMaxTime = 120;
const int InvaderFleetShotMinTime = 30;

bool IsInvaderDestroyed;
const int InvaderFleetDestroyedTime = 15;

void InvaderFleetInit();
void InvaderFleetUpdate();
void InvaderFleetDraw();

// HUD ==============================================================================
#define COLOR_HUD_TEXT al_map_rgb(255, 255, 255)

void HUDInit();
void HUDUpdate();
void HUDDraw();

// SHIELDS ==========================================================================
enum ShieldIntegirty
{
	SHIELD_INTEGRITY_COMPLETE = 0,
	SHIELD_INTEGRITY_COMPROMISED,
	SHIELD_INTEGRITY_DESTROYED
};

struct Shield
{
	int x, y;
	int integrity;
	ALLEGRO_BITMAP* sprite[2];
};

const int ShieldN = 4;
const int ShieldPieces = 11;
const int ShieldPiecesSize = ShieldN * ShieldPieces;

Shield shields[ShieldPiecesSize];

void ShieldsInit();
void ShieldsUpdate();
void ShieldsDraw();
void SetupShield(int x, int y, int shield);

// MAIN MENU ========================================================================
#define COLOR_BACKGROUND_MAIN_MENU al_map_rgb(33, 33, 35)
#define COLOR_TEXT_MAIN_MENU al_map_rgb(255, 255, 255)
const int LogoXPosition = DisplayW * 0.5f - 300;
const int LogoYPosition = 95;

const int mainMenuTextXPosition = BufferW * 0.5f;
const int mainMenuTextYPosition = BufferH * 0.5f + 10;

void MainMenuInit();
void MainMenuUpdate();
void MainMenuDraw();

// GAMEPLAY =========================================================================
#define COLOR_GROUND al_map_rgb(138, 178, 96)
#define COLOR_BACKGROUND_GAMEPLAY al_map_rgb(33, 33, 35)

void GameplayInit();
void GameplayUpdate();
void GameplayDraw();

// RESPAWNING =======================================================================
int respawnTimer;

void RespawningInit();
void RespawningUpdate();
void RespawningDraw();

// GAMEOVER =========================================================================
#define COLOR_BACKGROUND_GAMEEOVER al_map_rgb(33, 33, 35)
#define COLOR_TEXT_GAMEOVER al_map_rgb(255, 255, 255)

const int gameoverTextX = BufferW * 0.5f;
const int gameoverTextY = BufferH * 0.5f;
const int gameoverRestartTextY = BufferH * 0.5f + 15;

void GameOverInit();
void GameOverUpdate();
void GameOverDraw();

// MAIN =============================================================================
int main()
{
	// Initialization ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Utilities::MustInit(al_init(), "Allegro5");
	Utilities::MustInit(al_install_keyboard(), "Keyboard");
	Utilities::MustInit(al_init_primitives_addon(), "Primitives Addon");
	Utilities::MustInit(al_init_font_addon(), "Font Addon");
	Utilities::MustInit(al_init_ttf_addon(), "TTF Addon");
	Utilities::MustInit(al_init_image_addon(), "Image Addon");

	DisplayInit();
	KeyboardInit();
	SpritesInit();
	FontsInit();
	AudioInit();

	// Game Loop ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	ALLEGRO_TIMER* timerPtr = al_create_timer(1.0 / 60.0);
	Utilities::MustInit(timerPtr, "Timer");
	al_start_timer(timerPtr);

	ALLEGRO_EVENT_QUEUE* queuePtr = al_create_event_queue();
	Utilities::MustInit(queuePtr, "Queue");

	al_register_event_source(queuePtr, al_get_keyboard_event_source());
	al_register_event_source(queuePtr, al_get_display_event_source(displayPtr));
	al_register_event_source(queuePtr, al_get_timer_event_source(timerPtr));

	frames = 0;
	bool isDone = false;
	bool needRedraw = false;
	ALLEGRO_EVENT event;

	// Random seed.
	srand(time(NULL));

	SwitchToState(GameState::MainMenu);
	SwitchState();

	while (!isDone)
	{
		al_wait_for_event(queuePtr, &event);
		KeyboardUpdate(&event);

		if (event.type == ALLEGRO_EVENT_TIMER)
		{
			// UPDATE LOGIC ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
			switch (currentState)
			{
			case GameState::MainMenu:
				MainMenuUpdate();
				break;
			case GameState::Gameplay:
				GameplayUpdate();
				break;
			case GameState::Respawning:
				RespawningUpdate();
				break;
			case GameState::GameOver:
				GameOverUpdate();
				break;
			default:
				break;
			}

			if (key[ALLEGRO_KEY_ESCAPE]) isDone = true;

			needRedraw = true;
			frames++;
		}
		else if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
		{
			isDone = true;
		}

		if (isDone) break;

		if (needRedraw && al_event_queue_is_empty(queuePtr))
		{
			DisplayPreDraw();
			al_clear_to_color(al_map_rgb(0, 0, 0));

			// DRAW LOGIC ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
			switch (currentState)
			{
			case GameState::MainMenu:
				MainMenuDraw();
				break;
			case GameState::Gameplay:
				GameplayDraw();
				break;
			case GameState::Respawning:
				RespawningDraw();
				break;
			case GameState::GameOver:
				GameOverDraw();
				break;
			default:
				break;
			}

			DisplayPostDraw();

			//  Title HD.
			if (currentState == GameState::MainMenu)
			{
				al_draw_bitmap(sprites._logo, LogoXPosition, LogoYPosition, NULL);
			}

			al_flip_display();
			needRedraw = false;
		}

		// SWITCH STATE ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		if (currentState != newState) SwitchState();
	}

	// Deinitialization  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	AudioDeinit();
	FontsDeinit();
	SpritesDeinit();
	DisplayDeinit();
	al_destroy_timer(timerPtr);
	al_destroy_event_queue(queuePtr);

	return 0;
}
// END MAIN =========================================================================

// GAME =============================================================================
void SwitchToState(GameState state, bool callInit)
{
	newState = state;
	initState = callInit;
}

void SwitchState()
{
	if (initState)
	{
		switch (newState)
		{
		case GameState::MainMenu:
			MainMenuInit();
			break;
		case GameState::Gameplay:
			GameplayInit();
			break;
		case GameState::Respawning:
			RespawningInit();
			break;
		case GameState::GameOver:
			GameOverInit();
			break;
		default:
			break;
		}
	}

	currentState = (GameState)newState;
}

void ScoreSave()
{
	if (score > hiscore)
	{
		std::ofstream outFile("Score.txt");
		outFile << score;
		outFile.close();
	}
}

void ScoreLoad()
{
	std::ifstream inFile("Score.txt");
	inFile >> hiscore;
	inFile.close();
}

// DISPLAY ==========================================================================
void DisplayInit()
{
	al_set_new_display_option(ALLEGRO_SAMPLE_BUFFERS, 1, ALLEGRO_SUGGEST);
	al_set_new_display_option(ALLEGRO_SAMPLES, 8, ALLEGRO_SUGGEST);

	displayPtr = al_create_display(DisplayW, DisplayH);
	Utilities::MustInit(displayPtr, "Display");

	bufferPtr = al_create_bitmap(BufferW, BufferH);
	Utilities::MustInit(bufferPtr, "Buffer");
}

void DisplayDeinit()
{
	al_destroy_display(displayPtr);
	al_destroy_bitmap(bufferPtr);
}

void DisplayPreDraw()
{
	al_set_target_bitmap(bufferPtr);
}

void DisplayPostDraw()
{
	al_set_target_backbuffer(displayPtr);
	al_draw_scaled_bitmap(bufferPtr, 0.f, 0.f, BufferW, BufferH, 0, 0, DisplayW, DisplayH, NULL);
}

// KEYBOARD ==========================================================================
void KeyboardInit()
{
	memset(key, 0, sizeof(key));
}

void KeyboardUpdate(ALLEGRO_EVENT* event)
{
	switch (event->type)
	{
	case ALLEGRO_EVENT_TIMER:
		for (int i = 0; i < ALLEGRO_KEY_MAX; i++)
		{
			key[i] &= KeySeen;
		}
		break;

	case ALLEGRO_EVENT_KEY_DOWN:
		key[event->keyboard.keycode] = KeySeen | KeyReleased;
		break;

	case ALLEGRO_EVENT_KEY_UP:
		key[event->keyboard.keycode] &= KeyReleased;
		break;

	default:
		break;
	}
}

// SPRITES ===========================================================================
void SpritesInit()
{
	sprites._spriteSheet = al_load_bitmap("Assets/Sprites/SpriteSheet.png");

	// Logo.
	sprites._logo = al_load_bitmap("Assets/Sprites/Logo.png");

	// Cannon.
	sprites.cannon = SpriteGet(0, 0, CannonW, CannonH);
	sprites.cannonExplosion = SpriteGet(13, 0, CannonExplosionW, CannonExplosionH);
	sprites.cannonLaser = SpriteGet(62, 0, LaserW[LASER_TYPE_CANNON], LaserH[LASER_TYPE_CANNON]);

	// Laser.
	sprites.laserExplosionUp = SpriteGet(49, 8, LaserExplosionW, LaserExplosionH);
	sprites.laserExplosionDown = SpriteGet(43, 8, LaserExplosionW, LaserExplosionH);

	// Invaders.
	sprites.invaderSquid[0] = SpriteGet(0, 8, InvaderW[INVADER_TYPE_SQUID], InvaderH[INVADER_TYPE_SQUID]);
	sprites.invaderSquid[1] = SpriteGet(0, 8, InvaderW[INVADER_TYPE_SQUID], InvaderH[INVADER_TYPE_SQUID]);
	sprites.invaderCrab[0] = SpriteGet(8, 8, InvaderW[INVADER_TYPE_CRAB], InvaderH[INVADER_TYPE_CRAB]);
	sprites.invaderCrab[1] = SpriteGet(19, 8, InvaderW[INVADER_TYPE_CRAB], InvaderH[INVADER_TYPE_CRAB]);
	sprites.invaderOctopus[0] = SpriteGet(29, 0, InvaderW[INVADER_TYPE_OCTOPUS], InvaderH[INVADER_TYPE_OCTOPUS]);
	sprites.invaderOctopus[1] = SpriteGet(41, 0, InvaderW[INVADER_TYPE_OCTOPUS], InvaderH[INVADER_TYPE_OCTOPUS]);

	sprites.invaderExplosion = SpriteGet(30, 8, InvaderExplosionW, InvaderExplosionH);
	sprites.invaderLaser = SpriteGet(59, 0, LaserW[LASER_TYPE_INVADER], LaserH[LASER_TYPE_INVADER]);
	sprites.invaderLaserWiggly = SpriteGet(53, 0, LaserW[LASER_TYPE_INVADER_WIGGLY], LaserH[LASER_TYPE_INVADER_WIGGLY]);

	// Shield.
	sprites.shieldComplete[0] = SpriteGet(37, 16, ShieldW, ShieldH);
	sprites.shieldComplete[1] = SpriteGet(30, 16, ShieldW, ShieldH);
	sprites.shieldComplete[2] = SpriteGet(16, 16, ShieldW, ShieldH);
	sprites.shieldComplete[3] = SpriteGet(30, 16, ShieldW, ShieldH);
	sprites.shieldComplete[4] = SpriteGet(30, 16, ShieldW, ShieldH);
	sprites.shieldComplete[5] = SpriteGet(30, 16, ShieldW, ShieldH);
	sprites.shieldComplete[6] = SpriteGet(30, 16, ShieldW, ShieldH);
	sprites.shieldComplete[7] = SpriteGet(30, 16, ShieldW, ShieldH);
	sprites.shieldComplete[8] = SpriteGet(30, 16, ShieldW, ShieldH);
	sprites.shieldComplete[9] = SpriteGet(23, 16, ShieldW, ShieldH);
	sprites.shieldComplete[10] = SpriteGet(44, 16, ShieldW, ShieldH);

	sprites.shieldDestroyed[0] = SpriteGet(16, 20, ShieldW, ShieldH);
	sprites.shieldDestroyed[1] = SpriteGet(23, 20, ShieldW, ShieldH);
	sprites.shieldDestroyed[2] = SpriteGet(30, 20, ShieldW, ShieldH);
	sprites.shieldDestroyed[3] = SpriteGet(55, 7, ShieldW, ShieldH);
	sprites.shieldDestroyed[4] = SpriteGet(55, 11, ShieldW, ShieldH);
	sprites.shieldDestroyed[5] = SpriteGet(55, 15, ShieldW, ShieldH);
	sprites.shieldDestroyed[6] = SpriteGet(37, 20, ShieldW, ShieldH);
	sprites.shieldDestroyed[7] = SpriteGet(44, 20, ShieldW, ShieldH);
	sprites.shieldDestroyed[8] = SpriteGet(51, 20, ShieldW, ShieldH);
	sprites.shieldDestroyed[9] = SpriteGet(16, 20, ShieldW, ShieldH);
	sprites.shieldDestroyed[10] = SpriteGet(37, 20, ShieldW, ShieldH);
}

void SpritesDeinit()
{
	al_destroy_bitmap(sprites._spriteSheet);
	al_destroy_bitmap(sprites._logo);

	// Cannon.
	al_destroy_bitmap(sprites.cannon);
	al_destroy_bitmap(sprites.cannonExplosion);
	al_destroy_bitmap(sprites.cannonLaser);

	// Laser.
	al_destroy_bitmap(sprites.laserExplosionUp);
	al_destroy_bitmap(sprites.laserExplosionDown);

	// Invaders.
	al_destroy_bitmap(sprites.invaderSquid[0]);
	al_destroy_bitmap(sprites.invaderSquid[1]);
	al_destroy_bitmap(sprites.invaderCrab[0]);
	al_destroy_bitmap(sprites.invaderCrab[1]);
	al_destroy_bitmap(sprites.invaderOctopus[0]);
	al_destroy_bitmap(sprites.invaderOctopus[1]);

	al_destroy_bitmap(sprites.invaderExplosion);
	al_destroy_bitmap(sprites.invaderLaser);
	al_destroy_bitmap(sprites.invaderLaserWiggly);

	// Shields.
	for (int i = ShieldPieces - 1; i > 0; i--)
	{
		al_destroy_bitmap(sprites.shieldComplete[i]);
		al_destroy_bitmap(sprites.shieldDestroyed[i]);
	}
}

ALLEGRO_BITMAP* SpriteGet(int x, int y, int w, int h)
{
	ALLEGRO_BITMAP* sprite = al_create_sub_bitmap(sprites._spriteSheet, x, y, w, h);
	Utilities::MustInit(sprite, "SpriteGet");

	return sprite;
}

// AUDIO =============================================================================
void AudioInit()
{
	al_install_audio();
	al_init_acodec_addon();
	al_reserve_samples(256);

	sampleCannonExplosion = al_load_sample("Assets/Audio/CannonExplosion.wav");
	Utilities::MustInit(sampleCannonExplosion, "CannonExplosion.wav");

	sampleInvaderExplosion = al_load_sample("Assets/Audio/InvaderExplosion.wav");
	Utilities::MustInit(sampleInvaderExplosion, "InvaderExplosion.wav");

	sampleLaser = al_load_sample("Assets/Audio/Laser.wav");
	Utilities::MustInit(sampleLaser, "Laser.wav");
}

void AudioDeinit()
{
	al_destroy_sample(sampleCannonExplosion);
	al_destroy_sample(sampleInvaderExplosion);
	al_destroy_sample(sampleLaser);
}

// FONTS =============================================================================
void FontsInit()
{
	mainFont = al_load_font("Assets/Fonts/monogram.ttf", 16, NULL);
	Utilities::MustInit(mainFont, "font");
}

void FontsDeinit()
{
	al_destroy_font(mainFont);
}

// FX ================================================================================
void FxInit()
{
	// Cannon explosion.
	fx[FxCannonExplosionIndex].x = 0;
	fx[FxCannonExplosionIndex].y = 0;
	fx[FxCannonExplosionIndex].w = CannonExplosionW;
	fx[FxCannonExplosionIndex].h = CannonExplosionH;
	fx[FxCannonExplosionIndex].timer = CannonExplosionFrames;
	fx[FxCannonExplosionIndex].isAvalilable = true;
	fx[FxCannonExplosionIndex].sprite = sprites.cannonExplosion;

	// Invader explosion.
	fx[FxInvaderExplosionIndex].x = 0;
	fx[FxInvaderExplosionIndex].y = 0;
	fx[FxInvaderExplosionIndex].w = InvaderExplosionW;
	fx[FxInvaderExplosionIndex].h = InvaderExplosionH;
	fx[FxInvaderExplosionIndex].timer = InvaderFleetDestroyedTime;
	fx[FxInvaderExplosionIndex].isAvalilable = true;
	fx[FxInvaderExplosionIndex].sprite = sprites.invaderExplosion;

	// Laser explosion.
	for (int i = FxLaserExplosionIndex; i < FxN; i++)
	{
		fx[i].w = LaserExplosionW;
		fx[i].h = LaserExplosionH;
		fx[i].isAvalilable = true;
	}
}

void FxUpdate()
{
	// Todo: unified all the logic.
	// Cannon explosion.
	if (!fx[FxCannonExplosionIndex].isAvalilable)
	{
		fx[FxCannonExplosionIndex].timer--;
		if (fx[FxCannonExplosionIndex].timer == 0) fx[FxCannonExplosionIndex].isAvalilable = true;
	}

	// Invader explosion.
	if (!fx[FxInvaderExplosionIndex].isAvalilable)
	{
		fx[FxInvaderExplosionIndex].timer--;
		if (fx[FxInvaderExplosionIndex].timer == 0) fx[FxInvaderExplosionIndex].isAvalilable = true;
	}

	// Laser explosion.
	for (int i = FxLaserExplosionIndex; i < FxN; i++)
	{
		if (fx[i].isAvalilable) continue;

		fx[i].timer--;
		if (fx[i].timer == 0) fx[i].isAvalilable = true;
	}
}

void FxDraw()
{
	for (int i = 0; i < FxN; i++)
	{
		if (fx[i].isAvalilable) continue;

		al_draw_bitmap(fx[i].sprite, fx[i].x, fx[i].y, NULL);
	}
}

void FxAdd(int x, int y, FxType type)
{
	switch (type)
	{
	case FX_TYPE_CANNON_EXPLOSION:

		al_play_sample(sampleCannonExplosion, 1.f, 0.f, 1.f, ALLEGRO_PLAYMODE_ONCE, NULL);

		fx[FxCannonExplosionIndex].x = x;
		fx[FxCannonExplosionIndex].y = y;
		fx[FxCannonExplosionIndex].timer = CannonExplosionFrames;
		fx[FxCannonExplosionIndex].isAvalilable = false;
		break;

	case FX_TYPE_INVADER_EXPLOSION:

		al_play_sample(sampleInvaderExplosion, 1.f, 0.f, 1.f, ALLEGRO_PLAYMODE_ONCE, NULL);

		fx[FxInvaderExplosionIndex].x = x;
		fx[FxInvaderExplosionIndex].y = y;
		fx[FxInvaderExplosionIndex].timer = InvaderFleetDestroyedTime;
		fx[FxInvaderExplosionIndex].isAvalilable = false;
		break;

	case FX_TYPE_CANNON_LASER_EXPLOSION:

		for (int i = FxLaserExplosionIndex; i < FxN; i++)
		{
			if (!fx[i].isAvalilable) continue;

			fx[i].x = x;
			fx[i].y = y;
			fx[i].timer = LaserExplosionFrames;
			fx[i].isAvalilable = false;
			fx[i].sprite = sprites.laserExplosionUp;
			break;
		}

		break;

	case FX_TYPE_INVADER_LASER_EXPLOSION:

		for (int i = FxLaserExplosionIndex; i < FxN; i++)
		{
			if (!fx[i].isAvalilable) continue;

			fx[i].x = x;
			fx[i].y = y;
			fx[i].timer = LaserExplosionFrames;
			fx[i].isAvalilable = false;
			fx[i].sprite = sprites.laserExplosionDown;
			break;
		}

		break;
	}
}

// LASERS ============================================================================
void LaserInit()
{
	for (int i = 0; i < LaserN; i++)
	{
		lasers[i].isAvailable = true;
	}

	LaserFlipBitmap = false;
}

void LaserUpdate()
{
	// Cannon lasers.
	for (int i = 0; i < LaserCannonN; i++)
	{
		if (lasers[i].isAvailable) continue;

		lasers[i].y -= LaserCannonSpeed;

		// Check Y limit.
		if (lasers[i].y < -LaserH[(int)lasers[i].type])
		{
			lasers[i].isAvailable = true;
			FxAdd(lasers[i].x, 0, FX_TYPE_CANNON_LASER_EXPLOSION);
		}
	}

	// Invader lasers.
	for (int i = LaserCannonN; i < LaserInvaderN; i++)
	{
		if (lasers[i].isAvailable) continue;

		lasers[i].y += LaserinvaderSpeed;

		// Check ground colision.
		int laserPosition = lasers[i].y + LaserH[(int)lasers[i].type];
		if (laserPosition >= GroundPosition)
		{
			lasers[i].isAvailable = true;
			FxAdd(lasers[i].x, GroundPosition - LaserExplosionH, FX_TYPE_INVADER_LASER_EXPLOSION);
		}

		// Check cannon laser collision.
		if (LaserCollide(true, lasers[i].x, lasers[i].y, LaserW[lasers[i].type], LaserH[lasers[i].type]))
		{
			bool destroyLaser = true;
			if (lasers[i].type == LASER_TYPE_INVADER_WIGGLY)
			{
				if (Random::get(0, 100) >= 80) destroyLaser = false;
			}

			lasers[i].isAvailable = destroyLaser;

			FxAdd(lasers[i].x, lasers[i].y, FX_TYPE_CANNON_LASER_EXPLOSION);
		}
	}

	// Flip bitmaps.
	if ((frames % LaserFlipFrames) == 0) LaserFlipBitmap = !LaserFlipBitmap;
}

void LaserDraw()
{
	for (int i = 0; i < LaserN; i++)
	{
		if (lasers[i].isAvailable) continue;

		al_draw_bitmap(lasers[i].sprite, lasers[i].x, lasers[i].y, LaserFlipBitmap);
	}
}

void LaserClear()
{
	for (int i = 0; i < LaserN; i++)
	{
		lasers[i].isAvailable = true;
	}
}

bool LaserAdd(int x, int y, LaserType type)
{
	switch (type)
	{
	case LASER_TYPE_CANNON:

		for (int i = 0; i < LaserCannonN; i++)
		{
			if (!lasers[i].isAvailable) continue;

			al_play_sample(sampleLaser, 0.8f, 0.f, 1.f, ALLEGRO_PLAYMODE_ONCE, NULL);

			lasers[i].x = x;
			lasers[i].y = y;
			lasers[i].type = type;
			lasers[i].isAvailable = false;
			lasers[i].sprite = sprites.cannonLaser;
			return true;
		}

		break;
	case LASER_TYPE_INVADER:

		for (int i = LaserCannonN; i < LaserInvaderN; i++)
		{
			if (!lasers[i].isAvailable) continue;

			lasers[i].x = x;
			lasers[i].y = y;
			lasers[i].type = type;
			lasers[i].isAvailable = false;
			lasers[i].sprite = sprites.invaderLaser;
			return true;
		}

		break;
	case LASER_TYPE_INVADER_WIGGLY:

		for (int i = LaserCannonN; i < LaserInvaderN; i++)
		{
			if (!lasers[i].isAvailable) continue;

			lasers[i].x = x;
			lasers[i].y = y;
			lasers[i].type = type;
			lasers[i].isAvailable = false;
			lasers[i].sprite = sprites.invaderLaserWiggly;
			return true;
		}

		break;
	default:
		break;
	}

	return false;
}

bool LaserCollide(bool checkCannonLaser, int x, int y, int w, int h)
{
	if (checkCannonLaser)
	{
		for (int i = 0; i < LaserCannonN; i++)
		{
			if (lasers[i].isAvailable) continue;

			if (Utilities::CheckCollision(
				x, y,
				x + w, y + h,
				lasers[i].x, lasers[i].y,
				lasers[i].x + LaserW[lasers[i].type], lasers[i].y + LaserH[lasers[i].type]
			))
			{
				lasers[i].isAvailable = true;
				return true;
			}
		}

		return false;
	}
	else
	{
		for (int i = LaserCannonN; i < LaserInvaderN; i++)
		{
			if (lasers[i].isAvailable) continue;

			if (Utilities::CheckCollision(
				x, y,
				x + w, y + h,
				lasers[i].x, lasers[i].y,
				lasers[i].x + LaserW[lasers[i].type], lasers[i].y + LaserH[lasers[i].type]
			))
			{
				lasers[i].isAvailable = true;
				return true;
			}
		}
	}
	return false;
}

// CANNON ============================================================================
void CannonInit()
{
	cannon.x = (BufferW * 0.5f) - (CannonW * 0.5f);
	cannon.y = 216;

	cannon.lives = 3;

	// Sprites.
	cannon.sprite = sprites.cannon;
	cannon.spriteExplosion = sprites.cannonExplosion;

	// Shooting.
	canShoot = false;
}

void CannonUpdate()
{
	// Movement.
	if (key[ALLEGRO_KEY_RIGHT]) cannon.x += CannonSpeed;
	if (key[ALLEGRO_KEY_LEFT]) cannon.x -= CannonSpeed;

	// Position check.
	if (cannon.x <= CannonMinXPosition) cannon.x = CannonMinXPosition;
	if (cannon.x >= CannonMaxXPosition) cannon.x = CannonMaxXPosition;

	// Lasers collision.
	if (LaserCollide(false, cannon.x, cannon.y, CannonW, CannonH))
	{
		cannon.lives--;
		SwitchToState(GameState::Respawning);
	}

	// Lasers.
	if (key[ALLEGRO_KEY_SPACE] && canShoot && !IsInvaderDestroyed)
	{
		LaserAdd(cannon.x + (CannonW * 0.5f), cannon.y, LASER_TYPE_CANNON);
		canShoot = false;
	}
	else if (!key[ALLEGRO_KEY_SPACE] && !canShoot)
	{
		canShoot = true;
	}
}

void CannonDraw()
{
	al_draw_bitmap(cannon.sprite, cannon.x, cannon.y, NULL);
}

void CannonReset()
{
	int lives = cannon.lives;
	CannonInit();
	cannon.lives = lives;
}

// INVADER FLEET =====================================================================
void InvaderFleetInit()
{
	int x = 41;
	int y = 44;

	int cellW = 12;
	int cellH = 8;

	int cellSpace = 4;

	int fleetCellW = cellW + cellSpace;
	int fleetCellH = cellH + cellSpace;

	int row = 0;
	// Invader: Squid.
	for (; row < 1; row++)
	{
		for (int column = 0; column < InvaderFleetColumns; column++)
		{
			invaderFleet[row][column].x = x + 2 + (column * fleetCellW);
			invaderFleet[row][column].y = y + (row * fleetCellH);
			invaderFleet[row][column].isAlive = true;
			invaderFleet[row][column].type = INVADER_TYPE_SQUID;
			invaderFleet[row][column].sprite[0] = sprites.invaderSquid[0];
			invaderFleet[row][column].sprite[1] = sprites.invaderSquid[1];
		}
	}

	// Invader: Crab.
	for (row = 1; row < 3; row++)
	{
		for (int column = 0; column < InvaderFleetColumns; column++)
		{
			invaderFleet[row][column].x = x + (column * fleetCellW);
			invaderFleet[row][column].y = y + (row * fleetCellH);
			invaderFleet[row][column].isAlive = true;
			invaderFleet[row][column].type = INVADER_TYPE_CRAB;
			invaderFleet[row][column].sprite[0] = sprites.invaderCrab[0];
			invaderFleet[row][column].sprite[1] = sprites.invaderCrab[1];
		}
	}

	// Invader: Octopus.
	for (row = 3; row < InvaderFleetRows; row++)
	{
		for (int column = 0; column < InvaderFleetColumns; column++)
		{
			invaderFleet[row][column].x = x + (column * fleetCellW);
			invaderFleet[row][column].y = y + (row * fleetCellH);
			invaderFleet[row][column].isAlive = true;
			invaderFleet[row][column].type = INVADER_TYPE_OCTOPUS;
			invaderFleet[row][column].sprite[0] = sprites.invaderOctopus[0];
			invaderFleet[row][column].sprite[1] = sprites.invaderOctopus[1];
		}
	}

	// Sprites.
	invaderSpriteIndex = 0;

	// Fleet movement.
	invaderFleetMoveTime = (InvaderFleetN / 3) * 2;
	invaderFleetMoveTimer = invaderFleetMoveTime;

	invaderFleetNeedMoveY = false;

	invaderFleetMoveSpeedX = 3;
	invaderFleetMoveSpeedY = 6;

	// Invaders alive.
	invadersAlive = InvaderFleetN;

	// Shot.
	invaderFleetShotTimer = Random::get(InvaderFleetShotMinTime, InvaderFleetShotMaxTime);
	invaderFleetShotTimer = invaderFleetShotTime;

	// Invadr destroyed.
	IsInvaderDestroyed = false;
}

void InvaderFleetUpdate()
{
	// Shot Timer --------------------------------------------------------------------
	if (invaderFleetShotTimer > 0) invaderFleetShotTimer--;

	// Check collisions and shooting.
	for (int row = 0; row < InvaderFleetRows; row++)
	{
		for (int column = 0; column < InvaderFleetColumns; column++)
		{
			if (!invaderFleet[row][column].isAlive) continue;

			// Laser collision -------------------------------------------------------
			if (LaserCollide(true,
				invaderFleet[row][column].x,
				invaderFleet[row][column].y,
				InvaderW[invaderFleet[row][column].type],
				InvaderH[invaderFleet[row][column].type]))
			{
				invaderFleet[row][column].isAlive = false;

				// Add points to score.
				score += InvaderPoints[invaderFleet[row][column].type];

				// Reduce movement timer.
				if (invaderFleetMoveTime > 1) invaderFleetMoveTime--;

				// Wait before move.
				invaderFleetMoveTimer = InvaderFleetDestroyedTime;
				IsInvaderDestroyed = true;

				// Show effect.
				int x = invaderFleet[row][column].x;
				int y = invaderFleet[row][column].y;
				if (invaderFleet[row][column].type == INVADER_TYPE_SQUID) x -= 2;

				FxAdd(x, y, FX_TYPE_INVADER_EXPLOSION);

				// Invaders alive.
				invadersAlive--;
				if (invadersAlive == 0)
				{
					SwitchToState(GameState::GameOver);
					return;
				}
				else if (invadersAlive == 1)
				{
					invaderFleetMoveSpeedX *= 2;
				}

				continue;
			}

			// Lasers ----------------------------------------------------------------
			if ((invaderFleetShotTimer <= 0) && (Random::get(0, 100) >= 90))
			{
				int type = LASER_TYPE_INVADER;
				if (Random::get(0, 100) >= 80) type = LASER_TYPE_INVADER_WIGGLY;

				LaserAdd(
					invaderFleet[row][column].x + (int)(InvaderW[invaderFleet[row][column].type] * 0.5f),
					invaderFleet[row][column].y,
					(LaserType)type);

				// Reset shot timer.
				invaderFleetShotTimer = Random::get(InvaderFleetShotMinTime, InvaderFleetShotMaxTime);
			}

			// Shield collision ------------------------------------------------------
			for (int i = 0; i < ShieldPiecesSize; i++)
			{
				if (Utilities::CheckCollision(
					invaderFleet[row][column].x,
					invaderFleet[row][column].y,
					invaderFleet[row][column].x + InvaderW[invaderFleet[row][column].type],
					invaderFleet[row][column].y + InvaderH[invaderFleet[row][column].type],
					shields[i].x,
					shields[i].y,
					shields[i].x + ShieldW,
					shields[i].y + ShieldH
				))
				{
					shields[i].integrity = SHIELD_INTEGRITY_DESTROYED;
				}
			}

			// Cannon collision ------------------------------------------------------
			if (Utilities::CheckCollision(
				invaderFleet[row][column].x,
				invaderFleet[row][column].y,
				invaderFleet[row][column].x + InvaderW[invaderFleet[row][column].type],
				invaderFleet[row][column].y + InvaderH[invaderFleet[row][column].type],
				cannon.x,
				cannon.y,
				cannon.x + CannonW,
				cannon.y + CannonH))
			{
				SwitchToState(GameState::GameOver);
				return;
			}
		}
	}

	// Move fleet timer --------------------------------------------------------------
	if (invaderFleetMoveTimer > 0)
	{
		invaderFleetMoveTimer--;
		return;
	}
	IsInvaderDestroyed = false;

	// Move in X ---------------------------------------------------------------------
	if (!invaderFleetNeedMoveY)
	{
		bool changeDirection = false;
		for (int row = 0; row < InvaderFleetRows; row++)
		{
			for (int column = 0; column < InvaderFleetColumns; column++)
			{
				if (!invaderFleet[row][column].isAlive) continue;

				// Move ------------------------------------------------------------------
				invaderFleet[row][column].x += invaderFleetMoveSpeedX;

				// Check movement bounds -------------------------------------------------
				if (!changeDirection &&
					(invaderFleet[row][column].x <= HorizontalPadding) ||
					((invaderFleet[row][column].x + InvaderW[invaderFleet[row][column].type]) >= (BufferW - HorizontalPadding)))
				{
					changeDirection = true;
					invaderFleetNeedMoveY = true;
				}
			}
		}

		// Change direction.
		if (changeDirection) invaderFleetMoveSpeedX *= -1;

		// Sprite index.
		invaderSpriteIndex = invaderSpriteIndex == 0 ? 1 : 0;
	}

	// Move in Y ---------------------------------------------------------------------
	else if (invaderFleetNeedMoveY)
	{
		for (int row = 0; row < InvaderFleetRows; row++)
		{
			for (int column = 0; column < InvaderFleetColumns; column++)
			{
				if (!invaderFleet[row][column].isAlive) continue;

				// Move ------------------------------------------------------------------
				invaderFleet[row][column].y += invaderFleetMoveSpeedY;

				// Check movement bounds -------------------------------------------------
				if (invaderFleet[row][column].y + InvaderH[invaderFleet[row][column].type] >= GroundPosition - 4)
				{
					SwitchToState(GameState::GameOver);
					return;
				}
			}
		}

		invaderFleetNeedMoveY = false;
	}

	// Reset move timer --------------------------------------------------------------
	invaderFleetMoveTimer = invaderFleetMoveTime;
}

void InvaderFleetDraw()
{
	for (int row = 0; row < InvaderFleetRows; row++)
	{
		for (int column = 0; column < InvaderFleetColumns; column++)
		{
			if (!invaderFleet[row][column].isAlive) continue;

			al_draw_bitmap(
				invaderFleet[row][column].sprite[invaderSpriteIndex],
				invaderFleet[row][column].x,
				invaderFleet[row][column].y,
				NULL
			);
		}
	}
}

// HUD ===============================================================================
void HUDInit()
{
}

void HUDUpdate()
{
}

void HUDDraw()
{
	// Score.
	al_draw_text(mainFont, COLOR_HUD_TEXT, 34, 4, NULL, "SCORE");
	al_draw_textf(mainFont, COLOR_HUD_TEXT, 36, 16, NULL, "%04d", score);

	al_draw_text(mainFont, COLOR_HUD_TEXT, 98, 4, NULL, "HI-SCORE");
	al_draw_textf(mainFont, COLOR_HUD_TEXT, 110, 16, NULL, "%04d", hiscore);

	// Lives.
	al_draw_textf(mainFont, COLOR_HUD_TEXT, 34, 240, NULL, "%4d", cannon.lives);
	for (int i = 0; i < cannon.lives; i++)
	{
		al_draw_bitmap(
			sprites.cannon,
			HorizontalPadding + 45 + ((CannonW + 3) * i),
			243,
			NULL
		);
	}
}

// SHIELDS ==========================================================================
void ShieldsInit()
{
	// Setup shield integrity.
	for (int i = 0; i < ShieldPiecesSize; i++)
	{
		shields[i].integrity = SHIELD_INTEGRITY_COMPLETE;
	}

	// Setup sprites.
	SetupShield(45, 192, 0);
	SetupShield(93, 192, 1);
	SetupShield(141, 192, 2);
	SetupShield(189, 192, 3);
}

void ShieldsUpdate()
{
	for (int i = 0; i < ShieldPiecesSize; i++)
	{
		if (shields[i].integrity == SHIELD_INTEGRITY_DESTROYED) continue;

		// Check cannon laser.
		if (LaserCollide(true, shields[i].x, shields[i].y, ShieldW, ShieldH))
		{
			shields[i].integrity++;
		}

		// Check invader lasers.
		else if (LaserCollide(false, shields[i].x, shields[i].y, ShieldW, ShieldH))
		{
			shields[i].integrity++;
		}
	}
}

void ShieldsDraw()
{
	for (int i = 0; i < ShieldPiecesSize; i++)
	{
		if (shields[i].integrity == SHIELD_INTEGRITY_DESTROYED) continue;

		if (shields[i].integrity == SHIELD_INTEGRITY_COMPLETE)
		{
			al_draw_bitmap(shields[i].sprite[0], shields[i].x, shields[i].y, NULL);
		}
		else
		{
			al_draw_bitmap(shields[i].sprite[1], shields[i].x, shields[i].y, NULL);
		}
	}
}

void SetupShield(int x, int y, int shield)
{
	int rowN = 4;
	int columnN = 3;
	int index = 0;
	int indexSprite = 0;

	int shieldPosition = shield * ShieldPieces;

	// Setup position.
	for (int row = 0; row < rowN; row++)
	{
		for (int column = 0; column < columnN; column++)
		{
			index = shieldPosition + (row * columnN) + column;
			indexSprite = index - shieldPosition;

			shields[index].x = x + (ShieldW * column);
			shields[index].y = y + (ShieldH * row);
			shields[index].sprite[0] = sprites.shieldComplete[indexSprite];
			shields[index].sprite[1] = sprites.shieldDestroyed[indexSprite];
		}
	}

	// Fix the last row.
	shields[index - 1].x = x + (ShieldW * (columnN - 1));
}

// MAIN MENU =========================================================================

void MainMenuInit()
{
	showText = true;
}

void MainMenuUpdate()
{
	if (showText && (frames % 50) == 0) showText = false;
	else if (frames % 25 == 0) showText = true;

	if (key[ALLEGRO_KEY_ENTER]) SwitchToState(GameState::Gameplay);
}

void MainMenuDraw()
{
	al_clear_to_color(COLOR_BACKGROUND_MAIN_MENU);

	if (showText)
	{
		al_draw_text(
			mainFont,
			COLOR_TEXT_MAIN_MENU,
			mainMenuTextXPosition,
			mainMenuTextYPosition,
			ALLEGRO_ALIGN_CENTER,
			"PRESS [ENTER] TO START");
	}

	// Points (invaders).
	al_draw_bitmap(sprites.invaderSquid[0], 85, 180, NULL);
	al_draw_bitmap(sprites.invaderCrab[0], 85, 200, NULL);
	al_draw_bitmap(sprites.invaderOctopus[0], 85, 220, NULL);

	// Points (text).
	al_draw_text(mainFont, COLOR_TEXT_MAIN_MENU, 95, 176, NULL, " = 30 POINTS");
	al_draw_text(mainFont, COLOR_TEXT_MAIN_MENU, 95, 196, NULL, " = 20 POINTS");
	al_draw_text(mainFont, COLOR_TEXT_MAIN_MENU, 95, 216, NULL, " = 10 POINTS");
}

// GAMEPLAY ==========================================================================
void GameplayInit()
{
	score = 0;

	ScoreLoad();

	LaserInit();
	CannonInit();
	InvaderFleetInit();
	ShieldsInit();
	FxInit();
	HUDInit();
}

void GameplayUpdate()
{
	CannonUpdate();
	InvaderFleetUpdate();
	ShieldsUpdate();
	LaserUpdate();
	FxUpdate();
	HUDUpdate();
}

void GameplayDraw()
{
	al_clear_to_color(COLOR_BACKGROUND_GAMEPLAY);

	al_draw_line(HorizontalPadding, GroundPosition, BufferW - HorizontalPadding, GroundPosition, COLOR_GROUND, 2.f);

	LaserDraw();
	ShieldsDraw();
	CannonDraw();
	InvaderFleetDraw();
	FxDraw();
	HUDDraw();
}

// RESPAWNING =======================================================================
void RespawningInit()
{
	LaserClear();

	FxAdd(cannon.x, cannon.y, FX_TYPE_CANNON_EXPLOSION);

	respawnTimer = CannonExplosionFrames;
}

void RespawningUpdate()
{
	FxUpdate();

	respawnTimer--;
	if (respawnTimer > 0) return;

	if (cannon.lives == 0) SwitchToState(GameState::GameOver);
	else SwitchToState(GameState::Gameplay, false);
}

void RespawningDraw()
{
	al_clear_to_color(COLOR_BACKGROUND_GAMEEOVER);

	al_draw_line(HorizontalPadding, GroundPosition, BufferW - HorizontalPadding, GroundPosition, COLOR_GROUND, 2.f);

	InvaderFleetDraw();
	LaserDraw();
	ShieldsDraw();
	FxDraw();
	HUDDraw();
}

// GAMEOVER ==========================================================================
void GameOverInit()
{
	if (cannon.lives == 0) cannon.sprite = sprites.cannonExplosion;

	ScoreSave();
	showText = true;
}

void GameOverUpdate()
{
	if (showText && (frames % 50) == 0) showText = false;
	else if (frames % 25 == 0) showText = true;

	if (key[ALLEGRO_KEY_SPACE]) SwitchToState(GameState::MainMenu);
}

void GameOverDraw()
{
	al_clear_to_color(COLOR_BACKGROUND_GAMEEOVER);

	al_draw_line(HorizontalPadding, GroundPosition, BufferW - HorizontalPadding, GroundPosition, COLOR_GROUND, 2.f);

	CannonDraw();
	InvaderFleetDraw();
	LaserDraw();
	ShieldsDraw();
	HUDDraw();

	al_draw_text(mainFont, COLOR_TEXT_GAMEOVER, gameoverTextX, gameoverTextY, ALLEGRO_ALIGN_CENTER, "GAME OVER");

	if (showText)
	{
		al_draw_text(mainFont, COLOR_TEXT_GAMEOVER, gameoverTextX, gameoverRestartTextY, ALLEGRO_ALIGN_CENTER, "PRESS [SPACE] TO RESET");
	}
}

// ===================================================================================