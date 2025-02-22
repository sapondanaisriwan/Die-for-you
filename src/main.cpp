#include "raylib.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <algorithm> // For shuffle
#include <random>    // For random generator
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "button.hpp"
#include "type.hpp"

using namespace rapidjson;
using namespace std;

WindowState currentWindow = HOME_WINDOW;

int currentPage = 0;
int currentDesk = 0;
bool imageLoaded = false;
bool showAnswer = false;
bool isShuffled = false;
Texture2D wordImage;

Document getData()
{
    // Read the JSON file
    ifstream file("resources/data.json");

    // Read file content into a string
    stringstream buffer;
    buffer << file.rdbuf();
    string jsonStr = buffer.str();

    Document document;
    document.Parse(jsonStr.c_str());
    file.close();
    return document;
}

void shuffleDeck()
{
    Document document = getData();

    // Random number generator
    random_device rd;
    mt19937 rng(rd());

    // Loop through all desks and shuffle their "data" arrays
    for (SizeType i = 0; i < document.Size(); i++)
    {
        Value &item = document[i];

        // Check if "data" exists and is an array
        if (item.HasMember("data") && item["data"].IsArray())
        {
            Value &dataArray = item["data"]; // Get the data array
            Document::AllocatorType &allocator = document.GetAllocator();

            // Store elements in a vector
            vector<Value> dataVector;
            for (SizeType j = 0; j < dataArray.Size(); j++)
            {
                dataVector.push_back(Value(dataArray[j], allocator)); // Deep copy
            }

            // Shuffle the vector
            shuffle(dataVector.begin(), dataVector.end(), rng);

            // Clear the original data array
            dataArray.Clear();

            // Copy shuffled data back into JSON array
            for (auto &entry : dataVector)
            {
                // cout << entry["name"].GetString() << endl;
                dataArray.PushBack(entry, allocator);
            }
        }
    }

    // Convert JSON back to string
    StringBuffer bufferOut;
    Writer<StringBuffer> writer(bufferOut);
    document.Accept(writer);

    // Write the updated JSON back to the file
    ofstream outFile("resources/data.json");
    if (!outFile)
    {
        cerr << "Error: Cannot write to file!" << endl;
        return;
    }
    outFile << bufferOut.GetString();
    outFile.close();
}

Vector2 GetCenteredTextPos(Font font, string text, int fontSize, Vector2 screencenterPos, float yPos)
{
    Vector2 textXY = MeasureTextEx(font, text.c_str(), fontSize, 0);
    Vector2 result = {(screencenterPos.x - (textXY.x / 2.0f)), yPos};
    return result;
}

