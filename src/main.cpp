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

using namespace rapidjson;
using namespace std;


enum WindowState
{
    HOME_WINDOW,
    START_WINDOW,
    GAMEPLAY_WINDOW,
    BROWSER_WINDOW,
    ADD_WINDOW
};
WindowState currentWindow = HOME_WINDOW;

int currentPage = 0;
int currentDesk = 0;
bool imageLoaded = false;
bool showAnswer = false;
bool isShuffled = false;
bool challengeMode = false;
Texture2D wordImage;

bool firstStart = true;
vector<bool> approved;
bool endgame = false;
bool allApproved = false;

// datavalues
string wordDesk;
string meaning;
string imgPath;

// time value
int countdownTime = 30;
bool countdownStarted = false;
int startTime = 0;
bool timeOut = false;

void approveEdit(int deskIndex, int pageIndex, bool boolSet)
{
    // Load the JSON document
    ifstream file("resources/data.json");
    stringstream buffer;
    buffer << file.rdbuf();
    string jsonStr = buffer.str();
    file.close();

    Document document;
    document.Parse(jsonStr.c_str());

    // Modify the specified value
    if (document[deskIndex]["data"][pageIndex].HasMember("approved") && document[deskIndex]["data"][pageIndex]["approved"].IsBool())
    {
        document[deskIndex]["data"][pageIndex]["approved"].SetBool(boolSet);
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
    return;
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
void DrawCountdown(int startTime, int countdownTime, Font font, Vector2 position, int fontSize, Color color, bool &timeOut)
{
    int currentTime = static_cast<int>(GetTime());
    int elapsedTime = currentTime - startTime;
    int remainingTime = countdownTime - elapsedTime;

    if (remainingTime < 0)
    {
        remainingTime = 0;
    }
    std::string timeStr = std::to_string(remainingTime);
    DrawTextEx(font, timeStr.c_str(), position, fontSize, 0, color);
    if (remainingTime == 0)
    {
        timeOut = true;
    }
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
    Font Upperclock = LoadFont("resources/UpperClockVariable.ttf");

    // Smooth the font
    SetTextureFilter(InterSemiBold.texture, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(InterMedium.texture, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(InterRegular.texture, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(InterLight.texture, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(Upperclock.texture, TEXTURE_FILTER_BILINEAR);

    // homepage
    Button topbar{"img/homepage/topbar.png", {0, 0}, 1};
    // Button newDesk{"img/homepage/new-desk.png", {48, 95}, 1};
    Button SMT{"img/homepage/SMT.png", {48, 95}, 1};
    Button foodBtn{"img/homepage/Foods.png", {278.12, 95}, 1};
    Button animalBtn{"img/homepage/animals.png", {508.24, 95}, 1};
    Button flowerBtn{"img/homepage/flowers.png", {738.36, 95}, 1};
    Button countryBtn{"img/homepage/country.png", {48, 384}, 1};
    Button jobsBtn{"img/homepage/jobs.png", {278.12, 384}, 1};
    Button vegetableBtn{"img/homepage/vegetable.png", {508.24, 384}, 1};

    // gameplay
    Button gpBG{"img/gameplay/bg.png", {48, 48}, 1};
    Button gpHome{"img/gameplay/home-btn.png", {64 + 4, 650}, 1};
    Button gpPreviousFade{"img/gameplay/previous-btn.png", {443 - 4, 649}, 1};
    Button gpPrevious{"img/gameplay/previous-btn2.png", {443 - 4, 649}, 1};
    Button gpNext{"img/gameplay/next-btn.png", {529 + 4, 649}, 1};
    Button gpShowAns{"img/gameplay/show-ans-btn.png", {809, 651}, 1};
    Button gpHideAns{"img/gameplay/hide-ans-btn.png", {809, 651}, 1};
    Button gpEasy{"img/gameplay/easy-btn.png", {340 + 4, 590}, 1};
    // Button gpMedBtn{"img/gameplay/medium-btn.png", {452 + 4, 590}, 1};
    Button gpAgain{"img/gameplay/again-btn.png", {564 + 4, 590}, 1};

    // start screen
    Button stStartBtn{"img/buttons/start.png", {415, 219}, 1};
    Button stChallengeBtn{"img/buttons/challenge.png", {415, 310}, 1};
    Button stBrowseBtn{"img/buttons/browse.png", {415, 396}, 1};
    Button stAddBtn{"img/buttons/add.png", {415, 482}, 1};
    // Load the image for the current word

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
            countdownStarted = false;
            challengeMode = false;
            topbar.Draw();
            // newDesk.Draw();
            SMT.Draw();
            foodBtn.Draw();
            animalBtn.Draw();
            flowerBtn.Draw();
            countryBtn.Draw();
            jobsBtn.Draw();
            vegetableBtn.Draw();
            if (animalBtn.isPressed(mousePosition, mousePressed))
            {
                currentWindow = START_WINDOW;
                currentDesk = 0;
            }
            else if (foodBtn.isPressed(mousePosition, mousePressed))
            {
                currentWindow = START_WINDOW;
                currentDesk = 1;
            }
            else if (flowerBtn.isPressed(mousePosition, mousePressed))
            {
                currentWindow = START_WINDOW;
                currentDesk = 2;
            }
            else if (countryBtn.isPressed(mousePosition, mousePressed))
            {
                currentWindow = START_WINDOW;
                currentDesk = 3;
            }
            else if (jobsBtn.isPressed(mousePosition, mousePressed))
            {
                currentWindow = START_WINDOW;
                currentDesk = 4;
            }
            else if (vegetableBtn.isPressed(mousePosition, mousePressed))
            {
                currentWindow = START_WINDOW;
                currentDesk = 5;
            }
        }
        else if (currentWindow == START_WINDOW)
        {
            bool isStartPressed = stStartBtn.isPressed(mousePosition, mousePressed);
            bool isChallengePressed = stChallengeBtn.isPressed(mousePosition, mousePressed);
            bool isBrowsePressed = stBrowseBtn.isPressed(mousePosition, mousePressed);
            bool isAddPressed = stAddBtn.isPressed(mousePosition, mousePressed);

            if (isStartPressed)
            {
                currentWindow = GAMEPLAY_WINDOW;
            }
            else if (isChallengePressed)
            {
                challengeMode = true;
                countdownStarted = true;
                currentWindow = GAMEPLAY_WINDOW;
                startTime = static_cast<int>(GetTime());
            }
            else if (isBrowsePressed)
            {
                currentWindow = BROWSER_WINDOW;
            }
            else if (isAddPressed)
            {
                currentWindow = ADD_WINDOW;
            }
            stStartBtn.Draw();
            stChallengeBtn.Draw();
            stBrowseBtn.Draw();
            stAddBtn.Draw();
        }
        else if (currentWindow == GAMEPLAY_WINDOW)
        {

            if (isShuffled == false)
            {
                shuffleDeck();
                document = getData();
                isShuffled = !isShuffled;
            }

            Value &currentData = document[currentDesk]["data"];
            int dataSize = currentData.Size() - 1;
            if (firstStart == true)
            {
                currentPage = 0;
                approved.clear();
                for (int i = 0; i < dataSize; i++)
                {
                    approved.push_back(false);
                }
                firstStart = false;
                allApproved = false;
            }
            else
            {
                while (approved[currentPage] == true)
                {
                    currentPage++;
                    imageLoaded = false;
                    if (currentPage >= dataSize)
                    {
                        currentPage = 0;
                        endgame = true;
                        break;
                    }
                }
            }
            if (challengeMode)
            {
                if (endgame)
                {
                    cout << "success" << endl;
                    endgame = false;
                    firstStart = true;
                    break;
                }
                if (timeOut && !endgame)
                {
                    cout << "endgameTimeout" << endl;
                    break;
                }
            }
            else
            {
                if (endgame)
                {
                    cout << "endgame" << endl;
                    endgame = false;
                    firstStart = true;
                    currentWindow = HOME_WINDOW;
                    // break;
                }
            }

            Value &currentPageData = currentData[currentPage];
            wordDesk = currentPageData["word"].GetString();
            meaning = currentPageData["meaning"].GetString();
            imgPath = currentPageData["image"].GetString();
            // cout << "Word: " << wordDesk << endl;
            // cout << "Meaning: " << meaning << endl;
            // cout << "Image Path: " << imgPath << endl;

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
            else if (gpShowAns.isPressed(mousePosition, mousePressed))
            {
                showAnswer = !showAnswer;
            }
            gpBG.Draw();
            gpHome.Draw();
            gpShowAns.Draw();
            // gpNext.Draw();

            // time challange mode
            if (challengeMode)
            {
                DrawCountdown(startTime, countdownTime, Upperclock, Vector2{880, 71}, 64, BLACK, timeOut);
            }

            if (showAnswer)
            {
                gpEasy.Draw();
                // gpMedBtn.Draw();
                gpAgain.Draw();
                gpHideAns.Draw();

                // Show answer text
                Vector2 answerPos = GetCenteredTextPos(InterSemiBold, meaning, 32, screenCenterPos, 529 + 6);
                DrawTextEx(InterMedium, meaning.c_str(), answerPos, 32, 0, BLACK);
                if (gpEasy.isPressed(mousePosition, mousePressed))
                {
                    approved[currentPage] = true;
                    currentPage++;
                    UnloadTexture(wordImage);
                    imageLoaded = false;
                    showAnswer = false;
                }
                else if (gpAgain.isPressed(mousePosition, mousePressed))
                {
                    currentPage++;
                    UnloadTexture(wordImage);
                    imageLoaded = false;
                    showAnswer = false;
                }
            }
            else
            {
                gpShowAns.Draw();
            }

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

            if (currentPage > dataSize)
            {
                currentPage = 0;
            }

            // string pageIndex = to_string(currentPage + 1) + "/" + to_string(dataSize + 1);
            // Vector2 pageIndexPos = GetCenteredTextPos(Upperclock, pageIndex, 56, screenCenterPos, 660);
            // DrawTextEx(Upperclock, pageIndex.c_str(), Vector2{pageIndexPos.x + 4, pageIndexPos.y}, 56, 0, Color{88, 99, 128, 255});
        }
        EndDrawing();
    }

    CloseWindow();
    return 0;
}