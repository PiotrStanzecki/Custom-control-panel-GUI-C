#include "gui.h"


#define Rectangle Win32Rectangle
#define CloseWindow Win32CloseWindow
#define ShowCursor Win32ShowCursor


#include <windows.h>


#undef Rectangle
#undef CloseWindow
#undef ShowCursor
#undef DrawText
#undef DrawTextEx
#undef LoadImage
#undef PlaySound
#undef near
#undef far


scene_t currentScene = LOGIN;
bool isLoggedIn = false;
bool usernameActive = false;
bool passwordActive = false;
TextBox usernameInput;
TextBox passwordInput;

float loginTimer = 0.0f;
float alertTimer = 15.0f;     
float alertCountdown = 0.0f;  

LogEntry logEntries[10];
int logCount = 0;
Texture2D engineTexture;

button_t startBtn;
button_t stopBtn;
button_t fanUpBtn;   
button_t fanDownBtn; 
button_t lineUpBtn;   
button_t lineDownBtn; 
button_t logoutBtn;

int tries = 3;
int currentTries = 0;
bool started = false;
float stopTimer = 0.0f;
uint64_t counter = 0;

// Parametry symulacji i PC
float motorTemp = 40.0f;
int cpuUsage = 0;    
int ramUsage = 0;    
float cpuTemp = 0.0f;
int fanSpeed = 1000;
int lineSpeed = 0;
int baseLineSpeed = 150; 
float simUpdateTimer = 0.0f;


static FILETIME prevIdleTime, prevKernelTime, prevUserTime;
static bool isCpuInit = false;



int getRealCpuUsage() {
    FILETIME idleTime, kernelTime, userTime;
    if (!GetSystemTimes(&idleTime, &kernelTime, &userTime)) return 0;

    if (!isCpuInit) {
        prevIdleTime = idleTime;
        prevKernelTime = kernelTime;
        prevUserTime = userTime;
        isCpuInit = true;
        return 0;
    }

    ULONGLONG idle = ((ULONGLONG)idleTime.dwHighDateTime << 32) | idleTime.dwLowDateTime;
    ULONGLONG kernel = ((ULONGLONG)kernelTime.dwHighDateTime << 32) | kernelTime.dwLowDateTime;
    ULONGLONG user = ((ULONGLONG)userTime.dwHighDateTime << 32) | userTime.dwLowDateTime;

    ULONGLONG pIdle = ((ULONGLONG)prevIdleTime.dwHighDateTime << 32) | prevIdleTime.dwLowDateTime;
    ULONGLONG pKernel = ((ULONGLONG)prevKernelTime.dwHighDateTime << 32) | prevKernelTime.dwLowDateTime;
    ULONGLONG pUser = ((ULONGLONG)prevUserTime.dwHighDateTime << 32) | prevUserTime.dwLowDateTime;

    ULONGLONG diffIdle = idle - pIdle;
    ULONGLONG diffKernel = kernel - pKernel;
    ULONGLONG diffUser = user - pUser;

    ULONGLONG total = diffKernel + diffUser;
    int usage = 0;
    if (total > 0) {
        usage = (int)((total - diffIdle) * 100 / total);
    }

    prevIdleTime = idleTime;
    prevKernelTime = kernelTime;
    prevUserTime = userTime;

    return usage;
}

int getRealRamUsage() {
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    GlobalMemoryStatusEx(&memInfo);
    return (int)memInfo.dwMemoryLoad; 
}

// --- IMPLEMENTACJE FUNKCJI ---

