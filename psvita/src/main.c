/*******************************************************************************************
*
*   Snake by JJH47E
*
*   This game has been created using vitasdk (https://vitasdk.org/)
*
********************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

#include <psp2/ctrl.h>
#include <psp2/kernel/processmgr.h>
#include <psp2/types.h>

#include <vita2d.h>

//#ifndef _PSP2_KERNEL_RNG_H_
//#define _PSP2_KERNEL_RNG_H_
//#endif

const int screenWidth = 960;
const int screenHeight = 544;
SceFVector2 snake[300];

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

int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------

    SceCtrlData pad;
    srand(time(NULL));
    
    vita2d_texture *logo_gb;
    vita2d_texture *logo_matrix;
    vita2d_texture *logo_oled;
    vita2d_texture *logo;
    
    vita2d_font *font;

    vita2d_init();
    vita2d_set_clear_color(RGBA8(0, 0, 0, 255));

    memset(&pad, 0, sizeof(pad));

    int snakeLength = 4;
    int headX = 10;
    int headY = 5;
    int gridX = 23;
    int gridY = 12;
    int xVal;
    int yVal;
    int counter;
    
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
    
    bool inMenu = true;
    bool inGame = false;
    bool PostGame = false;
    bool inSettings = false;
    
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
    
    //char debug[50];
    //sprintf(debug, "");
    bool newFood = true;
    
    bool gotInput = false;

    bool firstTimeBool = false;
    
    int listCounter = 0;
    
    unsigned int bg;
    unsigned int fg;
    
    char diffs[3][15] = {"EASY", "HARD", "IMPOSSIBLE"};
    char themeNames[3][15] = {"OLED", "MATRIX", "GAMEBOY"};
    char temp[250];
    
    logo_gb = vita2d_load_PNG_file("app0:src/resources/logo_gb.png");
    logo_matrix = vita2d_load_PNG_file("app0:src/resources/logo_matrix.png");
    logo_oled = vita2d_load_PNG_file("app0:src/resources/logo_oled.png");
    
    //Sound eatFood = LoadSound("resources/food.wav");
    //Sound selectSound = LoadSound("resources/select.wav");
    //Sound deathSound = LoadSound("resources/death.wav");
    
    font = vita2d_load_font_file("app0:src/resources/font.otf");

    // FILE *fileptr = fopen("src/resources/save.txt", "r+");
    // if(fileptr == NULL){
    //     highScore = 0;
    //     theme = 0;
    //     difficulty = 1;
    //     bool save_data = false;
    // }
    // else{
    //     fscanf(fileptr, "theme==%i", &theme);
    //     fscanf(fileptr, "dif==%i", &difficulty);
    //     fscanf(fileptr, "hiscore==%i", &highScore);
    //     bool save_data = true;
    // }
    
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
        if(inGame == true){
            if (pad.buttons & SCE_CTRL_RIGHT){
                if(last_l == false){
                    gotInput = true;
                    right = true;
                    left = false;
                    up = false;
                    down = false;
                }
            }
            else if (pad.buttons & SCE_CTRL_LEFT) {
                if(last_r == false){
                    gotInput = true;
                    left = true;
                    right = false;
                    up = false;
                    down = false;
                }
            }
            else if (pad.buttons & SCE_CTRL_UP) {
                if(last_d == false){
                    gotInput = true;
                    up = true;
                    right = false;
                    left = false;
                    down = false;
                }
            }
            else if (pad.buttons & SCE_CTRL_DOWN) {
                if(last_u == false){
                    gotInput = true;
                    down = true;
                    right = false;
                    left = false;
                    up = false;
                }
            }
            if(newFood == true){
                newFood = false;
                //sprintf(debug, "food");
                while(1){
                    counter = 0;
                    yVal = rand() % (gridY-1);
                    xVal = rand() & (gridX-1);
                    for(int i = 0; i < snakeLength; i++){
                        if((float) snake[i].x == (float) xVal && (float) snake[i].y == (float) yVal){
                            counter = 1;
                            break;
                        }
                    }
                    if(counter == 0){
                        break;
                    }
                }
                SceFVector2 foodTemp = {(float) xVal, (float) yVal};
                food = foodTemp;
                //sprintf(debug, "%f, %f", food.x, food.y);
            }
            if(frameCounter > frames){
                frameCounter = 0;
                for(int i = 0; i < snakeLength - 1; i++){
                    if(snake[i].x == newHead.x && snake[i].y == newHead.y){
                        endGame = true;
                        break;
                    }
                }
                if(right){
                    headX++;
                    last_r = true;
                    last_l = false;
                    last_d = false;
                    last_u = false;
                }
                else if(left){
                    headX--;
                    last_l = true;
                    last_d = false;
                    last_u = false;
                    last_r = false;
                }
                else if(down){
                    headY++;
                    last_d = true;
                    last_u = false;
                    last_r = false;
                    last_l = false;
                }
                else if(up){
                    headY--;
                    last_u = true;
                    last_r = false;
                    last_l = false;
                    last_d = false;
                }
                if(headX >= gridX){
                    headX = 0;
                }
                else if(headX < 0){
                    headX = gridX - 1;
                }
                if(headY >= gridY){
                    headY = 0;
                }
                else if(headY < 0){
                    headY = gridY - 1;
                }
                if((newHead.x == food.x) && (newHead.y == food.y)){
                   snakeLength++;
                   //PlaySound(eatFood);
                   newFood = true;
                   snake[snakeLength - 1] = food;
                }
                SceFVector2 headTemp = {(float) headX, (float) headY};
                newHead = headTemp;
                shiftSnake(newHead, snakeLength);
            }
            if(endGame == true){
                //PlaySound(deathSound);
                inGame = false;
                PostGame = true;
                frameCounter = 0;
            }
            //sprintf(debug, "newHead.x: %f, food.x: %f, newHead.y: %f, food.y: %f, last_snake.x: %f", newHead.x, food.x, newHead.y, food.y);
        }
        else if(inMenu == true){
            if ((pad.buttons & SCE_CTRL_CROSS) && (firstTimeBool == true)){
                firstTimeBool = false;
                //PlaySound(selectSound);
                if(listCounter == 0){
                    inMenu = false;
                    inGame = true;
                    snakeLength = 4;
                    headX = 10;
                    headY = 5;
                    gridX = 23;
                    gridY = 12;
                    
                    SceFVector2 start_2 = {(float) headX, (float) headY + 1};
                    SceFVector2 start_3 = {(float) headX, (float) headY + 2};
                    SceFVector2 start_4 = {(float) headX, (float) headY + 3};
                    
                    SceFVector2 newHead = {(float) headX, (float) headY};
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
                else{
                    inMenu = false;
                    inSettings = true;
                    listCounter = 0;
                }
            }
            else if((pad.buttons & SCE_CTRL_UP) && (firstTimeBool == true)){
                firstTimeBool = false;
                //PlaySound(selectSound);
                listCounter--;
                listCounter = abs(listCounter % 2);
            }
            else if((pad.buttons & SCE_CTRL_DOWN) && (firstTimeBool == true)){
                firstTimeBool = false;
                //PlaySound(selectSound);
                listCounter++;
                listCounter = abs(listCounter % 2);
            }
            else if((!(pad.buttons & SCE_CTRL_CROSS)) && (!(pad.buttons & SCE_CTRL_DOWN)) && (!(pad.buttons & SCE_CTRL_UP))){
                firstTimeBool = true;
            }
        }
        else if(inSettings == true){
            if((pad.buttons & SCE_CTRL_DOWN) && (firstTimeBool == true)){
                firstTimeBool = false;
                //PlaySound(selectSound);
                listCounter++;
                listCounter = abs(listCounter % 3);
            }
            else if((pad.buttons & SCE_CTRL_UP) && (firstTimeBool == true)){
                firstTimeBool = false;
                //PlaySound(selectSound);
                listCounter--;
                listCounter = abs(listCounter % 3);
            }
            else if((pad.buttons & SCE_CTRL_RIGHT) && (firstTimeBool == true)){
                firstTimeBool = false;
                //PlaySound(selectSound);
                if(listCounter == 0){
                    difficulty++;
                    difficulty = abs(difficulty % 3);
                }
                else if(listCounter == 1){
                    theme++;
                    theme = abs(theme % 3);
                    // if(save_data){
                    //     fprintf(fileptr, "theme==%i", theme);
                    // }
                }
            }
            else if((pad.buttons & SCE_CTRL_LEFT) && (firstTimeBool == true)){
                firstTimeBool = false;
                //PlaySound(selectSound);
                if(listCounter == 0){
                    difficulty--;
                    difficulty = abs(difficulty % 3);
                }
                else if(listCounter == 1){
                    theme--;
                    theme = abs(theme % 3);
                    // if(save_data){
                    //     fprintf(fileptr, "theme==%i", theme);
                    // }
                }
            }
            else if((pad.buttons & SCE_CTRL_CROSS) && (firstTimeBool == true)){
                firstTimeBool = false;
                //PlaySound(selectSound);
                if(listCounter == 2){
                    inSettings = false;
                    inMenu = true;
                    listCounter = 0;
                }
            }
            else if((!(pad.buttons & SCE_CTRL_CROSS)) && (!(pad.buttons & SCE_CTRL_LEFT)) && (!(pad.buttons & SCE_CTRL_RIGHT)) && (!(pad.buttons & SCE_CTRL_DOWN)) && (!(pad.buttons & SCE_CTRL_UP))){
                firstTimeBool = true;
            }
        }
        else if(PostGame == true){
            if((pad.buttons & SCE_CTRL_CROSS) && (firstTimeBool == true) && (showPostHint == true)){
                firstTimeBool = false;
                //PlaySound(selectSound);
                if((snakeLength - 4) * multi > highScore){
                    highScore = (snakeLength - 4) * multi;
                    // if(save_data){
                    //     fprintf(fileptr, "hiscore==%i", highScore);
                    // }
                }
                showScore = false;
                showMulti = false;
                showFinalScore = false;
                showPostHint = false;
                frameCounter = 0;
                PostGame = false;
                inMenu = true;
            }
            else if((!(pad.buttons & SCE_CTRL_CROSS))){
                firstTimeBool = true;
            }
        }
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        vita2d_start_drawing();		
            vita2d_clear_screen();
            if(inGame == true){
                if(snakeLength - 4 < 10){
                    sprintf(temp, "00%i0", snakeLength - 4);
                    vita2d_font_draw_text(font, 10, 25, fg, 30, temp);
                }
                else{
                    sprintf(temp, "0%i0", snakeLength - 4);
                    vita2d_font_draw_text(font, 10, 25, fg, 30, temp);
                }
                drawFoodBit(food, bg, fg);
                for(int i = snakeLength - 1; i >= 0; i--){
                    drawSnakeBit(snake[i], bg, fg);
                }
                drawGrid(20, 16, 23, 12, 30, bg, fg);
            }
            else if(inMenu == true){
                vita2d_font_draw_text(font, 10, 20, fg, 20, "Made by JJH47E");
                vita2d_draw_texture(logo, screenWidth/2 - 231, 75 - 30);
                sprintf(temp, "HIGH SCORE: %i0", highScore);
                vita2d_font_draw_text(font, screenWidth/2 - 137, screenHeight - 75, fg, 32, temp);
                if(listCounter == 0){
                    vita2d_font_draw_text(font, screenWidth/2 - 84, 144 + 150, fg, 48, "-PLAY-");
                    vita2d_font_draw_text(font, screenWidth/2 - 126, 225 + 150, fg, 48, "SETTINGS");
                }
                else{
                    vita2d_font_draw_text(font, screenWidth/2 - 61, 144 + 150, fg, 48, "PLAY");
                    vita2d_font_draw_text(font, screenWidth/2 - 150, 225 + 150, fg, 48, "-SETTINGS-");
                }
            }
            else if(PostGame == true){
                if(frameCounter > 30 || showScore == true){
                    showScore = true;
                    if(snakeLength - 4 < 10){
                        sprintf(temp, "SCORE: 00%i0", snakeLength - 4);
                        vita2d_font_draw_text(font, screenWidth/2 - 91 - 20, 150, fg, 30, temp);
                    }
                    else{
                        sprintf(temp, "SCORE: 0%i0", snakeLength - 4);
                        vita2d_font_draw_text(font, screenWidth/2 - 91 - 20, 150, fg, 30, temp);
                    }
                }
                if(frameCounter > 60 || showMulti == true){
                    showMulti = true;
                    if(difficulty == 1){
                        sprintf(temp, "DIFFICULTY MULTIPLIER: x%i", (int)2);
                        vita2d_font_draw_text(font, screenWidth/2 - 223 - 20, 190, fg, 30, temp);
                        multi = 2;
                    }
                    else if(difficulty == 0){
                        sprintf(temp, "DIFFICULTY MULTIPLIER: x%i", (int)1);
                        vita2d_font_draw_text(font, screenWidth/2 - 223 - 20, 190, fg, 30, temp);
                        multi = 1;
                    }
                    else {
                        sprintf(temp, "DIFFICULTY MULTIPLIER: x%i", (int)3);
                        vita2d_font_draw_text(font, screenWidth/2 - 223 - 20, 190, fg, 30, temp);
                        multi = 3;
                    }
                }
                if(frameCounter > 120 || showFinalScore == true){
                    showFinalScore = true;
                    if((snakeLength - 4) * multi < 10){
                        sprintf(temp, "FINAL SCORE: 00%i0", (snakeLength - 4) * multi);
                        vita2d_font_draw_text(font, screenWidth/2 - 148 - 20, 240, fg, 30, temp);
                    }
                    else{
                        sprintf(temp, "FINAL SCORE: 0%i0", (snakeLength - 4) * multi);
                        vita2d_font_draw_text(font, screenWidth/2 - 153 - 20, 240, fg, 30, temp);
                    }
                    if((snakeLength - 4) * multi > highScore){
                        vita2d_font_draw_text(font, screenWidth/2 - 133 - 20, screenHeight - 150, fg, 30, "NEW HIGH SCORE");
                    }
                }
                if(frameCounter > 180 || showPostHint == true){
                    frameCounter = 0;
                    showPostHint = true;
                    vita2d_font_draw_text(font, screenWidth/2 - 286 - 50, screenHeight - 75, fg, 36, "PRESS X TO RETURN TO MENU");
                }
            }
            else if(inSettings == true){
                vita2d_font_draw_text(font, screenWidth/2 - 135, 25 + 50, fg, 50, "SETTINGS");
                if(listCounter == 0){
                    sprintf(temp, "<DIFFICULTY: %s>", diffs[difficulty]);
                    vita2d_font_draw_text(font, screenWidth/2 - 246, screenHeight/2 - 75 + 18, fg, 36, temp);
                    sprintf(temp, "THEME: %s", themeNames[theme]);
                    vita2d_font_draw_text(font, screenWidth/2 - 161, screenHeight/2 + 25 + 18, fg, 36, temp);
                    vita2d_font_draw_text(font, screenWidth/2 - 47, screenHeight - 75 + 18, fg, 36, "DONE");
                }
                else if(listCounter == 1){
                    sprintf(temp, "DIFFICULTY: %s", diffs[difficulty]);
                    vita2d_font_draw_text(font, screenWidth/2 - 233, screenHeight/2 - 75 + 18, fg, 36, temp);
                    sprintf(temp, "<THEME: %s>", themeNames[theme]);
                    vita2d_font_draw_text(font, screenWidth/2 - 175, screenHeight/2 + 25 + 18, fg, 36, temp);
                    vita2d_font_draw_text(font, screenWidth/2 - 47, screenHeight - 75 + 18, fg, 36, "DONE");
                }
                else{
                    sprintf(temp, "DIFFICULTY: %s", diffs[difficulty]);
                    vita2d_font_draw_text(font, screenWidth/2 - 233, screenHeight/2 - 75 + 18, fg, 36, temp);
                    sprintf(temp, "THEME: %s", themeNames[theme]);
                    vita2d_font_draw_text(font, screenWidth/2 - 161, screenHeight/2 + 25 + 18, fg, 36, temp);
                    vita2d_font_draw_text(font, screenWidth/2 - 65, screenHeight - 75 + 18, fg, 36, "-DONE-");
                }
            }

        vita2d_end_drawing();
        vita2d_swap_buffers();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    vita2d_fini();
    vita2d_free_texture(logo_gb);
    vita2d_free_texture(logo_oled);
    vita2d_free_texture(logo_matrix);
    vita2d_free_texture(logo);
    vita2d_free_font(font);
    
    //UnloadSound(eatFood);
    //UnloadSound(selectSound);
    //UnloadSound(deathSound);
    // Close window and OpenGL context
    sceKernelExitProcess(0);
    return 0;
    //--------------------------------------------------------------------------------------
}