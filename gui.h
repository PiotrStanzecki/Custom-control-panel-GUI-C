#ifndef GUI_H
#define GUI_H

#include "raylib.h"
#include "raymath.h"  
#include <stdlib.h>
#include <string.h> 
#include <stdint.h>
#include <math.h>
#include <time.h>

#define SCREEN_WIDTH            1200
#define SCREEN_HEIGHT           800
#define LOGIN_RECT_WIDTH        200
#define LOGIN_RECT_HEIGHT       40
#define LOGIN_SCREEN_BOX_COLOR  LIGHTGRAY

typedef enum { LOGIN, WORKING, ALERT, LOCKED } scene_t;

typedef struct {
    char text[20];
    Rectangle rect;
} TextBox;

typedef struct {
    char* message;
    Color color;
} LogEntry;

typedef struct {
    LogEntry entries[10];
    int count;
} LogSystem;

typedef struct{

    char label[20];
    Rectangle rect;
    Color color;
    

}button_t;


extern scene_t currentScene;
extern bool isLoggedIn;
extern bool usernameActive;
extern bool passwordActive;
extern TextBox usernameInput;
extern TextBox passwordInput;
extern uint64_t counter;
extern int Timer;
extern char* logEntries[10];
extern Texture2D engineTexture;
extern button_t start;
extern button_t stop;
extern int alertTimer;
extern int fanSpeed;
extern int lineSpeed;

int tries = 3;
int currentTries = 0;
bool started = false;
int stopTimer = 0;


static inline bool mouseInRect(Rectangle rect, Vector2 mousePos) {
    return CheckCollisionPointRec(mousePos, rect); 
}

static inline void initButton(button_t *button, const char* label, float x, float y, float width, float height, Color color) {
    strncpy(button->label, label, sizeof(button->label) - 1);
    button->label[sizeof(button->label) - 1] = '\0'; 
    button->rect = (Rectangle){ x, y, width, height };
    button->color = color;
}

static inline bool isButtonClicked(button_t *button) {
    Vector2 mouse = GetMousePosition();
    return IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && mouseInRect(button->rect, mouse);
}

static inline void drawButton(button_t *button) {
    DrawRectangleRec(button->rect, button->color);
    DrawText(button->label, (int)button->rect.x + 10, (int)button->rect.y + 10, 20, DARKGRAY);
}

static inline bool isHoveringButton(button_t *button) {
    Vector2 mouse = GetMousePosition();
    return (mouseInRect(button->rect, mouse));
}

static inline void initGUI() {
    usernameInput.rect = (Rectangle){ SCREEN_WIDTH/2 - LOGIN_RECT_WIDTH/2, SCREEN_HEIGHT/2 - 20, LOGIN_RECT_WIDTH, LOGIN_RECT_HEIGHT };
    passwordInput.rect = (Rectangle){ SCREEN_WIDTH/2 - LOGIN_RECT_WIDTH/2, SCREEN_HEIGHT/2 + 70, LOGIN_RECT_WIDTH, LOGIN_RECT_HEIGHT };
    usernameInput.text[0] = '\0';
    passwordInput.text[0] = '\0';
}

static inline void drawTextBox(TextBox *box, Color color, bool active, bool password) {
    DrawRectangleRec(box->rect, color);
    
    if (active) DrawRectangleLinesEx(box->rect, 2, WHITE);
    if (password) {
        char masked[20];
        memset(masked, '*', strlen(box->text));
        masked[strlen(box->text)] = '\0';
        DrawText(masked, (int)box->rect.x + 5, (int)box->rect.y + 5, 20, DARKGRAY);
    } else {
        DrawText(box->text, (int)box->rect.x + 5, (int)box->rect.y + 5, 20, DARKGRAY);
    }
}


static inline void drawLogs() {
    DrawRectangle(10, SCREEN_HEIGHT/2 + 100, SCREEN_WIDTH/2 + 10, SCREEN_HEIGHT/2 - 110, WHITE);
    for (int i = 0; i < 10; i++) {
        if (logEntries[i]) {
            DrawText(logEntries[i], 20, SCREEN_HEIGHT/2 + 110 + i * 20, 20, BLACK);
        }
    }
}