void initGUI() {
    usernameInput.rect = (Rectangle){ SCREEN_WIDTH/2 - LOGIN_RECT_WIDTH/2, SCREEN_HEIGHT/2 - 20, LOGIN_RECT_WIDTH, LOGIN_RECT_HEIGHT };
    passwordInput.rect = (Rectangle){ SCREEN_WIDTH/2 - LOGIN_RECT_WIDTH/2, SCREEN_HEIGHT/2 + 70, LOGIN_RECT_WIDTH, LOGIN_RECT_HEIGHT };
    usernameInput.text[0] = '\0';
    passwordInput.text[0] = '\0';

    int btnY_Start = SCREEN_HEIGHT/2 + 140;
    int btnY_Fan = SCREEN_HEIGHT/2 + 200;
    int btnY_Line = SCREEN_HEIGHT/2 + 260;

    initButton(&startBtn, "START", SCREEN_WIDTH/2 + 30, btnY_Start, 100, 40, GREEN);
    initButton(&stopBtn, "STOP", SCREEN_WIDTH/2 + 150, btnY_Start, 100, 40, RED);
    
    initButton(&fanUpBtn, "FAN +", SCREEN_WIDTH/2 + 30, btnY_Fan, 100, 40, SKYBLUE);
    initButton(&fanDownBtn, "FAN -", SCREEN_WIDTH/2 + 150, btnY_Fan, 100, 40, BLUE);

    initButton(&lineUpBtn, "LINE +", SCREEN_WIDTH/2 + 30, btnY_Line, 100, 40, YELLOW);
    initButton(&lineDownBtn, "LINE -", SCREEN_WIDTH/2 + 150, btnY_Line, 100, 40, ORANGE);

    // NOWE: Przycisk wylogowania w prawym górnym rogu
    initButton(&logoutBtn, "LOGOUT", SCREEN_WIDTH - 130, 20, 110, 40, ORANGE);
}

void addLog(const char* msg, Color color) {
    for (int i = 9; i > 0; i--) {
        logEntries[i] = logEntries[i-1];
    }
    
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    char timeStr[15];
    strftime(timeStr, 15, "[%H:%M:%S]", tm_info);

    snprintf(logEntries[0].message, 128, "%s %s", timeStr, msg);
    logEntries[0].color = color;
    
    if (logCount < 10) logCount++;
}

void simulateProcess() {
    simUpdateTimer += GetFrameTime();
    
    if (simUpdateTimer >= 1.0f) {
        simUpdateTimer = 0.0f;

        cpuUsage = getRealCpuUsage();
        ramUsage = getRealRamUsage();

        cpuTemp = 35.0f + (cpuUsage / 2.0f); 

        if (started) {
            lineSpeed = baseLineSpeed + (rand() % 20) - 10; 
            if (lineSpeed < 0) lineSpeed = 0;

            motorTemp += (lineSpeed / 100.0f) * 1.8f; 
        }

        motorTemp -= (fanSpeed / 1000.0f) * 1.0f; 

        if (motorTemp < 25.0f) motorTemp = 25.0f;

        if (motorTemp > 90.0f) {
            addLog("AWARIA: Silnik przegrzany! Zatrzymanie awaryjne!", RED);
            started = false;
            lineSpeed = 0;
        } 
        else if (motorTemp > 75.0f && fanSpeed < 2000) {
            addLog("OSTRZEZENIE: Wysoka temp. silnika. Zwieksz nadmuch!", ORANGE);
        }
    }
}

void drawTextBox(TextBox *box, Color color, bool active, bool password) {
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

void drawLogs() {
    DrawRectangle(10, SCREEN_HEIGHT/2 + 100, SCREEN_WIDTH/2 + 10, SCREEN_HEIGHT/2 - 110, BLACK);
    DrawText("DZIENNIK ZDARZEN:", 20, SCREEN_HEIGHT/2 + 110, 20, GRAY);
    
    for (int i = 0; i < logCount; i++) {
        DrawText(logEntries[i].message, 20, SCREEN_HEIGHT/2 + 140 + i * 20, 20, logEntries[i].color);
    }
}

void drawLoginScreen() {
    Vector2 mouse = GetMousePosition();
    
    DrawText("LOGOWANIE DO SYSTEMU", SCREEN_WIDTH/2 - 250, SCREEN_HEIGHT/3 - 40, 40, LIGHTGRAY);

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
        
        if (IsKeyPressed(KEY_BACKSPACE)) {
            int len = strlen(activeBox->text);
            if (len > 0) activeBox->text[len - 1] = '\0';
        }
    }

    drawTextBox(&usernameInput, LOGIN_SCREEN_BOX_COLOR, usernameActive, false);
    drawTextBox(&passwordInput, LOGIN_SCREEN_BOX_COLOR, passwordActive, true);

    if (IsKeyPressed(KEY_ENTER)) {
        if (strcmp(usernameInput.text, "admin") == 0 && strcmp(passwordInput.text, "admin") == 0) {
            loginTimer = 1.5f; 
            alertTimer = 15.0f + (rand() % 20); 
            currentScene = WORKING;
            addLog("System: Logowanie udane.", DARKGREEN);
        } else {
            currentTries++;
            strcpy(usernameInput.text, "WRONG!"); 
            passwordInput.text[0] = '\0';
            loginTimer = 2.0f; 
            if (currentTries >= tries) {
                currentScene = LOCKED; 
            }
        }
    }

    if (loginTimer > 0.0f) {
        loginTimer -= GetFrameTime();
        if (currentScene == LOGIN) {
            DrawText("ACCESS DENIED", SCREEN_WIDTH/2 - 100, SCREEN_HEIGHT/2 + 120, 20, RED);
        }
        if (loginTimer <= 0.0f && currentScene == LOGIN) {
            usernameInput.text[0] = '\0'; 
        }
    }
}

