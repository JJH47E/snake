/*******************************************************************************************
*
*   Snake by JJH47E
*
*   Modified by Graphene
*
*   This game has been created using vitasdk/dolcesdk (https://vitasdk.org/; https://github.com/DolceSDK)
*
********************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <psp2/ctrl.h>
#include <psp2/kernel/processmgr.h>
#include <psp2/kernel/modulemgr.h>
#include <psp2/kernel/clib.h>
#include <psp2/kernel/rng.h>
#include <psp2/apputil.h>
#include <psp2/sas.h>
#include <psp2/libdbg.h>
#include <psp2/fios2.h>
#include <psp2/types.h>

#include <vita2d_sys.h>
#include <vitaSAS.h>

typedef enum SnakeGameStatus {
	STATUS_IN_GAME,
	STATUS_IN_SETTINGS,
	STATUS_IN_MENU,
	STATUS_IN_POST_GAME
} SnakeGameStatus;

typedef enum SnakeSoundId {
	SND_EAT,
	SND_SELECT,
	SND_DEATH
} SnakeSoundId;

static const int screenWidth = 960;
static const int screenHeight = 544;
static SceFVector2 snake[300];

// FIOS2 stuff
//--------------------------------------------------------------------------------------
#define RAMCACHEBLOCKSIZE 64 * 1024
#define MAX_PATH_LENGTH 256

static SceInt64 s_op_storage[SCE_FIOS_OP_STORAGE_SIZE(64, MAX_PATH_LENGTH) / sizeof(SceInt64) + 1];
static SceInt64 s_chunk_storage[SCE_FIOS_CHUNK_STORAGE_SIZE(1024) / sizeof(SceInt64) + 1];
static SceInt64 s_fh_storage[SCE_FIOS_FH_STORAGE_SIZE(32, MAX_PATH_LENGTH) / sizeof(SceInt64) + 1];
static SceInt64 s_dh_storage[SCE_FIOS_DH_STORAGE_SIZE(32, MAX_PATH_LENGTH) / sizeof(SceInt64) + 1];

static SceFiosPsarcDearchiverContext s_dearchiver_context = SCE_FIOS_PSARC_DEARCHIVER_CONTEXT_INITIALIZER;
static SceByte s_dearchiver_work_buffer[3 * 64 * 1024] __attribute__((aligned(64)));
static SceInt32 s_archive_index = 0;
static SceFiosBuffer s_mount_buffer = SCE_FIOS_BUFFER_INITIALIZER;
static SceFiosFH s_archive_fh = -1;

SceFiosRamCacheContext s_ramcache_context = SCE_FIOS_RAM_CACHE_CONTEXT_INITIALIZER;
static SceByte s_ramcache_work_buffer[10 * (64 * 1024)] __attribute__((aligned(8)));

void drawGrid(int offsetX, int offsetY, int x, int y, int upperOffset, unsigned int bg, unsigned int fg){
    //draw vertical lines
    for(int i = 0; i < x + 1; i++){
        SceFVector2 startV = {offsetX + (i * (screenWidth - 2 * offsetX) / x), offsetY + upperOffset};
        SceFVector2 endV = {offsetX + (i * (screenWidth - 2 * offsetX) / x), screenHeight - offsetY};
        vita2d_draw_line(startV.x, startV.y, endV.x, endV.y, bg);
        vita2d_draw_line(startV.x + 1, startV.y, endV.x + 1, endV.y, bg);
    }
    //draw horizontal lines
    for(int i = 0; i < y + 1 + 1; i++){
        SceFVector2 startV = {offsetX, offsetY + upperOffset + (i * (screenHeight - (2 * offsetY + upperOffset)) / y)};
        SceFVector2 endV = {screenWidth - offsetX, offsetY + upperOffset + (i * (screenHeight - (2 * offsetY + upperOffset)) / y)};
        vita2d_draw_line(startV.x, startV.y, endV.x, endV.y, bg);
        vita2d_draw_line(startV.x, startV.y + 1, endV.x, endV.y + 1, bg);
    }
}

void drawSnakeBit(SceFVector2 coords, unsigned int bg, unsigned int fg){
    SceFVector2 position = {(coords.x * 40) + 20, (coords.y * 40) + 16 + 30};
    vita2d_draw_rectangle(position.x, position.y, 40, 40, fg);
}

void drawFoodBit(SceFVector2 coords, unsigned int bg, unsigned int fg){
    SceFVector2 position = {(((int) coords.x) * 40) + 20 + 10, (((int) coords.y) * 40) + 16 + 30 + 10};
    vita2d_draw_rectangle(position.x, position.y, 20, 20, fg);
}

void shiftSnake(SceFVector2 newVal, int length){
    for(int i = 0; i < length - 1; i++){
        snake[i] = snake[i + 1];
    }
    snake[length-1] = newVal;
}

int getCenteredTextX(vita2d_pvf* font, char* text, float scale)
{
	return ((screenWidth / 2) - vita2d_pvf_text_width(font, scale, text) / 2);
}

unsigned int getRandomNumber(unsigned int min, unsigned int max, unsigned int check_val)
{
repeat:;

	unsigned int result, temp_result;
	sceKernelGetRandomNumber(&temp_result, sizeof(int));
	result = temp_result % max;
	if (result == check_val || result < min)
		goto repeat;

	return result;
}

int psarcInit(void)
{
	SceFiosParams params = SCE_FIOS_PARAMS_INITIALIZER;
	params.opStorage.pPtr = s_op_storage;
	params.opStorage.length = sizeof(s_op_storage);
	params.chunkStorage.pPtr = s_chunk_storage;
	params.chunkStorage.length = sizeof(s_chunk_storage);
	params.fhStorage.pPtr = s_fh_storage;
	params.fhStorage.length = sizeof(s_fh_storage);
	params.dhStorage.pPtr = s_dh_storage;
	params.dhStorage.length = sizeof(s_dh_storage);
	params.pathMax = MAX_PATH_LENGTH;

	return sceFiosInitialize(&params);
}

void psarcOpenArchive(const char* path, const char* mountpoint)
{
	//dearchiver overlay
	s_dearchiver_context.workBufferSize = sizeof(s_dearchiver_work_buffer);
	s_dearchiver_context.pWorkBuffer = s_dearchiver_work_buffer;
	sceFiosIOFilterAdd(s_archive_index, sceFiosIOFilterPsarcDearchiver, &s_dearchiver_context);

	//ramcache
	s_ramcache_context.pPath = path;
	s_ramcache_context.workBufferSize = sizeof(s_ramcache_work_buffer);
	s_ramcache_context.pWorkBuffer = s_ramcache_work_buffer;
	s_ramcache_context.blockSize = RAMCACHEBLOCKSIZE;
	sceFiosIOFilterAdd(s_archive_index + 1, sceFiosIOFilterCache, &s_ramcache_context);

	SceFiosSize result = sceFiosArchiveGetMountBufferSizeSync(NULL, path, NULL);
	s_mount_buffer.length = (unsigned int)result;
	s_mount_buffer.pPtr = malloc(s_mount_buffer.length);
	sceFiosArchiveMountSync(NULL, &s_archive_fh, path, mountpoint, s_mount_buffer, NULL);
}

void psarcFinish(void)
{
	sceFiosArchiveUnmountSync(NULL, s_archive_fh);
	free(s_mount_buffer.pPtr);
	sceFiosIOFilterRemove(s_archive_index);
	sceFiosIOFilterRemove(s_archive_index + 1);
	sceFiosTerminate();
}

SceVoid _start(SceUInt32 args, ScePVoid argp)
{
    // Initialization
    //--------------------------------------------------------------------------------------

    SceCtrlData pad;
    srand(time(NULL));
    
    vita2d_texture *logo_gb;
    vita2d_texture *logo_matrix;
    vita2d_texture *logo_oled;
    vita2d_texture *logo;

	vitaSASAudio *eatFood;
	vitaSASAudio *selectSound;
	vitaSASAudio *deathSound;
    
    vita2d_pvf *font;
	void *fontBuffer;

	//Silence SceDbg for release build
	sceDbgSetMinimumLogLevel(SCE_DBG_LOG_LEVEL_ERROR);

	//Load vita2d_sys, vitaSAS modules
	sceKernelLoadStartModule("app0:module/vitaSAS.suprx", 0, NULL, 0, NULL, NULL);
	sceKernelLoadStartModule("app0:module/vita2d_sys.suprx", 0, NULL, 0, NULL, NULL);

    vita2d_init();
	vita2d_set_vblank_wait(0);
    vita2d_set_clear_color(RGBA8(0, 0, 0, 255));

	//Mount gamedata archive overlay
	psarcInit();
	psarcOpenArchive("app0:gamedata.psarc", "/gamedata");

    sceClibMemset(&pad, 0, sizeof(pad));

	int saved = 1;
    int snakeLength = 4;
    int headX = 10;
    int headY = 5;
    int gridX = 23;
    int gridY = 12;
    unsigned int xVal;
    unsigned int yVal;
    int counter;
	int appState = STATUS_IN_MENU;
    
    SceFVector2 start_2 = {(float) headX, (float) headY + 1};
    SceFVector2 start_3 = {(float) headX, (float) headY + 2};
    SceFVector2 start_4 = {(float) headX, (float) headY + 3};
    
    SceFVector2 newHead = {(float) headX, (float) headY};
    SceFVector2 food;
    
    snake[0] = start_4;
    snake[1] = start_3;
    snake[2] = start_2;
    snake[3] = newHead;
    
    bool right = false;
    bool left = false;
    bool up = true;
    bool down = false;
    bool endGame = false;
    
    bool last_r = false;
    bool last_l = false;
    bool last_u = false;
    bool last_d = false;
    
    bool showScore = false;
    bool showMulti = false;
    bool showFinalScore = false;
    bool showPostHint = false;
    
    int frameCounter = 0;
    int multi = 0;
    int frames = 20;
    
    //persistent data 
    int highScore = 0;
    int theme = 2;
    int difficulty = 1;
    
    bool newFood = true;
    
    bool gotInput = false;

    bool firstTimeBool = false;
    
    int listCounter = 0;
    
    unsigned int bg;
    unsigned int fg;
    
    char diffs[3][15] = {"EASY", "HARD", "IMPOSSIBLE"};
    char themeNames[3][15] = {"OLED", "MATRIX", "GAMEBOY"};
    char temp[250];
    
	// Graphics
	//----------------------------------------------------------------------------------
    logo_gb = vita2d_load_GXT_file("/gamedata/tex/texture.gxt", 0, VITA2D_IO_TYPE_FIOS2);
    logo_matrix = vita2d_load_additional_GXT(logo_gb, 1);
    logo_oled = vita2d_load_additional_GXT(logo_gb, 2);

	SceFiosStat fiosStat;
	sceClibMemset(&fiosStat, 0, sizeof(SceFiosStat));
	sceFiosStatSync(NULL, "/gamedata/fnt/font.otf", &fiosStat);
	fontBuffer = malloc((SceSize)fiosStat.fileSize);

	SceFiosFH fd;
	sceFiosFHOpenSync(NULL, &fd, "/gamedata/fnt/font.otf", NULL);
	sceFiosFHReadSync(NULL, fd, fontBuffer, (SceFiosSize)fiosStat.fileSize);
	sceFiosFHCloseSync(NULL, fd);

    font = vita2d_load_custom_pvf_buffer(fontBuffer, (SceSize)fiosStat.fileSize, 13.0f, 13.0f);

	// Audio
	//----------------------------------------------------------------------------------
	vitaSAS_init(SCE_FALSE);
	VitaSASSystemParam initParam;
	initParam.outputPort = SCE_AUDIO_OUT_PORT_TYPE_MAIN;
	initParam.samplingRate = 48000;
	initParam.numGrain = SCE_SAS_GRAIN_SAMPLES;
	initParam.thPriority = SCE_KERNEL_HIGHEST_PRIORITY_USER;
	initParam.thStackSize = 128 * 1024;
	initParam.thCpu = SCE_KERNEL_THREAD_CPU_AFFINITY_MASK_DEFAULT;
	initParam.isSubSystem = SCE_FALSE;
	initParam.subSystemNum = VITASAS_NO_SUBSYSTEM;

	vitaSAS_create_system_with_config("numGrains=256 numVoices=3 numReverbs=0", &initParam);

	vitaSASVoiceParam voiceParam;
	voiceParam.loop = SCE_SAS_LOOP_DISABLE;
	voiceParam.loopSize = -1;
	voiceParam.pitch = SCE_SAS_PITCH_BASE;
	voiceParam.volLDry = SCE_SAS_VOLUME_MAX;
	voiceParam.volRDry = SCE_SAS_VOLUME_MAX;
	voiceParam.volLWet = 0;
	voiceParam.volRWet = 0;
	voiceParam.adsr1 = SCE_SAS_ADSR_MODE_LINEAR_INC;
	voiceParam.adsr2 = SCE_SAS_ADSR_MODE_LINEAR_INC;

	eatFood = vitaSAS_load_audio_WAV("/gamedata/snd/food.wav", VITA2D_IO_TYPE_FIOS2);
	selectSound = vitaSAS_load_audio_WAV("/gamedata/snd/select.wav", VITA2D_IO_TYPE_FIOS2);
	deathSound = vitaSAS_load_audio_WAV("/gamedata/snd/death.wav", VITA2D_IO_TYPE_FIOS2);

	//Umount gamedata archive and finalize FIOS2, we loaded all of the assets at this point)
	psarcFinish();

	vitaSAS_set_voice_PCM(SND_EAT, eatFood, &voiceParam);
	vitaSAS_set_voice_PCM(SND_SELECT, selectSound, &voiceParam);
	vitaSAS_set_voice_PCM(SND_DEATH, deathSound, &voiceParam);
	

	// Safememory
	//----------------------------------------------------------------------------------
	SceAppUtilInitParam init_param;
	SceAppUtilBootParam boot_param;
	sceClibMemset(&init_param, 0, sizeof(SceAppUtilInitParam));
	sceClibMemset(&boot_param, 0, sizeof(SceAppUtilBootParam));
	sceAppUtilInit(&init_param, &boot_param);

	/*
	Safememory layout:
	{
		int isSaved;
		int theme;
		int difficulty;
		int highScore;
	}
	*/
	int saveState;
	sceAppUtilLoadSafeMemory(&saveState, sizeof(int), 0);

	if (!saveState) {
		highScore = 0;
		theme = 0;
		difficulty = 1;
	}
	else {
		sceAppUtilLoadSafeMemory(&theme, sizeof(int), sizeof(int));
		sceAppUtilLoadSafeMemory(&difficulty, sizeof(int), sizeof(int) * 2);
		sceAppUtilLoadSafeMemory(&highScore, sizeof(int), sizeof(int) * 3);
	}
    
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (1)
    {
        sceCtrlPeekBufferPositive(0, &pad, 1);
        // Update
        //----------------------------------------------------------------------------------
        if(theme == 0){
            //theme oled
            bg = RGBA8(0, 0, 0, 255);
            fg = RGBA8(255, 255, 255, 255);
            logo = logo_oled;
        }
        else if(theme == 1){
            //matrix theme
            bg = RGBA8(0, 0, 0, 255);
            fg = RGBA8(76, 220, 0, 255);
            logo = logo_matrix;
        }
        else {
            //theme gb
            theme = 2;
            bg = RGBA8(202, 220, 160, 255);
            fg = RGBA8(138, 172, 15, 255);
            logo = logo_gb;
        }
        vita2d_set_clear_color(bg);
        if(difficulty == 0){
            frames = 15;
        }
        else if(difficulty == 1){
            frames = 10;
        }
        else{
            frames = 5;
        }
        frameCounter++;
		switch (appState) {
		case STATUS_IN_GAME:
			if (pad.buttons & SCE_CTRL_RIGHT) {
				if (last_l == false) {
					gotInput = true;
					right = true;
					left = false;
					up = false;
					down = false;
				}
			}
			else if (pad.buttons & SCE_CTRL_LEFT) {
				if (last_r == false) {
					gotInput = true;
					left = true;
					right = false;
					up = false;
					down = false;
				}
			}
			else if (pad.buttons & SCE_CTRL_UP) {
				if (last_d == false) {
					gotInput = true;
					up = true;
					right = false;
					left = false;
					down = false;
				}
			}
			else if (pad.buttons & SCE_CTRL_DOWN) {
				if (last_u == false) {
					gotInput = true;
					down = true;
					right = false;
					left = false;
					up = false;
				}
			}
			if (newFood == true) {
				newFood = false;
				while (1) {
					counter = 0;
					yVal = getRandomNumber(0, gridY, yVal);
					xVal = getRandomNumber(0, gridX, xVal);
					for (int i = 0; i < snakeLength; i++) {
						if ((float)snake[i].x == (float)xVal && (float)snake[i].y == (float)yVal) {
							counter = 1;
							break;
						}
					}
					if (counter == 0) {
						break;
					}
				}
				SceFVector2 foodTemp = { (float)xVal, (float)yVal };
				food = foodTemp;
			}
			if (frameCounter > frames) {
				frameCounter = 0;
				for (int i = 0; i < snakeLength - 1; i++) {
					if (snake[i].x == newHead.x && snake[i].y == newHead.y) {
						endGame = true;
						break;
					}
				}
				if (right) {
					headX++;
					last_r = true;
					last_l = false;
					last_d = false;
					last_u = false;
				}
				else if (left) {
					headX--;
					last_l = true;
					last_d = false;
					last_u = false;
					last_r = false;
				}
				else if (down) {
					headY++;
					last_d = true;
					last_u = false;
					last_r = false;
					last_l = false;
				}
				else if (up) {
					headY--;
					last_u = true;
					last_r = false;
					last_l = false;
					last_d = false;
				}
				if (headX >= gridX) {
					headX = 0;
				}
				else if (headX < 0) {
					headX = gridX - 1;
				}
				if (headY >= gridY) {
					headY = 0;
				}
				else if (headY < 0) {
					headY = gridY - 1;
				}
				if ((newHead.x == food.x) && (newHead.y == food.y)) {
					snakeLength++;
					vitaSAS_set_key_on(SND_EAT);
					newFood = true;
					snake[snakeLength - 1] = food;
				}
				SceFVector2 headTemp = { (float)headX, (float)headY };
				newHead = headTemp;
				shiftSnake(newHead, snakeLength);
			}
			if (endGame == true) {
				vitaSAS_set_key_on(SND_DEATH);
				appState = STATUS_IN_POST_GAME;
				frameCounter = 0;
			}
			break;
		case STATUS_IN_MENU:
			if ((pad.buttons & SCE_CTRL_CROSS) && (firstTimeBool == true)) {
				firstTimeBool = false;
				vitaSAS_set_key_on(SND_SELECT);
				if (listCounter == 0) {
					appState = STATUS_IN_GAME;
					snakeLength = 4;
					headX = 10;
					headY = 5;
					gridX = 23;
					gridY = 12;

					SceFVector2 start_2 = { (float)headX, (float)headY + 1 };
					SceFVector2 start_3 = { (float)headX, (float)headY + 2 };
					SceFVector2 start_4 = { (float)headX, (float)headY + 3 };

					SceFVector2 newHead = { (float)headX, (float)headY };
					SceFVector2 food;

					snake[0] = start_4;
					snake[1] = start_3;
					snake[2] = start_2;
					snake[3] = newHead;

					right = false;
					left = false;
					up = true;
					down = false;
					endGame = false;
					last_r = false;
					last_l = false;
					last_u = false;
					last_d = false;

					newFood = true;
				}
				else {
					appState = STATUS_IN_SETTINGS;
					listCounter = 0;
				}
			}
			else if ((pad.buttons & SCE_CTRL_UP) && (firstTimeBool == true)) {
				firstTimeBool = false;
				vitaSAS_set_key_on(SND_SELECT);
				listCounter--;
				listCounter = abs(listCounter % 2);
			}
			else if ((pad.buttons & SCE_CTRL_DOWN) && (firstTimeBool == true)) {
				firstTimeBool = false;
				vitaSAS_set_key_on(SND_SELECT);
				listCounter++;
				listCounter = abs(listCounter % 2);
			}
			else if ((!(pad.buttons & SCE_CTRL_CROSS)) && (!(pad.buttons & SCE_CTRL_DOWN)) && (!(pad.buttons & SCE_CTRL_UP))) {
				firstTimeBool = true;
			}
			break;
		case STATUS_IN_SETTINGS:
			if ((pad.buttons & SCE_CTRL_DOWN) && (firstTimeBool == true)) {
				firstTimeBool = false;
				vitaSAS_set_key_on(SND_SELECT);
				listCounter++;
				listCounter = abs(listCounter % 3);
			}
			else if ((pad.buttons & SCE_CTRL_UP) && (firstTimeBool == true)) {
				firstTimeBool = false;
				vitaSAS_set_key_on(SND_SELECT);
				listCounter--;
				listCounter = abs(listCounter % 3);
			}
			else if ((pad.buttons & SCE_CTRL_RIGHT) && (firstTimeBool == true)) {
				firstTimeBool = false;
				vitaSAS_set_key_on(SND_SELECT);
				if (listCounter == 0) {
					difficulty++;
					difficulty = abs(difficulty % 3);
					sceAppUtilSaveSafeMemory(&difficulty, sizeof(int), sizeof(int) * 2);
				}
				else if (listCounter == 1) {
					theme++;
					theme = abs(theme % 3);
					sceAppUtilSaveSafeMemory(&theme, sizeof(int), sizeof(int));
				}
				sceAppUtilSaveSafeMemory(&saved, sizeof(int), 0);
			}
			else if ((pad.buttons & SCE_CTRL_LEFT) && (firstTimeBool == true)) {
				firstTimeBool = false;
				vitaSAS_set_key_on(SND_SELECT);
				if (listCounter == 0) {
					difficulty--;
					difficulty = abs(difficulty % 3);
					sceAppUtilSaveSafeMemory(&difficulty, sizeof(int), sizeof(int) * 2);
				}
				else if (listCounter == 1) {
					theme--;
					theme = abs(theme % 3);
					sceAppUtilSaveSafeMemory(&theme, sizeof(int), sizeof(int));
				}
				sceAppUtilSaveSafeMemory(&saved, sizeof(int), 0);
			}
			else if ((pad.buttons & SCE_CTRL_CROSS) && (firstTimeBool == true)) {
				firstTimeBool = false;
				vitaSAS_set_key_on(SND_SELECT);
				if (listCounter == 2) {
					appState = STATUS_IN_MENU;
					listCounter = 0;
				}
			}
			else if ((!(pad.buttons & SCE_CTRL_CROSS)) && (!(pad.buttons & SCE_CTRL_LEFT)) && (!(pad.buttons & SCE_CTRL_RIGHT)) && (!(pad.buttons & SCE_CTRL_DOWN)) && (!(pad.buttons & SCE_CTRL_UP))) {
				firstTimeBool = true;
			}
			break;
		case STATUS_IN_POST_GAME:
			if ((pad.buttons & SCE_CTRL_CROSS) && (firstTimeBool == true) && (showPostHint == true)) {
				firstTimeBool = false;
				vitaSAS_set_key_on(SND_SELECT);
				if ((snakeLength - 4) * multi > highScore) {
					highScore = (snakeLength - 4) * multi;
					sceAppUtilSaveSafeMemory(&saved, sizeof(int), 0);
					sceAppUtilSaveSafeMemory(&highScore, sizeof(int), sizeof(int) * 3);
				}
				showScore = false;
				showMulti = false;
				showFinalScore = false;
				showPostHint = false;
				frameCounter = 0;
				appState = STATUS_IN_MENU;
			}
			else if ((!(pad.buttons & SCE_CTRL_CROSS))) {
				firstTimeBool = true;
			}
			break;
		}

		//vitaSAS cleanup
		for (int i = 0; i < 3; i++) {
			if (vitaSAS_get_end_state(i)) {
				vitaSAS_set_key_off(i);
			}
		}

        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        vita2d_start_drawing();		
        vita2d_clear_screen();
		switch (appState) {
		case STATUS_IN_GAME:
			if (snakeLength - 4 < 10) {
				sprintf(temp, "00%i0", snakeLength - 4);
				vita2d_pvf_draw_text(font, 10, 25, fg, 1.2f, temp);
			}
			else {
				sprintf(temp, "0%i0", snakeLength - 4);
				vita2d_pvf_draw_text(font, 10, 25, fg, 1.2f, temp);
			}
			drawFoodBit(food, bg, fg);
			for (int i = snakeLength - 1; i >= 0; i--) {
				drawSnakeBit(snake[i], bg, fg);
			}
			drawGrid(20, 16, 23, 12, 30, bg, fg);
			break;
		case STATUS_IN_MENU:
			vita2d_pvf_draw_text(font, 10, 20, fg, 1.0f, "Made by JJH47E, Graphene");
			vita2d_draw_texture(logo, screenWidth / 2 - 231, 75 - 30);
			sprintf(temp, "HIGH SCORE: %i0", highScore);
			vita2d_pvf_draw_text(font, getCenteredTextX(font, temp, 1.2f), screenHeight - 75, fg, 1.2f, temp);
			if (listCounter == 0) {
				vita2d_pvf_draw_text(font, getCenteredTextX(font, "-PLAY-", 1.5f), 144 + 150, fg, 1.5f, "-PLAY-");
				vita2d_pvf_draw_text(font, getCenteredTextX(font, "SETTINGS", 1.5f), 225 + 150, fg, 1.5f, "SETTINGS");
			}
			else {
				vita2d_pvf_draw_text(font, getCenteredTextX(font, "PLAY", 1.5f), 144 + 150, fg, 1.5f, "PLAY");
				vita2d_pvf_draw_text(font, getCenteredTextX(font, "-SETTINGS-", 1.5f), 225 + 150, fg, 1.5f, "-SETTINGS-");
			}
			break;
		case STATUS_IN_POST_GAME:
			if (frameCounter > 30 || showScore == true) {
				showScore = true;
				if (snakeLength - 4 < 10) {
					sprintf(temp, "SCORE: 00%i0", snakeLength - 4);
					vita2d_pvf_draw_text(font, getCenteredTextX(font, temp, 1.2f), 150, fg, 1.2f, temp);
				}
				else {
					sprintf(temp, "SCORE: 0%i0", snakeLength - 4);
					vita2d_pvf_draw_text(font, getCenteredTextX(font, temp, 1.2f), 150, fg, 1.2f, temp);
				}
			}
			if (frameCounter > 60 || showMulti == true) {
				showMulti = true;
				if (difficulty == 1) {
					sprintf(temp, "DIFFICULTY MULTIPLIER: x%i", (int)2);
					vita2d_pvf_draw_text(font, getCenteredTextX(font, temp, 1.2f), 190, fg, 1.2f, temp);
					multi = 2;
				}
				else if (difficulty == 0) {
					sprintf(temp, "DIFFICULTY MULTIPLIER: x%i", (int)1);
					vita2d_pvf_draw_text(font, getCenteredTextX(font, temp, 1.2f), 190, fg, 1.2f, temp);
					multi = 1;
				}
				else {
					sprintf(temp, "DIFFICULTY MULTIPLIER: x%i", (int)3);
					vita2d_pvf_draw_text(font, getCenteredTextX(font, temp, 1.2f), 190, fg, 1.2f, temp);
					multi = 3;
				}
			}
			if (frameCounter > 120 || showFinalScore == true) {
				showFinalScore = true;
				if ((snakeLength - 4) * multi < 10) {
					sprintf(temp, "FINAL SCORE: 00%i0", (snakeLength - 4) * multi);
					vita2d_pvf_draw_text(font, getCenteredTextX(font, temp, 1.2f), 240, fg, 1.2f, temp);
				}
				else {
					sprintf(temp, "FINAL SCORE: 0%i0", (snakeLength - 4) * multi);
					vita2d_pvf_draw_text(font, getCenteredTextX(font, temp, 1.2f), 240, fg, 1.2f, temp);
				}
				if ((snakeLength - 4) * multi > highScore) {
					vita2d_pvf_draw_text(font, getCenteredTextX(font, "NEW HIGH SCORE", 1.2f), screenHeight - 150, fg, 1.2f, "NEW HIGH SCORE");
				}
			}
			if (frameCounter > 180 || showPostHint == true) {
				frameCounter = 0;
				showPostHint = true;
				vita2d_pvf_draw_text(font, getCenteredTextX(font, "PRESS X TO RETURN TO MENU", 1.5f), screenHeight - 75, fg, 1.5f, "PRESS X TO RETURN TO MENU");
			}
			break;
		case STATUS_IN_SETTINGS:
			vita2d_pvf_draw_text(font, getCenteredTextX(font, "SETTINGS", 1.8f), 25 + 50, fg, 1.8f, "SETTINGS");
			if (listCounter == 0) {
				sprintf(temp, "<DIFFICULTY: %s>", diffs[difficulty]);
				vita2d_pvf_draw_text(font, getCenteredTextX(font, temp, 1.5f), screenHeight / 2 - 75 + 18, fg, 1.5f, temp);
				sprintf(temp, "THEME: %s", themeNames[theme]);
				vita2d_pvf_draw_text(font, getCenteredTextX(font, temp, 1.5f), screenHeight / 2 + 25 + 18, fg, 1.5f, temp);
				vita2d_pvf_draw_text(font, getCenteredTextX(font, "DONE", 1.5f), screenHeight - 75 + 18, fg, 1.5f, "DONE");
			}
			else if (listCounter == 1) {
				sprintf(temp, "DIFFICULTY: %s", diffs[difficulty]);
				vita2d_pvf_draw_text(font, getCenteredTextX(font, temp, 1.5f), screenHeight / 2 - 75 + 18, fg, 1.5f, temp);
				sprintf(temp, "<THEME: %s>", themeNames[theme]);
				vita2d_pvf_draw_text(font, getCenteredTextX(font, temp, 1.5f), screenHeight / 2 + 25 + 18, fg, 1.5f, temp);
				vita2d_pvf_draw_text(font, getCenteredTextX(font, "DONE", 1.5f), screenHeight - 75 + 18, fg, 1.5f, "DONE");
			}
			else {
				sprintf(temp, "DIFFICULTY: %s", diffs[difficulty]);
				vita2d_pvf_draw_text(font, getCenteredTextX(font, temp, 1.5f), screenHeight / 2 - 75 + 18, fg, 1.5f, temp);
				sprintf(temp, "THEME: %s", themeNames[theme]);
				vita2d_pvf_draw_text(font, getCenteredTextX(font, temp, 1.5f), screenHeight / 2 + 25 + 18, fg, 1.5f, temp);
				vita2d_pvf_draw_text(font, getCenteredTextX(font, "-DONE-", 1.5f), screenHeight - 75 + 18, fg, 1.5f, "-DONE-");
			}
			break;
		}

        vita2d_end_drawing();
        vita2d_wait_rendering_done();
		vita2d_end_shfb();
        //----------------------------------------------------------------------------------
    }

    //--------------------------------------------------------------------------------------
}