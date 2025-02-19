#include "raylib.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include "rapidjson/document.h"
#include "button.hpp"

using namespace rapidjson;
using namespace std;

enum WindowState
{
    HOME_WINDOW,
    GAMEPLAY_WINDOW
};
WindowState currentWindow = HOME_WINDOW;

int main()
{

    // Read the JSON file
    ifstream file("data.json");

    // Read file content into a string
    stringstream buffer;
    buffer << file.rdbuf();
    string jsonStr = buffer.str();

    Document document;
    if (document.Parse(jsonStr.c_str()).HasParseError())
    {
        cout << "Failed to parse JSON" << endl;
        return 1;
    }

    // // Loop through each object in the array
    // for (const auto &item : document.GetArray())
    // {
    //     cout << "Desk: " << item["desk"].GetString() << endl;

    //     // Loop through the "data" array
    //     for (const auto &entry : item["data"].GetArray())
    //     {
    //         cout << "  Word: " << entry["word"].GetString() << endl;
    //         cout << "  Meaning: " << entry["meaning"].GetString() << endl;
    //         cout << "  Image: " << entry["image"].GetString() << endl;
    //         cout << endl;
    //     }
    // }

    // Initialize window
    const float screenWidth = 1000;
    const float screenHeight = 750;

    InitWindow(screenWidth, screenHeight, "Die for you");
    SetTargetFPS(60);

    // homepage
    Button topbar{"img/homepage/topbar.png", {0, 0}, 1};
    Button newDesk{"img/homepage/new-desk.png", {48, 95}, 1};
    Button foodBtn{"img/homepage/Foods.png", {278.12, 95}, 1};
    Button animalBtn{"img/homepage/animals.png", {508.24, 95}, 1};
    Button flowerBtn{"img/homepage/flowers.png", {738.36, 95}, 1};
    Button countryBtn{"img/homepage/country.png", {48, 384}, 1};
    Button jobsBtn{"img/homepage/jobs.png", {278.12, 384}, 1};
    Button vegetableBtn{"img/homepage/vegetable.png", {508.24, 384}, 1};

    // gameplay
    Button gpBG{"img/gameplay/bg.png", {48, 48}, 1};
    Button gpHome{"img/gameplay/home-btn.png", {64, 650}, 1};
    Button gpPrevious{"img/gameplay/previous-btn.png", {472, 650}, 1};
    Button gpNext{"img/gameplay/next-btn.png", {528, 650}, 1};
    Button gpAns{"img/gameplay/show-answer-btn.png", {809, 650}, 1};
    Button gpEasyBtn{"img/gameplay/easy-btn.png", {340, 590}, 1};
    Button gpMedBtn{"img/gameplay/medium-btn.png", {452, 590}, 1};
    Button gpHardBtn{"img/gameplay/hard-btn.png", {564, 590}, 1};
    
    //fade button
    Button gpPreviousF{"img/gameplay/Fprevious-btn.png", {472, 650}, 1};
    Button gpNextF{"img/gameplay/Fnext-btn.png", {528, 650}, 1};

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
            if (gpHome.isPressed(mousePosition, mousePressed))
            {
                currentWindow = HOME_WINDOW;
            }
            gpBG.Draw();
            gpHome.Draw();
            gpNext.Draw();
            gpPrevious.Draw();
            gpAns.Draw();
            gpEasyBtn.Draw();
            gpMedBtn.Draw();
            gpHardBtn.Draw();
        }
        EndDrawing();
    }

    CloseWindow();
    return 0;
}