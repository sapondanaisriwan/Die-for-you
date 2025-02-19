#include "raylib.h"
#include <iostream>
#include "button.hpp"

using namespace std;

enum WindowState
{
    HOME_WINDOW,
    GAMEPLAY_WINDOW
};
WindowState currentWindow = HOME_WINDOW;

void homepage(Texture2D textures[], Vector2 positions[])
{
}

int main()
{
    // Initialize window
    const float screenWidth = 1000;
    const float screenHeight = 750;

    InitWindow(screenWidth, screenHeight, "Die for you");
    SetTargetFPS(60);

    Button topbar{"img/homepage/topbar.png", {0, 0}, 1};
    Button newDesk{"img/homepage/new-desk.png", {48, 95}, 1};
    Button foodBtn{"img/homepage/Foods.png", {278.12, 95}, 1};
    Button animalBtn{"img/homepage/animals.png", {508.24, 95}, 1};
    Button flowerBtn{"img/homepage/flowers.png", {738.36, 95}, 1};
    Button countryBtn{"img/homepage/country.png", {48, 384}, 1};
    Button jobsBtn{"img/homepage/jobs.png", {278.12, 384}, 1};
    Button vegetableBtn{"img/homepage/vegetable.png", {508.24, 384}, 1};

    while (!WindowShouldClose())
    {

        Vector2 mousePosition = GetMousePosition();
        bool mousePressed = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);

        BeginDrawing();
        ClearBackground(RAYWHITE);
        if (currentWindow == HOME_WINDOW)
        {
            if (foodBtn.isPressed(mousePosition, mousePressed))
            {
                currentWindow = GAMEPLAY_WINDOW;
            }
            topbar.Draw();
            newDesk.Draw();
            foodBtn.Draw();
            animalBtn.Draw();
            flowerBtn.Draw();
            countryBtn.Draw();
            jobsBtn.Draw();
            vegetableBtn.Draw();
        }
        else if (currentWindow == GAMEPLAY_WINDOW)
        {
        }
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
