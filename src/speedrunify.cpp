#include "raylib.h"
#include <ctime>
#include <vector>
#include <string>
#include <iostream>

extern "C" {
    __declspec(dllimport) void* __stdcall GetActiveWindow();
    __declspec(dllimport) int __stdcall SetWindowPos(void* hWnd, void* hWndInsertAfter, int X, int Y, int cx, int cy, unsigned int uFlags);
}

#define HWND_TOPMOST ((void*)-1)
#define HWND_NOTOPMOST ((void*)-2)
#define SWP_NOSIZE 0x0001
#define SWP_NOMOVE 0x0002

int main() {
    SetConfigFlags(FLAG_WINDOW_TRANSPARENT | FLAG_WINDOW_UNDECORATED | FLAG_MSAA_4X_HINT);
    int baseWidth = 420;
    int baseHeight = 200;
    InitWindow(baseWidth, baseHeight, "Speedrunify");
    Image appIcon = LoadImage("icon.png"); 
    if (appIcon.data != NULL) {
        ImageFormat(&appIcon, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
        ImageResize(&appIcon, 64, 64);
        SetWindowIcon(appIcon);
    }
    UnloadImage(appIcon);

    int count = 0;
    int charList[] = { 0x1F4CC, 0x23F1, 0x1F552, 0x2699, 0x1F504, 0x25B6, 0x23F8, 0x2716 };
    int* cp = LoadCodepoints(" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~", &count);
    
    std::vector<int> allCP;
    for(int i=0; i<count; i++) allCP.push_back(cp[i]);
    for(int i=0; i<8; i++) allCP.push_back(charList[i]);

    Font mainFont = LoadFontEx("C:/Windows/Fonts/seguiemj.ttf", 96, allCP.data(), (int)allCP.size());
    SetTextureFilter(mainFont.texture, TEXTURE_FILTER_BILINEAR);
    UnloadCodepoints(cp);

    SetTargetFPS(60);

    bool isAlwaysOnTop = false, is24Hour = true, showSettings = false, swRunning = false;
    float stopwatchTime = 0.0f, currentScale = 1.0f, snapMargin = 30.0f;
    int currentMode = 0, dummySize = 0;
    Vector2 dragOffset = { 0, 0 };
    bool isDragging = false;

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        float sw = (float)GetScreenWidth();
        float sh = (float)GetScreenHeight();
        Vector2 mouse = GetMousePosition();

        if (currentMode == 1 && swRunning) stopwatchTime += dt;

        if (IsKeyDown(KEY_LEFT_CONTROL)) {
            float wheel = GetMouseWheelMove();
            if (wheel != 0) {
                currentScale += wheel * 0.1f;
                if (currentScale < 0.6f) currentScale = 0.6f;
                if (currentScale > 1.8f) currentScale = 1.8f;
                SetWindowSize((int)(baseWidth * currentScale), (int)(baseHeight * currentScale));
            }
        }

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            if (mouse.y < sh * 0.75f && mouse.x < sw - 50) {
                dragOffset = mouse;
                isDragging = true;
            }
        }
        if (isDragging) {
            if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
                Vector2 winPos = GetWindowPosition();
                float nextX = winPos.x + (mouse.x - dragOffset.x);
                float nextY = winPos.y + (mouse.y - dragOffset.y);
                int mW = GetMonitorWidth(GetCurrentMonitor()), mH = GetMonitorHeight(GetCurrentMonitor());
                if (nextX < snapMargin) nextX = 0;
                if (nextX > (mW - sw) - snapMargin) nextX = (float)(mW - sw);
                if (nextY < snapMargin) nextY = 0;
                if (nextY > (mH - sh) - snapMargin) nextY = (float)(mH - sh);
                SetWindowPosition((int)nextX, (int)nextY);
            } else isDragging = false;
        }

        char mainBuf[32], subBuf[64];
        if (currentMode == 0) {
            time_t now = time(0);
            struct tm *ltm = localtime(&now);
            strftime(mainBuf, sizeof(mainBuf), is24Hour ? "%H:%M:%S" : "%I:%M:%S %p", ltm);
            strftime(subBuf, sizeof(subBuf), "%A, %B %d, %Y", ltm);
        } else {
            int mins = (int)stopwatchTime / 60;
            int secs = (int)stopwatchTime % 60;
            int ms = (int)((stopwatchTime - (int)stopwatchTime) * 100);
            sprintf(mainBuf, "%02d:%02d.%02d", mins, secs, ms);
            sprintf(subBuf, "STOPWATCH MODE");
        }

        BeginDrawing();
            ClearBackground(BLANK);
            DrawRectangleRounded({5, 5, sw - 10, sh - 10}, 0.15f, 12, ColorAlpha(BLACK, 0.85f));
            DrawRectangleRoundedLines({5, 5, sw - 10, sh - 10}, 0.15f, 12, ColorAlpha(WHITE, 0.1f));

            Rectangle setRec = { 20, 15, 30, 30 };
            bool isSettingHovered = CheckCollisionPointRec(mouse, setRec);
            if (isSettingHovered && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) showSettings = !showSettings;
            Color settingColor = isSettingHovered ? WHITE : ( showSettings ? WHITE : GRAY ); 
            DrawTextEx(mainFont, CodepointToUTF8(0x2699, &dummySize), {20, 15}, 16 * currentScale, 1, settingColor);

            Rectangle exitRec = { sw - 45, 15, 30, 30 };
            if (CheckCollisionPointRec(mouse, exitRec) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) break;
            DrawTextEx(mainFont, CodepointToUTF8(0x2716, &dummySize), { sw - 45, 15}, 14 * currentScale, 1, CheckCollisionPointRec(mouse, exitRec) ? WHITE : GRAY);

            float subS = sh * 0.10f;
            float mainS = sh * 0.25f;
            Vector2 subV = MeasureTextEx(mainFont, subBuf, subS, 2);
            Vector2 mainV = MeasureTextEx(mainFont, mainBuf, mainS, 2);
            float subY = sh * 0.22f;
            float mainY = subY + subV.y + (8 * currentScale);

            DrawTextEx(mainFont, subBuf, {(sw - subV.x)/2, subY}, subS, 2, ColorAlpha(WHITE, 0.6f));
            DrawTextEx(mainFont, mainBuf, {(sw - mainV.x)/2, mainY}, mainS, 2, WHITE);

            float bY = sh - (45 * currentScale);
            float iS = 18 * currentScale;

            Rectangle pRec = { 20, bY, 80 * currentScale, 30 * currentScale };
            bool pinHovered = CheckCollisionPointRec(mouse, pRec);
            if (pinHovered && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                isAlwaysOnTop = !isAlwaysOnTop;
                SetWindowPos(GetActiveWindow(), isAlwaysOnTop ? HWND_TOPMOST : HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
            }
            Color pinColor = pinHovered ? WHITE : ( isAlwaysOnTop ? WHITE : GRAY );
            DrawTextEx(mainFont, TextFormat("%s %s", CodepointToUTF8(0x1F4CC, &dummySize), isAlwaysOnTop ? "" : ""), {20, bY}, iS, 1, pinColor);

            const char* mIcon = CodepointToUTF8(currentMode == 0 ? 0x23F1 : 0x1F552, &dummySize);
            float mWidth = MeasureTextEx(mainFont, TextFormat("%s", mIcon), iS, 1).x;
            Rectangle mRec = { sw - mWidth - 20, bY, mWidth, 30 * currentScale };
            bool isModeHovered = CheckCollisionPointRec(mouse, mRec);
            Color modeColor = isModeHovered ? WHITE : GRAY;
            if (isModeHovered && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                currentMode = (currentMode == 0) ? 1 : 0;
            }
            DrawTextEx(mainFont, TextFormat("%s", mIcon), {sw - mWidth - 20, bY}, iS, 1, modeColor);

            if (currentMode == 1) {
                Rectangle playRec = { sw/2 - 40 * currentScale, bY, 30, 30 };
                bool isPlayHovered = CheckCollisionPointRec(mouse, playRec);
                if (isPlayHovered && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) swRunning = !swRunning;
                Color playColor = (!swRunning) ? SKYBLUE : (isPlayHovered ? WHITE : GRAY);
                DrawTextEx(mainFont, CodepointToUTF8(swRunning ? 0x23F8 : 0x25B6, &dummySize), {playRec.x, bY}, iS + 5, 1, playColor);

                Rectangle resRec = { sw/2 + 10 * currentScale, bY, 30, 30 };
                bool isResetHovered = CheckCollisionPointRec(mouse, resRec);
                if (isResetHovered && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) { 
                    stopwatchTime = 0; 
                    swRunning = false; 
                }
                Color resetColor = isResetHovered ? WHITE : GRAY;
                DrawTextEx(mainFont, CodepointToUTF8(0x1F504, &dummySize), {resRec.x, bY}, iS + 5, 1, resetColor);
            }

            if (showSettings) {
                DrawRectangleRounded({20, 50, 160 * currentScale, 40 * currentScale}, 0.2f, 8, ColorAlpha(DARKGRAY, 0.95f));
                if (CheckCollisionPointRec(mouse, {20, 50, 160*currentScale, 40*currentScale}) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) is24Hour = !is24Hour;
                DrawTextEx(mainFont, is24Hour ? "24H" : "AM/PM", {35, 60}, 14 * currentScale, 1, WHITE);
            }

        EndDrawing();
    }

    UnloadFont(mainFont);
    CloseWindow();
    return 0;
}