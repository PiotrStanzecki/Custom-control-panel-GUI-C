#include "raylib.h"
#include "gui.h"


scene_t currentScene = LOGIN;
bool isLoggedIn = false;
bool usernameActive = false;
bool passwordActive = false;
TextBox usernameInput;
TextBox passwordInput;
uint64_t counter = 0;
int Timer = 0;
int alertTimer = 0;
int fanSpeed;
int lineSpeed = 0;
char* logEntries[10] = { "System started", "User logged in"};
Texture2D engineTexture;
button_t start;
button_t stop;

int main() {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Control Panel");
    SetTargetFPS(60);

    initGUI();

    Image tempImage = LoadImage("cos.png"); 
    engineTexture = LoadTextureFromImage(tempImage); 
    UnloadImage(tempImage); 

    while (!WindowShouldClose()) {
        BeginDrawing();
            ClearBackground(DARKGRAY);
            drawMainScreen();
            counter++;
        EndDrawing();
    }
    UnloadTexture(engineTexture); 
    CloseWindow();
    return 0;
}