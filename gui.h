#ifndef GUI_H
#define GUI_H

#include "raylib.h"
#include "raymath.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <time.h>
#include <stdio.h>

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
    char message[128];
    Color color;
} LogEntry;

typedef struct {
    char label[20];
    Rectangle rect;
    Color color;
} button_t;


extern scene_t currentScene;
extern bool isLoggedIn;
extern bool usernameActive;
extern bool passwordActive;
extern TextBox usernameInput;
extern TextBox passwordInput;

extern float loginTimer; 
extern float alertTimer;
extern float alertCountdown;

extern LogEntry logEntries[10];
extern int logCount;
extern Texture2D engineTexture;

extern button_t startBtn;
extern button_t stopBtn;
extern button_t fanUpBtn;   
extern button_t fanDownBtn; 
extern button_t lineUpBtn;   
extern button_t lineDownBtn; 

extern int tries;
extern int currentTries;
extern bool started;
extern float stopTimer;
extern uint64_t counter;


extern float motorTemp;
extern float cpuTemp;
extern int cpuUsage;
extern int ramUsage;
extern int fanSpeed;
extern int lineSpeed;
extern int baseLineSpeed;    
extern float simUpdateTimer;



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

// Deklaracje funkcji
void initGUI();
void addLog(const char* msg, Color color);
void simulateProcess();
int getRealCpuUsage();
int getRealRamUsage();
void drawTextBox(TextBox *box, Color color, bool active, bool password);
void drawLogs();
void drawLoginScreen();
void drawWorkingScreen();
void drawAlertScreen();
void drawMainScreen();

#endif