void drawWorkingScreen() {
    if (loginTimer > 0.0f) {
        DrawText("ACCESS GRANTED! WELCOME!", SCREEN_WIDTH/2 - 150, SCREEN_HEIGHT/2, 20, GREEN);
        loginTimer -= GetFrameTime();
        return;
    }

    alertTimer -= GetFrameTime();
    if (alertTimer <= 0.0f) {
        currentScene = ALERT;
        alertCountdown = 30.0f; 
        return;
    }

    simulateProcess();

    DrawText("PANEL STEROWANIA", SCREEN_WIDTH/2 - 250, 20, 40, LIGHTGRAY);

   
    drawButton(&logoutBtn);
    if(isHoveringButton(&logoutBtn)) DrawRectangleLinesEx(logoutBtn.rect, 3, WHITE);

    if(isButtonClicked(&logoutBtn)) {
        currentScene = LOGIN;
        started = false;
        lineSpeed = 0;
        usernameInput.text[0] = '\0';
        passwordInput.text[0] = '\0';
        addLog("System: Wylogowano przez operatora.", ORANGE);
        alertTimer = 15.0f;
        return; 
    }

    drawButton(&startBtn);
    drawButton(&stopBtn);
    drawButton(&fanUpBtn);   
    drawButton(&fanDownBtn); 
    drawButton(&lineUpBtn);   
    drawButton(&lineDownBtn); 

    if(isHoveringButton(&startBtn)) DrawRectangleLinesEx(startBtn.rect, 3, WHITE);
    if(isHoveringButton(&stopBtn)) DrawRectangleLinesEx(stopBtn.rect, 3, WHITE);
    if(isHoveringButton(&fanUpBtn)) DrawRectangleLinesEx(fanUpBtn.rect, 3, WHITE);
    if(isHoveringButton(&fanDownBtn)) DrawRectangleLinesEx(fanDownBtn.rect, 3, WHITE);
    if(isHoveringButton(&lineUpBtn)) DrawRectangleLinesEx(lineUpBtn.rect, 3, WHITE);
    if(isHoveringButton(&lineDownBtn)) DrawRectangleLinesEx(lineDownBtn.rect, 3, WHITE);

    if(isButtonClicked(&startBtn) && !started) {
        started = true;
        addLog("Operator: Uruchomienie linii produkcyjnej.", BLUE);
    }
    if(isButtonClicked(&stopBtn) && started) {
        started = false;
        lineSpeed = 0;
        addLog("Operator: Zatrzymanie linii produkcyjnej.", RED);
    }

    if(isButtonClicked(&fanUpBtn)) {
        fanSpeed += 250;
        if (fanSpeed > 5000) fanSpeed = 5000;
        addLog(TextFormat("Operator: Zwiekszono nadmuch (%d RPM)", fanSpeed), SKYBLUE);
    }
    if(isButtonClicked(&fanDownBtn)) {
        fanSpeed -= 250;
        if (fanSpeed < 0) fanSpeed = 0;
        addLog(TextFormat("Operator: Zmniejszono nadmuch (%d RPM)", fanSpeed), BLUE);
    }

    if(isButtonClicked(&lineUpBtn)) {
        baseLineSpeed += 50;
        if (baseLineSpeed > 800) baseLineSpeed = 800; // Limit
        addLog(TextFormat("Operator: Przyspieszono linie (%d bazowo)", baseLineSpeed), YELLOW);
    }
    if(isButtonClicked(&lineDownBtn)) {
        baseLineSpeed -= 50;
        if (baseLineSpeed < 50) baseLineSpeed = 50; // Limit
        addLog(TextFormat("Operator: Zwolniono linie (%d bazowo)", baseLineSpeed), ORANGE);
    }

    int paramX = SCREEN_WIDTH/2 + 50;
    int paramY = SCREEN_HEIGHT/2 - 100;
    DrawText("PARAMETRY PC:", paramX, paramY, 20, GRAY);
    DrawText(TextFormat("Zasoby CPU PC: %d %%", cpuUsage), paramX, paramY + 30, 20, cpuUsage > 80 ? RED : GREEN);
    DrawText(TextFormat("Zasoby RAM PC: %d %%", ramUsage), paramX, paramY + 60, 20, ramUsage > 80 ? RED : GREEN);
    
    DrawText("PARAMETRY LINII:", paramX, paramY + 100, 20, GRAY);
    DrawText(TextFormat("Temp. Silnika linii: %.1f C", motorTemp), paramX, paramY + 130, 20, motorTemp > 75 ? RED : WHITE);
    DrawText(TextFormat("Predkosc Wentylatora: %d RPM", fanSpeed), paramX, paramY + 160, 20, SKYBLUE);
    DrawText(TextFormat("Predkosc Linii: %d szt/min", lineSpeed), paramX, paramY + 190, 20, lineSpeed == 0 ? RED : GREEN);

    
    Rectangle sourceRec = { 0.0f, 0.0f, (float)engineTexture.width, (float)engineTexture.height };
    float displayWidth = 150.0f;
    float displayHeight = 120.0f;
    Rectangle destRec = { 160.0f, (float)SCREEN_HEIGHT / 3.0f, displayWidth, displayHeight };
    Vector2 origin = { displayWidth / 2.0f, displayHeight / 2.0f };
    Color pulseColor = (Color){ 255, 255, 255, (uint8_t)(128 + 127 * sin(counter * 0.1f)) };

    if(started) {
        DrawTexturePro(engineTexture, sourceRec, destRec, origin, 0.0f, GREEN);
    } else {
        DrawTexturePro(engineTexture, sourceRec, destRec, origin, 0.0f, pulseColor);
    }

    drawLogs();
}

