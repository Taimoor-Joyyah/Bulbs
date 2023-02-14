#include <iostream>
#include <random>
#include <thread>
#include <cmath>

#define RAYGUI_IMPLEMENTATION

#include "raylib.h"
#include "raygui.h"
#include "raymath.h"

struct Bulb {
    Vector2 pos;
    Color color;
};

void generateBulbs(Bulb bulbs[100]) {
    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_real_distribution<float> randX(320, 1920);
    std::uniform_real_distribution<float> randY(0, 1080);
    std::uniform_real_distribution<float> randC(1, 255);

    for (int i = 0; i < 100; ++i) {
        bulbs[i].pos.x = randX(mt);
        bulbs[i].pos.y = randY(mt);
        bulbs[i].color.r = randC(mt);
        bulbs[i].color.b = randC(mt);
        bulbs[i].color.g = randC(mt);
        bulbs[i].color.a = randC(mt);
    }
}

bool inControls(const Vector2 &mouse) {
    return mouse.x <= 300 && mouse.y <= 600;
}

bool inRange(const Vector2 &mouse, const Vector2 &bulb, float range) {
    return Vector2Distance(mouse, bulb) <= range;
}

void renderData(Color render[1920 * 1080], Bulb bulbs[10000], int *countB, float *intensity, bool *singleRender,
                bool *stop, bool *renderStop, int processId) {
    int bS = 64;
    while (!*stop) {
        if (*singleRender) {
            for (int bY = processId / 2; !*renderStop && bY < 1080.0 / bS; bY += 2)
                for (int bX = processId % 2; !*renderStop && bX < 1920.0 / bS; bX += 2)
                    for (int y = 0; !*renderStop && y < bS && bY * bS + y < 1080; y++) {
                        for (int x = 0; !*renderStop && x < bS && bX * bS + x >= 320 && bX * bS + x < 1920; x++) {
                            int index = (bY * bS + y) * 1920 + (bX * bS + x);
                            float r = 0;
                            float b = 0;
                            float g = 0;
                            for (int i = 0; i < *countB; ++i) {
                                float factor = *intensity / pow(Vector2Distance({(float) bX * bS + x, (float) bY * bS + y},
                                                                            bulbs[i].pos),2);
                                r += bulbs[i].color.r * factor / bulbs[i].color.a;
                                b += bulbs[i].color.b * factor / bulbs[i].color.a;
                                g += bulbs[i].color.g * factor / bulbs[i].color.a;
                            }
                            render[index].r = 255 * std::min(1.0f, r);
                            render[index].b = 255 * std::min(1.0f, b);
                            render[index].g = 255 * std::min(1.0f, g);
                            render[index].a = 255;
                        }
                    }
            *singleRender = false;
            *renderStop = false;
        }
    }
}

int main() {
    const int width = 800;
    const int height = 800;

    InitWindow(width, height, "Bulb Rendering");
    SetWindowState(FLAG_WINDOW_RESIZABLE | FLAG_WINDOW_MAXIMIZED);
    SetTargetFPS(60);

    auto *bulbs = new Bulb[100];
    generateBulbs(bulbs);

    int countB = 100;
    float radiusB = 20;
    float scale = 5;
    float power = 5;
    float intensity = pow(scale, power);
    bool showRender = false;
    bool continuousRender = false;
    bool singleRender = false;
    bool renderStop = false;
    int selectedBulb = -1;
    bool stop = false;

    auto *render = new Color[1080 * 1920];
    std::thread t1(renderData, render, bulbs, &countB, &intensity, &singleRender, &stop, &renderStop, 0);
    std::thread t2(renderData, render, bulbs, &countB, &intensity, &singleRender, &stop, &renderStop, 1);
    std::thread t3(renderData, render, bulbs, &countB, &intensity, &singleRender, &stop, &renderStop, 2);
    std::thread t4(renderData, render, bulbs, &countB, &intensity, &singleRender, &stop, &renderStop, 3);

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground({8, 8, 8, 255});

        if (showRender) {
            int bS = 64;
            for (int bY = 0; bY < 1080.0 / bS; ++bY)
                for (int bX = 0; bX < 1920.0 / bS; ++bX)
                    for (int y = 0; y < bS && bY * bS + y < 1080; y++)
                        for (int x = 0; x < bS && bX * bS + x >= 320 && bX * bS + x < 1920; x++)
                            DrawPixel(bX * bS + x, bY * bS + y, render[(bY * bS + y) * 1920 + (bX * bS + x)]);
        } else
            for (int i = 0; i < countB; ++i) {
                DrawCircle(bulbs[i].pos.x, bulbs[i].pos.y, radiusB, {255, 187, 115, 255});
            }


        GuiWindowBox({0, 0, 320, 1080}, "Controls");

        GuiLabel({20, 25, 70, 20}, "Bulbs");
        GuiLabel({20, 55, 70, 20}, "RadiusB");
        GuiLabel({20, 85, 70, 20}, "Scale");
        GuiLabel({20, 115, 70, 20}, "Power");

        GuiLabel({20, 135, 70, 20}, "Show Render: ");
        GuiLabel({100, 135, 70, 20}, std::to_string(showRender).c_str());
        GuiLabel({20, 155, 70, 20}, "Cont Render: ");
        GuiLabel({100, 155, 70, 20}, std::to_string(continuousRender).c_str());


        countB = (int) GuiSliderPro({100, 25, 150, 20}, "1", "100", (float) countB, 1, 100, 25);
        radiusB = GuiSliderPro({100, 55, 150, 20}, "1", "100", radiusB, 5, 100, 25);
        scale = GuiSliderPro({100, 85, 150, 20}, "0", "10", scale, 0, 10, 10);
        power = GuiSliderPro({100, 115, 150, 20}, "0", "10", power, 0, 10, 10);

        intensity = pow(scale, power) / 100000;

        if (GuiButton({100, 200, 100, 50}, "Random Generate")) {
            generateBulbs(bulbs);
            singleRender = true;
        }

        if (GuiButton({100, 260, 100, 50}, "Render")
            || continuousRender || GetKeyPressed() == KEY_R) {
            singleRender = true;
        }


        if (IsKeyDown(KEY_S))
            showRender = true;
        if (IsKeyDown(KEY_V))
            continuousRender = true;
        if (IsKeyDown(KEY_B)) {
            continuousRender = false;
            showRender = false;
        }
        if (IsKeyDown(KEY_P)) {
            renderStop = true;
            continuousRender = false;
        }

        DrawFPS(1800, 20);

        Vector2 mouse = GetMousePosition();
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && !inControls(mouse)) {
            if (selectedBulb == -1) {
                for (int i = 0; i < countB; ++i)
                    if (inRange(mouse, bulbs[i].pos, radiusB)) {
                        selectedBulb = i;
                        break;
                    }
            } else {
                bulbs[selectedBulb].pos.x = mouse.x;
                bulbs[selectedBulb].pos.y = mouse.y;
            }
        } else
            selectedBulb = -1;
        EndDrawing();
    }
    stop = true;
    CloseWindow();

    return 0;
}