static inline void drawLoginScreen() {
    Vector2 mouse = GetMousePosition();
    
    DrawText("LOGIN", SCREEN_WIDTH/2 - LOGIN_RECT_WIDTH/2, SCREEN_HEIGHT/3 - 40, 80, LIGHTGRAY);

    DrawText("Username:", (int)usernameInput.rect.x, (int)usernameInput.rect.y - 30, 20, LIGHTGRAY);
    DrawText("Password:", (int)passwordInput.rect.x, (int)passwordInput.rect.y - 30, 20, LIGHTGRAY);

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        usernameActive = mouseInRect(usernameInput.rect, mouse);
        passwordActive = mouseInRect(passwordInput.rect, mouse);
    }

    
    TextBox *activeBox = NULL;
    if (usernameActive) activeBox = &usernameInput;
    else if (passwordActive) activeBox = &passwordInput;

    if (activeBox) {
        int key = GetCharPressed(); 
        while (key > 0) {
            int len = strlen(activeBox->text);
            if (len < sizeof(activeBox->text) - 1) {
                activeBox->text[len] = (char)key;
                activeBox->text[len + 1] = '\0';
            }
            key = GetCharPressed();
        }
        
        // Handle Backspace
        if (IsKeyPressed(KEY_BACKSPACE)) {
            int len = strlen(activeBox->text);
            if (len > 0) activeBox->text[len - 1] = '\0';
        }
    }

    drawTextBox(&usernameInput, LOGIN_SCREEN_BOX_COLOR, usernameActive, false);
    drawTextBox(&passwordInput, LOGIN_SCREEN_BOX_COLOR, passwordActive, true);

    if(IsKeyPressed(KEY_ENTER)) {
        if (strcmp(usernameInput.text, "admin") == 0 && strcmp(passwordInput.text, "admin") == 0) {
            Timer = 90;
            alertTimer = 300;
            currentScene = WORKING;
        } else {
            currentTries++;
            strcpy(usernameInput.text, "WRONG!"); 
            passwordInput.text[0] = '\0';
            Timer = 120; // Set timer for 2 seconds
            if (currentTries >= tries) {
                currentScene = LOCKED; 
            }
        }
    }

    if (Timer > 0) {
        Timer--;
        DrawText("ACCESS DENIED", SCREEN_WIDTH/2 - 100, SCREEN_HEIGHT/2 + 120, 20, RED);
        
        if (Timer == 0) {
            usernameInput.text[0] = '\0'; 
        }
    }

    drawTextBox(&usernameInput, LOGIN_SCREEN_BOX_COLOR, usernameActive, false);
    drawTextBox(&passwordInput, LOGIN_SCREEN_BOX_COLOR, passwordActive, true);
}


void drawWorkingScreen() {
    if (Timer > 0) {
        DrawText("ACCESS GRANTED! WELCOME!", SCREEN_WIDTH/2 - 150, SCREEN_HEIGHT/2, 20, GREEN);
        Timer--;
    } else {

        if(alertTimer > 0){

        
        
        
            DrawText("Control Panel", SCREEN_WIDTH/2 - 200, 20, 60, LIGHTGRAY);

            initButton(&start, "START", SCREEN_WIDTH/2 + 30, SCREEN_HEIGHT/2 + 100, 100, 50, GREEN);
            drawButton(&start);

            initButton(&stop, "STOP", SCREEN_WIDTH/2 + 150, SCREEN_HEIGHT/2 + 100, 100, 50, RED);
            drawButton(&stop);

            if(isHoveringButton(&start)) {
                DrawRectangleLinesEx(start.rect, 3, WHITE);
            }
            if(isHoveringButton(&stop)) {
                DrawRectangleLinesEx(stop.rect, 3, WHITE);
            }


            if (engineTexture.id > 0) {
                
                Rectangle sourceRec = { 0.0f, 0.0f, (float)engineTexture.width, (float)engineTexture.height };

                
                float displayWidth = 150.0f;
                float displayHeight = 120.0f;

                
                Rectangle destRec = { 
                    160.0f, 
                    (float)SCREEN_HEIGHT / 3.0f , 
                    displayWidth, 
                    displayHeight 
                };

                
                Vector2 origin = { displayWidth / 2.0f, displayHeight / 2.0f };
                Color pulseColor = (Color){ 255, 255, 255, (uint8_t)(128 + 127 * sin(counter * 0.1f)) }; // Smooth pulsing effect


                if(isButtonClicked(&start) || started) {
                    DrawTexturePro(engineTexture, sourceRec, destRec, origin, 0.0f, GREEN);
                    started = true;
                    
                } else if(stopTimer > 0) {
                    DrawTexturePro(engineTexture, sourceRec, destRec, origin, 0.0f, RED);
                    stopTimer--;
                }
                else {
                    DrawTexturePro(engineTexture, sourceRec, destRec, origin, 0.0f, pulseColor);
                }

                if(isButtonClicked(&stop)) {
                    started = false;
                    stopTimer = 300;
                }
            
            }

            drawLogs();
            alertTimer--;
        }else {
            currentScene = ALERT;
        }

    }
}


void drawAlertScreen() {
    ClearBackground(RED);
    DrawText("Checking Activity", SCREEN_WIDTH/2 - 200, SCREEN_HEIGHT/2, 20, WHITE);
    if(mouseInRect((Rectangle){0, 0, SCREEN_WIDTH, SCREEN_HEIGHT}, GetMousePosition())) {
        if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            srand(time(NULL));

            alertTimer = 300 + (rand())%500; 
            currentScene = WORKING; 
        }
    }
}


static inline void drawMainScreen() {
    switch(currentScene) {
        case LOGIN:   
            drawLoginScreen();   
            break;
        case WORKING:
            drawWorkingScreen();
            break;
        case ALERT:   
            drawAlertScreen();
            break;
        case LOCKED:    
            ClearBackground(RED);
            DrawText("TOO MANY ATTEMPTS! LOCKED OUT!", SCREEN_WIDTH/2 - 200, SCREEN_HEIGHT/2, 20, WHITE);  
            break;
    }
}

#endif