void drawAlertScreen() {
    ClearBackground(MAROON);
    DrawText("DIAGNOSTYKA: SPRAWDZANIE OBECNOSCI OPERATORA!", SCREEN_WIDTH/2 - 350, SCREEN_HEIGHT/2 - 50, 25, WHITE);
    DrawText(TextFormat("Zostalo: %.1f sekund do wylogowania", alertCountdown), SCREEN_WIDTH/2 - 200, SCREEN_HEIGHT/2 + 20, 20, YELLOW);
    DrawText("Nacisnij dowolny klawisz lub kliknij aby potwierdzic.", SCREEN_WIDTH/2 - 250, SCREEN_HEIGHT/2 + 70, 20, LIGHTGRAY);

    alertCountdown -= GetFrameTime();

    if (alertCountdown <= 0.0f) {
        currentScene = LOGIN;
        started = false;
        lineSpeed = 0;
        usernameInput.text[0] = '\0';
        passwordInput.text[0] = '\0';
        addLog("SYSTEM: Wylogowano z powodu braku aktywnosci!", RED);
        alertTimer = 15.0f;
    } 
    else if (GetKeyPressed() > 0 || IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        alertTimer = 20.0f + (rand() % 40); 
        currentScene = WORKING; 
        addLog("Operator: Potwierdzono obecnosc.", GREEN);
    }
}

void drawMainScreen() {
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
            DrawText("TOO MANY ATTEMPTS! LOCKED OUT!", SCREEN_WIDTH/2 - 250, SCREEN_HEIGHT/2, 30, WHITE);  
            break;
    }
}

int main(void) {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Symulator Linii Produkcyjnej");
    SetTargetFPS(60);
    srand(time(NULL));

    engineTexture = LoadTexture("cos.png");

    initGUI();

    while (!WindowShouldClose()) {
        counter++;
        
        BeginDrawing();
        ClearBackground(GetColor(0x202020FF)); 
        
        drawMainScreen();
        
        EndDrawing();
    }

    UnloadTexture(engineTexture);
    CloseWindow();
    return 0;
}