int main()
{

    // Get the JSON document
    // shuffleDeck();
    Document document = getData();
    // Initialize window
    const float screenWidth = 1000;
    const float screenHeight = 750;

    // Center position
    Vector2 screenCenterPos = {screenWidth / 2.0f, screenHeight / 2.0f};

    InitWindow(screenWidth, screenHeight, "Die for you");
    SetTargetFPS(60);

    // Get font
    Font InterSemiBold = LoadFont("resources/Inter_SemiBold.ttf");
    Font InterMedium = LoadFont("resources/Inter_Medium.ttf");
    Font InterRegular = LoadFont("resources/Inter_Regular.ttf");
    Font InterLight = LoadFont("resources/InterLight.ttf");

    // Smooth the font
    SetTextureFilter(InterSemiBold.texture, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(InterMedium.texture, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(InterRegular.texture, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(InterLight.texture, TEXTURE_FILTER_BILINEAR);

    float xPos = 48;
    float yPos = 95;
    float xImage = 88;
    float yImage = 120;
    oldPos oldPos = {xPos, yPos, xImage, yImage};
    vector<Button> deckButtons = {};
    vector<Button> deckCovers = {};
    for (SizeType i = 0; i < document.Size(); i++)
    {
        Value &obj = document[i];
        if (obj.HasMember("cover"))
        {
            string coverPath = obj["cover"].GetString();
            deckButtons.push_back(Button("img/homepage/card-template.png", {xPos, yPos}, 1));
            deckCovers.push_back(Button(coverPath.c_str(), {xImage, yImage}, {144, 144}));
            xPos += 230;
            xImage += 230;
            if (xPos > 800)
            {
                xPos = oldPos.xPos;
                yPos += 289;
                xImage = oldPos.xImage;
                yImage += 287;
            }
        }
    }

    // gameplay
    Button gpBG{"img/gameplay/bg.png", {48, 48}, 1};
    Button gpHome{"img/gameplay/home-btn.png", {64 + 4, 650}, 1};
    Button gpPreviousFade{"img/gameplay/previous-btn.png", {443 - 4, 649}, 1};
    Button gpPrevious{"img/gameplay/previous-btn2.png", {443 - 4, 649}, 1};
    Button gpNext{"img/gameplay/next-btn.png", {529 + 4, 649}, 1};
    Button gpShowAns{"img/gameplay/show-ans-btn.png", {809, 651}};
    Button gpHideAns{"img/gameplay/hide-ans-btn.png", {809, 651}, 1};
    Button gpEasyBtn{"img/gameplay/easy-btn.png", {340 + 4, 590}, 1};
    Button gpMedBtn{"img/gameplay/medium-btn.png", {452 + 4, 590}, 1};
    Button gpHardBtn{"img/gameplay/hard-btn.png", {564 + 4, 590}, 1};

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
            showAnswer = false;

            for (size_t i = 0; i < deckButtons.size(); i++)
            {
                deckButtons[i].Draw();
                deckCovers[i].Draw();
                bool isClicked = deckButtons[i].isPressed(mousePosition, mousePressed) || deckCovers[i].isPressed(mousePosition, mousePressed);
                if (isClicked)
                {
                    currentWindow = GAMEPLAY_WINDOW;
                    currentDesk = i;
                }
            }
        }
        else if (currentWindow == GAMEPLAY_WINDOW)
        {

            if (isShuffled == false)
            {
                shuffleDeck();
                document = getData();
                isShuffled = !isShuffled;
            }

            Value &currentDeskData = document[currentDesk]["data"];
            Value &currentPageData = currentDeskData[currentPage];

            int dataSize = currentDeskData.Size() - 1;
            string wordDesk = currentPageData["word"].GetString();
            string meaning = currentPageData["meaning"].GetString();
            string imgPath = currentPageData["image"].GetString();

            if (!imageLoaded)
            {
                wordImage = LoadTexture(imgPath.c_str());

                imageLoaded = true;
                cout << imgPath << endl;
            }

            if (gpHome.isPressed(mousePosition, mousePressed))
            {
                currentWindow = HOME_WINDOW;
                isShuffled = !isShuffled;
                UnloadTexture(wordImage);
            }
            else if (gpNext.isPressed(mousePosition, mousePressed) && currentPage < dataSize)
            {
                currentPage++;
                UnloadTexture(wordImage);
                imageLoaded = false;
                showAnswer = false;
            }
            else if (gpShowAns.isPressed(mousePosition, mousePressed))
            {
                showAnswer = !showAnswer;
            }
            else if ((gpPrevious.isPressed(mousePosition, mousePressed) || gpPreviousFade.isPressed(mousePosition, mousePressed)) && currentPage > 0)
            {
                currentPage--;
                UnloadTexture(wordImage);
                imageLoaded = false;
                showAnswer = false;
            }

            gpBG.Draw();
            gpHome.Draw();
            gpNext.Draw();
            gpShowAns.Draw();

            if (showAnswer)
            {
                gpEasyBtn.Draw();
                gpMedBtn.Draw();
                gpHardBtn.Draw();
                gpHideAns.Draw();

                // Show answer text
                Vector2 answerPos = GetCenteredTextPos(InterSemiBold, meaning, 32, screenCenterPos, 529 + 6);
                DrawTextEx(InterMedium, meaning.c_str(), answerPos, 32, 0, BLACK);
            }
            else
            {
                gpShowAns.Draw();
            }

            if (currentPage > 0)
                gpPrevious.Draw();
            else
            {
                gpPreviousFade.Draw();
            }

            string pageIndex = to_string(currentPage + 1) + "/" + to_string(dataSize + 1);
            Vector2 pageIndexPos = GetCenteredTextPos(InterRegular, pageIndex, 20, screenCenterPos, 660);
            DrawTextEx(InterRegular, pageIndex.c_str(), Vector2{pageIndexPos.x + 4, pageIndexPos.y}, 20, 0, Color{88, 99, 128, 255});

            if (imageLoaded)
            {
                // show the deks's word at the top of the screen
                Vector2 textPos = GetCenteredTextPos(InterSemiBold, wordDesk, 36, screenCenterPos, 80 + 6);
                DrawTextEx(InterSemiBold, wordDesk.c_str(), textPos, 36, 0, BLACK);

                // Show the desk's image
                Rectangle imageRec = {0, 0, (float)wordImage.width, (float)wordImage.height};
                Vector2 imageCenter = {wordImage.width / 2.0f, wordImage.height / 2.0f};
                DrawTexturePro(wordImage, imageRec, (Rectangle){screenCenterPos.x, 328, imageRec.width, imageRec.height}, imageCenter, 0, WHITE);
            }
        }
        EndDrawing();
    }

    CloseWindow();
    return 0;
}