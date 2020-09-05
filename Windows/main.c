#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "raylib.h"

//TO FIX BUG, SNAKELENGTH++ HAPPENS BEFORE THE GAME KNOWS WHERE TO DRAW NEW SNAKE BIT

const int screenWidth = 960;
const int screenHeight = 544;
Vector2 snake[250];

void drawGrid(int offsetX, int offsetY, int x, int y, int upperOffset, Color bg, Color fg){
    //draw vertical lines
    for(int i = 0; i < x + 1; i++){
        Vector2 startV = {offsetX + (i * (screenWidth - 2 * offsetX) / x), offsetY + upperOffset};
        Vector2 endV = {offsetX + (i * (screenWidth - 2 * offsetX) / x), screenHeight - offsetY};
        DrawLineEx(startV, endV, (float)2, bg);
    }
    //draw horizontal lines
    for(int i = 0; i < y + 1 + 1; i++){
        Vector2 startV = {offsetX, offsetY + upperOffset + (i * (screenHeight - (2 * offsetY + upperOffset)) / y)};
        Vector2 endV = {screenWidth - offsetX, offsetY + upperOffset + (i * (screenHeight - (2 * offsetY + upperOffset)) / y)};
        DrawLineEx(startV, endV, (float)2, bg);
    }
}

void drawSnakeBit(Vector2 coords, Color bg, Color fg){
    Vector2 position = {(coords.x * 40) + 20, (coords.y * 40) + 16 + 30};
    Vector2 size = {40, 40};
    DrawRectangleV(position, size, fg);
}

void drawFoodBit(Vector2 coords, Color bg, Color fg){
    Vector2 position = {(((int) coords.x) * 40) + 20 + 10, (((int) coords.y) * 40) + 16 + 30 + 10};
    Vector2 size = {20, 20};
    DrawRectangleV(position, size, fg);
}

void shiftSnake(Vector2 newVal, int length){
    for(int i = 0; i < length - 1; i++){
        snake[i] = snake[i + 1];
    }
    snake[length-1] = newVal;
}

