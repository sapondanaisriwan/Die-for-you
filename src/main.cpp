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

int currentPage = 0;
int currentDesk = 0;
Texture2D wordImage;
bool imageLoaded = false;

Document getData()
{
    // Read the JSON file
    ifstream file("data.json");

    // Read file content into a string
    stringstream buffer;
    buffer << file.rdbuf();
    string jsonStr = buffer.str();

    Document document;
    document.Parse(jsonStr.c_str());
    return document;
}

int main()
{

    // Get the JSON document
    Document document = getData();

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
    Button gpPreviousFade{"img/gameplay/previous-btn.png", {472, 650}, 1};
    Button gpPrevious{"img/gameplay/previous-btn2.png", {472, 650}, 1};
    Button gpNext{"img/gameplay/next-btn.png", {528, 650}, 1};
    Button gpAns{"img/gameplay/show-answer-btn.png", {809, 650}, 1};
    Button gpEasyBtn{"img/gameplay/easy-btn.png", {340, 590}, 1};
    Button gpMedBtn{"img/gameplay/medium-btn.png", {452, 590}, 1};
    Button gpHardBtn{"img/gameplay/hard-btn.png", {564, 590}, 1};

    while (!WindowShouldClose())
    {

        Vector2 mousePosition = GetMousePosition();
        bool mousePressed = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);

        BeginDrawing();
        ClearBackground(RAYWHITE);

        if (currentWindow == HOME_WINDOW)
        {
            currentPage = 0;
            imageLoaded = false;
            topbar.Draw();
            newDesk.Draw();
            foodBtn.Draw();
            animalBtn.Draw();
            flowerBtn.Draw();
            countryBtn.Draw();
            jobsBtn.Draw();
            vegetableBtn.Draw();
            if (animalBtn.isPressed(mousePosition, mousePressed))
            {
                currentWindow = GAMEPLAY_WINDOW;
                currentDesk = 0;
            }
            else if (foodBtn.isPressed(mousePosition, mousePressed))
            {
                currentWindow = GAMEPLAY_WINDOW;
                currentDesk = 1;
            }
            else if (flowerBtn.isPressed(mousePosition, mousePressed))
            {
                currentWindow = GAMEPLAY_WINDOW;
                currentDesk = 2;
            }
            else if (countryBtn.isPressed(mousePosition, mousePressed))
            {
                currentWindow = GAMEPLAY_WINDOW;
                currentDesk = 3;
            }
            else if (jobsBtn.isPressed(mousePosition, mousePressed))
            {
                currentWindow = GAMEPLAY_WINDOW;
                currentDesk = 4;
            }
            else if (vegetableBtn.isPressed(mousePosition, mousePressed))
            {
                currentWindow = GAMEPLAY_WINDOW;
                currentDesk = 5;
            }
        }
        else if (currentWindow == GAMEPLAY_WINDOW)
        {

            if (gpHome.isPressed(mousePosition, mousePressed))
            {
                currentWindow = HOME_WINDOW;
                UnloadTexture(wordImage);
            }

            if (!imageLoaded)
            {
                string imgPath = document[currentDesk]["data"][currentPage]["image"].GetString();
                wordImage = LoadTexture(imgPath.c_str());
                imageLoaded = true;
                cout << imgPath << endl;
            }

            if (gpNext.isPressed(mousePosition, mousePressed))
            {
                currentPage++;
                UnloadTexture(wordImage);
                imageLoaded = false;
            }

            if (gpPrevious.isPressed(mousePosition, mousePressed) && currentPage > 0)
            {
                currentPage--;
                UnloadTexture(wordImage);
                imageLoaded = false;
            }
            if (gpPreviousFade.isPressed(mousePosition, mousePressed) && currentPage > 0)
            {
                currentPage--;
                UnloadTexture(wordImage);
                imageLoaded = false;
            }

            gpBG.Draw();
            gpHome.Draw();
            gpNext.Draw();

            if (currentPage > 0)
                gpPrevious.Draw();
            else
                gpPreviousFade.Draw();

            gpAns.Draw();
            gpEasyBtn.Draw();
            gpMedBtn.Draw();
            gpHardBtn.Draw();

            if (imageLoaded)
            {
                DrawTexture(wordImage, 254, 148, WHITE);
            }
        }
        EndDrawing();
    }

    CloseWindow();
    return 0;
}