int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 960;
    const int screenHeight = 544;
    
    //Vector2 snakeTest = {5, 5};
    //Vector2 snake[100];
    //snake[0] = snakeTest;
    
    int snakeLength = 4;
    int snakeCounter = 0;
    int headX = 10;
    int headY = 5;
    int gridX = 23;
    int gridY = 12;
    int xVal;
    int yVal;
    int counter;
    
    Vector2 start_2 = {(float) headX, (float) headY + 1};
    Vector2 start_3 = {(float) headX, (float) headY + 2};
    Vector2 start_4 = {(float) headX, (float) headY + 3};
    
    Vector2 newHead = {(float) headX, (float) headY};
    Vector2 food;
    Vector2 tempSnake;
    
    snake[0] = start_4;
    snake[1] = start_3;
    snake[2] = start_2;
    snake[3] = newHead;
    
    bool right = false;
    bool left = false;
    bool up = true;
    bool down = false;
    bool endGame = false;
    bool addSnake = false;
    
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
    
    int highScore = LoadStorageValue(0);
    int theme = LoadStorageValue(1);
    int difficulty = 1;
    
    char debug[50];
    sprintf(debug, "");
    bool newFood = true;
    
    bool gotInput = false;
    
    int listCounter = 0;
    int difficultyCounter;
    
    Color bg;
    Color fg;
    
    char diffs[3][15] = {"EASY", "HARD", "IMPOSSIBLE"};
    char themeNames[3][15] = {"OLED", "MATRIX", "GAMEBOY"};

    InitWindow(screenWidth, screenHeight, "Snake");
    
    Texture2D logo_gb = LoadTexture("resources/logo_gb.png");
    Texture2D logo_matrix = LoadTexture("resources/logo_matrix.png");
    Texture2D logo_oled = LoadTexture("resources/logo_oled.png");
    Texture2D logo;
    
    InitAudioDevice();
    SetMasterVolume(1.0f);
    
    Sound eatFood = LoadSound("resources/food.wav");
    Sound selectSound = LoadSound("resources/select.wav");
    Sound deathSound = LoadSound("resources/death.wav");
    
    Font font = LoadFont("resources/font.otf");

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        if(theme == 0){
            //theme oled
            bg = (Color){0, 0, 0, 255};
            fg = (Color){255, 255, 255, 255};
            logo = logo_oled;
        }
        else if(theme == 1){
            //matrix theme
            bg = (Color){0, 0, 0, 255};
            fg = (Color){76, 220, 0, 255};
            logo = logo_matrix;
        }
        else {
            theme = 2;
            bg = (Color){202, 220, 160, 255};
            fg = (Color){138, 172, 15, 255};
            logo = logo_gb;
            //theme gb
        }
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
            if (IsKeyPressed(KEY_RIGHT)){
                if(last_l == false){
                    gotInput = true;
                    right = true;
                    left = false;
                    up = false;
                    down = false;
                }
            }
            else if (IsKeyPressed(KEY_LEFT)) {
                if(last_r == false){
                    gotInput = true;
                    left = true;
                    right = false;
                    up = false;
                    down = false;
                }
            }
            else if (IsKeyPressed(KEY_UP)) {
                if(last_d == false){
                    gotInput = true;
                    up = true;
                    right = false;
                    left = false;
                    down = false;
                }
            }
            else if (IsKeyPressed(KEY_DOWN)) {
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
                sprintf(debug, "food");
                while(1){
                    counter = 0;
                    yVal = GetRandomValue(0, 9);
                    xVal = GetRandomValue(0, 19);
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
                Vector2 foodTemp = {(float) xVal, (float) yVal};
                food = foodTemp;
                sprintf(debug, "%f, %f", food.x, food.y);
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
                if(newHead.x == food.x & newHead.y == food.y){
                   snakeLength++;
                   PlaySound(eatFood);
                   newFood = true;
                   snake[snakeLength - 1] = food;
                }
                Vector2 headTemp = {(float) headX, (float) headY};
                newHead = headTemp;
                shiftSnake(newHead, snakeLength);
            }
            if(endGame == true){
                PlaySound(deathSound);
                inGame = false;
                PostGame = true;
                frameCounter = 0;
            }
            //sprintf(debug, "newHead.x: %f, food.x: %f, newHead.y: %f, food.y: %f, last_snake.x: %f", newHead.x, food.x, newHead.y, food.y);
        }
        else if(inMenu == true){
            if (IsKeyPressed(KEY_SPACE)){
                PlaySound(selectSound);
                if(listCounter == 0){
                    inMenu = false;
                    inGame = true;
                    snakeLength = 4;
                    snakeCounter = 0;
                    headX = 10;
                    headY = 5;
                    gridX = 23;
                    gridY = 12;
                    
                    Vector2 start_2 = {(float) headX, (float) headY + 1};
                    Vector2 start_3 = {(float) headX, (float) headY + 2};
                    Vector2 start_4 = {(float) headX, (float) headY + 3};
                    
                    Vector2 newHead = {(float) headX, (float) headY};
                    Vector2 food;
                    Vector2 tempSnake;
                    
                    snake[0] = start_4;
                    snake[1] = start_3;
                    snake[2] = start_2;
                    snake[3] = newHead;
                    
                    right = false;
                    left = false;
                    up = true;
                    down = false;
                    endGame = false;
                    addSnake = false;                
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
            else if(IsKeyPressed(KEY_UP)){
                PlaySound(selectSound);
                listCounter--;
                listCounter = abs(listCounter % 2);
            }
            else if(IsKeyPressed(KEY_DOWN)){
                PlaySound(selectSound);
                listCounter++;
                listCounter = abs(listCounter % 2);
            }
        }
        else if(inSettings == true){
            if(IsKeyPressed(KEY_DOWN)){
                PlaySound(selectSound);
                listCounter++;
                listCounter = abs(listCounter % 3);
            }
            else if(IsKeyPressed(KEY_UP)){
                PlaySound(selectSound);
                listCounter--;
                listCounter = abs(listCounter % 3);
            }
            else if(IsKeyPressed(KEY_RIGHT)){
                PlaySound(selectSound);
                if(listCounter == 0){
                    difficulty++;
                    difficulty = abs(difficulty % 3);
                }
                else if(listCounter == 1){
                    theme++;
                    theme = abs(theme % 3);
                    SaveStorageValue(1, theme);
                }
            }
            else if(IsKeyPressed(KEY_LEFT)){
                PlaySound(selectSound);
                if(listCounter == 0){
                    difficulty--;
                    difficulty = abs(difficulty % 3);
                }
                else if(listCounter == 1){
                    theme--;
                    theme = abs(theme % 3);
                    SaveStorageValue(1, theme);
                }
            }
            else if(IsKeyPressed(KEY_SPACE)){
                PlaySound(selectSound);
                if(listCounter == 2){
                    inSettings = false;
                    inMenu = true;
                    listCounter = 0;
                }
            }
        }
        else if(PostGame == true){
            if(IsKeyPressed(KEY_SPACE) && showPostHint){
                PlaySound(selectSound);
                if((snakeLength - 4) * multi > LoadStorageValue(0)){
                    highScore = (snakeLength - 4) * multi;
                    SaveStorageValue(0, (snakeLength - 4) * multi);
                }
                showScore = false;
                showMulti = false;
                showFinalScore = false;
                showPostHint = false;
                frameCounter = 0;
                PostGame = false;
                inMenu = true;
            }
        }
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();
        
            ClearBackground(bg);
            if(inGame == true){
                if(snakeLength - 4 < 10){
                    DrawTextEx(font, FormatText("00%i0", snakeLength - 4), (Vector2){20, 10}, 30, (float)0, fg);
                }
                else{
                    DrawTextEx(font, FormatText("0%i0", snakeLength - 4), (Vector2){20, 10}, 30, (float)0, fg);
                }
                drawFoodBit(food, bg, fg);
                for(int i = snakeLength - 1; i >= 0; i--){
                    drawSnakeBit(snake[i], bg, fg);
                }
                drawGrid(20, 16, 23, 12, 30, bg, fg);
            }
            else if(inMenu == true){
                DrawTexture(logo, screenWidth/2 - 231, 75 - 30, WHITE);
                DrawTextEx(font, FormatText("HIGH SCORE: %i0", highScore), (Vector2){screenWidth/2 - MeasureText("HIGH SCORE: 000", 32)/2, screenHeight - 75}, 32, (float)0, fg);
                if(listCounter == 0){
                    DrawTextEx(font, "-PLAY-", (Vector2){screenWidth/2 - MeasureText("-PLAY-", 48)/2, 144 + 150}, 48, (float)0, fg);
                    DrawTextEx(font, "SETTINGS", (Vector2){screenWidth/2 - MeasureText("SETTINGS", 48)/2, 225 + 150}, 48, (float)0, fg);
                }
                else{
                    DrawTextEx(font, "-SETTINGS-", (Vector2){screenWidth/2 - MeasureText("-SETTINGS-", 48)/2, 225 + 150}, 48, (float)0, fg);
                    DrawTextEx(font, "PLAY", (Vector2){screenWidth/2 - MeasureText("PLAY", 48)/2, 144 + 150}, 48, (float)0, fg);
                }
            }
            else if(PostGame == true){
                if(frameCounter > 30 || showScore == true){
                    showScore = true;
                    if(snakeLength - 4 < 10){
                        DrawTextEx(font, FormatText("SCORE: 00%i0", snakeLength - 4), (Vector2){screenWidth/2 - MeasureText(FormatText("SCORE: 00%i0", snakeLength - 4), 30)/2, 150}, 30, (float)0, fg);
                    }
                    else{
                        DrawTextEx(font, FormatText("SCORE: 0%i0", snakeLength - 4), (Vector2){screenWidth/2 - MeasureText(FormatText("SCORE: 0%i0", snakeLength - 4), 30)/2, 150}, 30, (float)0, fg);
                    }
                }
                if(frameCounter > 60 || showMulti == true){
                    showMulti = true;
                    if(difficulty == 1){
                        DrawTextEx(font, FormatText("DIFFICULTY MULTIPLIER: x%i", (int)2), (Vector2){screenWidth/2 - MeasureText(FormatText("DIFFICULTY MULTIPLIER: x%i", (int)2), 30)/2, 190}, 30, (float)0, fg);
                        multi = 2;
                    }
                    else if(difficulty == 0){
                        DrawTextEx(font, FormatText("DIFFICULTY MULTIPLIER: x%i", (int)1), (Vector2){screenWidth/2 - MeasureText(FormatText("DIFFICULTY MULTIPLIER: x%i", (int)1), 30)/2, 190}, 30, (float)0, fg);
                        multi = 1;
                    }
                    else {
                        DrawTextEx(font, FormatText("DIFFICULTY MULTIPLIER: x%i", (int)3), (Vector2){screenWidth/2 - MeasureText(FormatText("DIFFICULTY MULTIPLIER: x%i", (int)3), 30)/2, 190}, 30, (float)0, fg);
                        multi = 3;
                    }
                }
                if(frameCounter > 120 || showFinalScore == true){
                    showFinalScore = true;
                    if((snakeLength - 4) * multi < 10){
                        DrawTextEx(font, FormatText("FINAL SCORE: 00%i0", (snakeLength - 4) * multi), (Vector2){screenWidth/2 - MeasureText(FormatText("FINAL SCORE: 00%i0", (snakeLength - 4) * multi), 30)/2, 240}, 30, (float)0, fg);
                    }
                    else{
                        DrawTextEx(font, FormatText("FINAL SCORE: 0%i0", (snakeLength - 4) * multi), (Vector2){screenWidth/2 - MeasureText(FormatText("FINAL SCORE: 0%i0", (snakeLength - 4) * multi), 30)/2, 240}, 30, (float)0, fg);
                    }
                    if((snakeLength - 4) * multi > highScore){
                        DrawTextEx(font, "NEW HIGH SCORE", (Vector2){screenWidth/2 - MeasureText("NEW HIGH SCORE", 30)/2, screenHeight - 150}, 30, (float)0, fg);
                    }
                }
                if(frameCounter > 180 || showPostHint == true){
                    frameCounter = 0;
                    showPostHint = true;
                    DrawTextEx(font, "PRESS SPACE TO RETURN TO MENU", (Vector2){screenWidth/2 - MeasureText("PRESS SPACE TO RETURN TO MENU", 36)/2, screenHeight - 75}, 36, (float)1, fg);
                }
            }
            else if(inSettings == true){
                DrawTextEx(font, "SETTINGS", (Vector2){screenWidth/2 - MeasureText("SETTINGS", 50)/2, 25}, 50, (float)1, fg);
                if(listCounter == 0){
                    DrawTextEx(font, FormatText("<DIFFICULTY: %s>", diffs[difficulty]), (Vector2){screenWidth/2 - MeasureText(FormatText("<DIFFICULTY: %s>", diffs[difficulty]), 36)/2, screenHeight/2 - 75}, 36, (float)1, fg);
                    DrawTextEx(font, FormatText("THEME: %s", themeNames[theme]), (Vector2){screenWidth/2 - MeasureText(FormatText("THEME: %s", themeNames[theme]), 36)/2, screenHeight/2 + 25}, 36, (float)1, fg);
                    DrawTextEx(font, "DONE", (Vector2){screenWidth/2 - MeasureText("DONE", 36)/2, screenHeight - 75}, 36, (float)1, fg);
                }
                else if(listCounter == 1){
                    DrawTextEx(font, FormatText("DIFFICULTY: %s", diffs[difficulty]), (Vector2){screenWidth/2 - MeasureText(FormatText("DIFFICULTY: %s", diffs[difficulty]), 36)/2, screenHeight/2 - 75}, 36, (float)1, fg);
                    DrawTextEx(font, FormatText("<THEME: %s>", themeNames[theme]), (Vector2){screenWidth/2 - MeasureText(FormatText("<THEME: %s>", themeNames[theme]), 36)/2, screenHeight/2 + 25}, 36, (float)1, fg);
                    DrawTextEx(font, "DONE", (Vector2){screenWidth/2 - MeasureText("DONE", 36)/2, screenHeight - 75}, 36, (float)1, fg);
                }
                else{
                    DrawTextEx(font, FormatText("DIFFICULTY: %s", diffs[difficulty]), (Vector2){screenWidth/2 - MeasureText(FormatText("DIFFICULTY: %s", diffs[difficulty]), 36)/2, screenHeight/2 - 75}, 36, (float)1, fg);
                    DrawTextEx(font, FormatText("THEME: %s", themeNames[theme]), (Vector2){screenWidth/2 - MeasureText(FormatText("THEME: %s", themeNames[theme]), 36)/2, screenHeight/2 + 25}, 36, (float)1, fg);
                    DrawTextEx(font, "-DONE-", (Vector2){screenWidth/2 - MeasureText("-DONE-", 36)/2, screenHeight - 75}, 36, (float)1, fg);
                }
            }

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadTexture(logo_gb);
    UnloadTexture(logo_oled);
    UnloadTexture(logo_matrix);
    UnloadTexture(logo);
    UnloadFont(font);
    UnloadSound(eatFood);
    UnloadSound(selectSound);
    UnloadSound(deathSound);
    CloseAudioDevice();
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}