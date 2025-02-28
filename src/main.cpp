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
bool challengeMode = false;
Texture2D wordImage;

// Time Variables
int startTime = 0;
int countdownTime = 5;
bool timeOut = false;
bool countdownStarted = false;

Vector2 GetCenteredTextPos(Font font, string text, int fontSize, Vector2 screencenterPos, float yPos)
{
    Vector2 textXY = MeasureTextEx(font, text.c_str(), fontSize, 0);
    Vector2 result = {(screencenterPos.x - (textXY.x / 2.0f)), yPos};
    return result;
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

void updateJSONFile(Document &document)
{
    // Convert JSON back to string
    StringBuffer bufferOut;
    Writer<StringBuffer> writer(bufferOut);
    document.Accept(writer);

    // Write the updated JSON back to the file
    ofstream outFile("resources/data.json");
    if (!outFile)
    {
        cout << "[Error]: Cannot write to file!" << endl;
        return;
    }
    outFile << bufferOut.GetString();
    outFile.close();

    cout << "Data has been updated!" << endl;
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

    updateJSONFile(document);
}

// delete func
bool deleteWord(int deskIndex, int wordIndex)
{
    // อ่านข้อมูล JSON
    Document document = getData();

    // ตรวจสอบว่า deskIndex และ wordIndex ถูกต้องหรือไม่
    // if (deskIndex < 0 || deskIndex >= document.Size())
    if (deskIndex < 0 || deskIndex >= static_cast<int>(document.Size()))
    {
        cerr << "Error: Invalid desk index!" << endl;
        return false;
    }

    Value &deskData = document[deskIndex];
    if (!deskData.HasMember("data") || !deskData["data"].IsArray())
    {
        cerr << "Error: Desk has no data array!" << endl;
        return false;
    }

    Value &dataArray = deskData["data"];

    // if (wordIndex < 0 || wordIndex >= dataArray.Size())
    if (wordIndex < 0 || wordIndex >= static_cast<int>(dataArray.Size()))
    {
        cerr << "Error: Invalid word index!" << endl;
        return false;
    }

    // ลบรายการที่ต้องการออกจาก array
    dataArray.Erase(dataArray.Begin() + wordIndex);

    updateJSONFile(document);
    cout << "Word deleted successfully!" << endl;
    return true;
}

// change the approved status of all card to false
void resetApproved()
{
    Document document = getData();
    for (size_t i = 0; i < document.Size(); i++)
    {
        Value &obj = document[i];
        if (obj.HasMember("data"))
        {
            Value &data = obj["data"];
            for (size_t j = 0; j < data.Size(); j++)
            {
                data[j]["approved"].SetBool(false);
            }
        }
    }
    updateJSONFile(document);
}

void updateApproved(int currentDeck, int dataIndex, bool isApproved)
{
    Document document = getData();
    Value &deckData = document[currentDeck]["data"];
    int deckDataSize = deckData.Size() - 1;

    if (dataIndex < 0 || dataIndex > deckDataSize)
    {
        cout << "[Error]: Invalid data index!" << endl;
        return;
    }

    document[currentDeck]["data"][dataIndex]["approved"].SetBool(isApproved);

    updateJSONFile(document);
    return;
};

bool checkEndGame(int currentDesk = 0)
{
    // check if the game is over by checking if all the cards are approved
    Document document = getData();
    Value &currentData = document[currentDesk]["data"];
    int dataSize = currentData.Size();

    for (int i = 0; i < dataSize; i++)
    {
        if (!currentData[i]["approved"].GetBool())
        {
            return false;
        }
    }
    return true;
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
    resetApproved();
    // Initialize window
    const float screenWidth = 1000;
    const float screenHeight = 750;
    int selectedIndex = 0;

    // Center position
    Vector2 screenCenterPos = {screenWidth / 2.0f, screenHeight / 2.0f};

    InitWindow(screenWidth, screenHeight, "Die for you");
    SetTargetFPS(60);

    // icon
    Image icon = LoadImage("img/icon.png");
    SetWindowIcon(icon);

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

    float xPos = 48;
    float yPos = 95;
    float xImage = 88;
    float yImage = 120;
    oldPos oldPos = {xPos, yPos, xImage, yImage};
    vector<Button> deckButtons = {};
    vector<Button> deckCovers = {};
    vector<string> deckName = {};
    for (SizeType i = 0; i < document.Size(); i++)
    {
        Value &obj = document[i];
        if (obj.HasMember("cover"))
        {
            string coverPath = obj["cover"].GetString();
            deckName.push_back(document[i]["deck"].GetString());
            deckButtons.push_back(Button("img/homepage/card-template.png", {xPos, yPos}, 1));
            deckCovers.push_back(Button(coverPath.c_str(), {xImage, yImage}, {144, 144}));

            xPos += 230;
            xImage += 230;

            if ((i + 1) % 8 == 0)
            {
                xPos = oldPos.xPos;
                xImage = oldPos.xImage;
            }

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
    Button gpBG{"img/gameplay/bg.png", {43, 48}};
    Button gpHome{"img/start/home-btn.png", {83, 637}};
    Button gpPreviousFade{"img/gameplay/previous-btn.png", {443 - 4, 649}};
    Button gpPrevious{"img/gameplay/previous-btn2.png", {443 - 4, 649}};
    Button gpNext{"img/gameplay/next-btn.png", {529 + 4, 649}};
    Button gpShowAns{"img/gameplay/show-ans-btn.png", {809, 651}};
    Button gpHideAns{"img/gameplay/hide-ans-btn.png", {809, 651}};
    Button gpEasyBtn{"img/gameplay/easy-btn.png", {340 + 4, 590}};
    Button gpMedBtn{"img/gameplay/medium-btn.png", {452 + 4, 590}};
    Button gpHardBtn{"img/gameplay/hard-btn.png", {564 + 4, 590}};

    // start screen
    Button stStartBtn{"img/start/start-btn.png", {537, 245}};
    Button stChallengeBtn{"img/start/challenge-btn.png", {537, 326}};
    Button stBrowseBtn{"img/start/browse-btn.png", {692, 412}};
    Button stAddBtn{"img/start/add-btn.png", {537, 412}};
    Button startDeleteBtn{"img/start/delete-btn.png", {808, 637}};

    // browse
    Button browseBackBtn{"img/buttons/back.png", {50, 50}};
    Button browseDeleteBtn{"img/start/delete-btn.png", {770, 50}};
    Button browseEditBtn{"img/buttons/edit2.png", {880, 50}};

    // endgame
    Button endHomeBtn{"img/buttons/home.png", {522, 593}};
    Button endRestartBtn{"img/buttons/retry2.png", {378, 593}};
    Button congratscat{"img/end/congrat.png", {280, 70}};
    Button timesupcat{"img/end/timeup.png", {280, 100}};

    Color backgroundColor = Color{221, 245, 253, 255};

    while (!WindowShouldClose())
    {
        Vector2 mousePosition = GetMousePosition();
        bool mousePressed = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);

        BeginDrawing();
        ClearBackground(backgroundColor);

        if (currentWindow == HOME_WINDOW)
        {
            currentPage = 0;
            imageLoaded = false;
            showAnswer = false;
            challengeMode = false;
            countdownStarted = false;
            timeOut = false;
            for (size_t i = 0; i < deckButtons.size(); i++)
            {
                deckButtons[i].Draw();
                deckCovers[i].Draw();
                // show dack's name
                Vector2 centerText = MeasureTextEx(InterMedium, deckName[i].c_str(), 24, 0);
                Vector2 textPos = {(deckCovers[i].getPosition().x + (deckCovers[i].getImageSize().width / 2.0f) - (centerText.x / 2.0f)) - 2, deckCovers[i].getPosition().y + 190 - 20};
                DrawTextEx(InterMedium, deckName[i].c_str(), textPos, 24, 0, BLACK);

                bool isClicked = deckButtons[i].isPressed(mousePosition, mousePressed) || deckCovers[i].isPressed(mousePosition, mousePressed);
                if (isClicked)
                {
                    currentWindow = START_WINDOW;
                    currentDesk = i;
                }
            }
        }
        else if (currentWindow == START_WINDOW)
        {
            bool isStartPressed = stStartBtn.isPressed(mousePosition, mousePressed);
            bool isChallengePressed = stChallengeBtn.isPressed(mousePosition, mousePressed);
            bool isBrowsePressed = stBrowseBtn.isPressed(mousePosition, mousePressed);
            bool isAddPressed = stAddBtn.isPressed(mousePosition, mousePressed);
            bool isHomePressed = gpHome.isPressed(mousePosition, mousePressed);

            gpBG.Draw();

            if (isStartPressed && document[currentDesk]["data"].Size() > 0)
            {
                currentWindow = GAMEPLAY_WINDOW;
            }
            else if (isChallengePressed && document[currentDesk]["data"].Size() > 0)
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
            else if (isHomePressed)
            {
                currentWindow = HOME_WINDOW;
            }

            // new position
            Vector2 newPosition = {173, 201};
            Vector2 originalPosition = deckCovers[currentDesk].getPosition();
            // resize
            float scale = 2; // 2เท่า
            Texture2D texture = deckCovers[currentDesk].GetTexture();
            float newWidth = texture.width * scale;
            float newHeight = texture.height * scale;
            Rectangle sourceRec = {0, 0, (float)texture.width, (float)texture.height};
            Rectangle destRec = {newPosition.x, newPosition.y, newWidth, newHeight}; // new pic
            Vector2 origin = {0, 0};
            DrawTexturePro(texture, sourceRec, destRec, origin, 0.0f, WHITE);
            deckCovers[currentDesk].SetPosition(originalPosition); // คืนค่าเดิมหน้า home

            gpHome.Draw();
            startDeleteBtn.Draw();
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

            if (checkEndGame(currentDesk))
            {
                currentWindow = ENDGAME_WINDOW;
                UnloadTexture(wordImage);
            }
            else if (challengeMode && timeOut)
            {
                currentWindow = TIMEOUT_WINDOW;
                UnloadTexture(wordImage);
            }

            Value &currentData = document[currentDesk]["data"];

            int dataSize = currentData.Size() - 1;
            bool isApproved = currentData[currentPage]["approved"].GetBool();

            while (isApproved == true && checkEndGame(currentDesk) == false)
            {
                if (currentPage < dataSize)
                {
                    currentPage++;
                }
                else
                {
                    currentPage = 0;
                }
                isApproved = currentData[currentPage]["approved"].GetBool();
            }

            string wordDesk = currentData[currentPage]["word"].GetString();
            string meaning = currentData[currentPage]["meaning"].GetString();
            string imgPath = currentData[currentPage]["image"].GetString();

            bool ishomePressed = gpHome.isPressed(mousePosition, mousePressed);
            bool isNextPressed = gpNext.isPressed(mousePosition, mousePressed);
            bool isShowAnsPressed = gpShowAns.isPressed(mousePosition, mousePressed);
            bool isPreviousPressed = (gpPrevious.isPressed(mousePosition, mousePressed) || gpPreviousFade.isPressed(mousePosition, mousePressed));
            bool isEasyPressed = gpEasyBtn.isPressed(mousePosition, mousePressed);
            bool isHardPressed = gpHardBtn.isPressed(mousePosition, mousePressed);

            if (!imageLoaded)
            {
                wordImage = LoadTexture(imgPath.c_str());
                imageLoaded = true;
                cout << imgPath << endl;
            }

            if (ishomePressed)
            {
                currentWindow = HOME_WINDOW;
                isShuffled = !isShuffled;
                UnloadTexture(wordImage);
            }
            else if (isNextPressed && currentPage < dataSize)
            {
                currentPage++;
                UnloadTexture(wordImage);
                imageLoaded = false;
                showAnswer = false;
            }
            else if (isShowAnsPressed)
            {
                showAnswer = !showAnswer;
            }
            else if (isPreviousPressed && currentPage > 0)
            {
                currentPage--;
                UnloadTexture(wordImage);
                imageLoaded = false;
                showAnswer = false;
            }
            else if (isEasyPressed)
            {
                updateApproved(currentDesk, currentPage, true);
                currentPage++;
                UnloadTexture(wordImage);
                imageLoaded = false;
                showAnswer = false;
                document = getData();
            }
            else if (isHardPressed)
            {
                updateApproved(currentDesk, currentPage, false);
                currentPage++;
                UnloadTexture(wordImage);
                imageLoaded = false;
                showAnswer = false;
                document = getData();
            }

            if (currentPage > dataSize)
            {
                currentPage = 0;
                UnloadTexture(wordImage);
                imageLoaded = false;
                showAnswer = false;
            }

            gpBG.Draw();
            gpHome.Draw();
            gpNext.Draw();
            gpShowAns.Draw();

            // time challange mode
            if (challengeMode)
            {
                DrawCountdown(startTime, countdownTime, Upperclock, Vector2{880, 71}, 64, BLACK, timeOut);
            }

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
        else if (currentWindow == ENDGAME_WINDOW)
        {
            gpBG.Draw();
            congratscat.Draw();
            endHomeBtn.Draw();
            endRestartBtn.Draw();
            bool isRestartPressed = endRestartBtn.isPressed(mousePosition, mousePressed);
            bool isHomePressed = endHomeBtn.isPressed(mousePosition, mousePressed);
            if (isHomePressed)
            {
                currentWindow = HOME_WINDOW;
            }
            else if (isRestartPressed)
            {
                currentPage = 0;
                imageLoaded = false;
                showAnswer = false;
                currentWindow = START_WINDOW;
                resetApproved();
                document = getData();
            }
            // แสดงข้อความว่าเกมจบแล้ว
            string endGameText = "Congratulations !";
            string endGameText2 = "You've memorized all the words";
            Vector2 endGameTextPos = GetCenteredTextPos(InterSemiBold, endGameText, 54, screenCenterPos, 480);
            DrawTextEx(InterSemiBold, endGameText.c_str(), endGameTextPos, 54, 0, BLACK);
            Vector2 endGameTextPos2 = GetCenteredTextPos(InterSemiBold, endGameText2, 30, screenCenterPos, 540);
            DrawTextEx(InterSemiBold, endGameText2.c_str(), endGameTextPos2, 30, 0, BLACK);
        }
        else if (currentWindow == TIMEOUT_WINDOW)
        {
            gpBG.Draw();
            timesupcat.Draw();
            endHomeBtn.Draw();
            endRestartBtn.Draw();
            bool isRestartPressed = endRestartBtn.isPressed(mousePosition, mousePressed);
            bool isHomePressed = endHomeBtn.isPressed(mousePosition, mousePressed);
            if (isHomePressed)
            {
                currentWindow = HOME_WINDOW;
            }
            else if (isRestartPressed)
            {
                currentWindow = START_WINDOW;
                resetApproved();
            }
            // แสดงข้อความว่าเกมจบแล้ว
            string endGameText = "Time's Up!";
            Vector2 endGameTextPos = GetCenteredTextPos(InterSemiBold, endGameText, 60, screenCenterPos, 500);
            DrawTextEx(InterSemiBold, endGameText.c_str(), endGameTextPos, 60, 0, BLACK);
        }
        else if (currentWindow == BROWSER_WINDOW)
        {

            browseBackBtn.Draw();
            browseEditBtn.Draw();
            browseDeleteBtn.Draw();

            gpNext.Draw();
            gpPrevious.Draw();

            const Value &deskData = document[currentDesk]["data"];
            int dataSize = deskData.Size();

            const int maxRowsPerPage = 10;
            int startIndex = currentPage * maxRowsPerPage; // เพื่อที่จะเริ่มเเต่ละหน้า
            int endIndex = min((currentPage + 1) * maxRowsPerPage, dataSize);

            // คำนวณจุดศูนย์กลางจอ
            const float screenWidth = 1000.0f;
            const float screenCenterX = screenWidth / 2.0f;

            float yOffset = 150.0f;
            float columnSpacing = 100.0f;
            float xWord = screenCenterX - columnSpacing - 150.0f;
            float xMeaning = screenCenterX + columnSpacing;

            // หัวข้อตาราง
            DrawTextEx(InterSemiBold, "Word", {xWord, yOffset - 30.0f}, 26, 0, BLACK);
            DrawTextEx(InterSemiBold, "Meaning", {xMeaning, yOffset - 30.0f}, 26, 0, BLACK);

            // บิวเขียนเซ็ตปุ่มต่อตรงนี้
            if (browseBackBtn.isPressed(mousePosition, mousePressed))
            {
                currentWindow = START_WINDOW;
            }

            // ทำDelete

            if (browseDeleteBtn.isPressed(mousePosition, mousePressed) && selectedIndex != -1)
            {
                // คำนวณ index จริงในข้อมูล โดยนำ selectedIndex มารวมกับ startIndex
                int actualIndex = startIndex + (selectedIndex - startIndex);

                // เรียกใช้ฟังก์ชัน deleteWord เพื่อลบรายการที่เลือก
                if (deleteWord(currentDesk, actualIndex))
                {
                    // อัปเดตข้อมูลหลังจากลบ
                    document = getData();

                    // ตรวจสอบว่าหลังจากลบแล้ว หน้าปัจจุบันยังมีข้อมูลหรือไม่
                    int newDataSize = document[currentDesk]["data"].Size();
                    if (startIndex >= newDataSize && currentPage > 0)
                    {
                        // ถ้าหน้าปัจจุบันไม่มีข้อมูลแล้ว ให้ย้อนกลับไปหน้าก่อนหน้า
                        currentPage--;
                    }

                    selectedIndex = -1; // รีเซ็ตการเลือก
                    continue;
                }
            }

            for (int i = startIndex; i < endIndex; ++i)
            {
                string word = deskData[i]["word"].GetString();
                string meaning = deskData[i]["meaning"].GetString();

                float rowWidth = screenWidth * 0.6f;
                Rectangle rowBox = {screenCenterX - rowWidth / 2, yOffset, rowWidth, 30.0f};

                // Highlight เเถวที่กดก่อนที่จะพิมพ์ตัวหนังสือออกมานะ ไม่งั้นมันจะทับกัน
                if (i == selectedIndex)
                {
                    DrawRectangleRec(rowBox, Color{255, 225, 230, 255});
                }

                // แสดง Word
                DrawTextEx(InterRegular, word.c_str(), {xWord, yOffset}, 24, 0, DARKGRAY);

                // แสดง Meaning
                DrawTextEx(InterRegular, meaning.c_str(), {xMeaning, yOffset}, 24, 0, DARKGRAY);

                // เช็คที่คลิก
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(GetMousePosition(), rowBox))
                {
                    selectedIndex = i; // ตั้งค่าแถวที่ถูกเลือก
                }

                yOffset += 50.0f;
            }

            // อันนี้ปุ่มเปลี่ยนหน้าเฉยๆ
            if (gpNext.isPressed(mousePosition, mousePressed) && currentPage < dataSize / 10)
            {
                currentPage++;
            }
            else if ((gpPrevious.isPressed(mousePosition, mousePressed) || gpPreviousFade.isPressed(mousePosition, mousePressed)) && currentPage > 0)
            {
                currentPage--;
            }

            // แสดงหมายเลขหน้าปัจจุบัน ไม่รู้จะเอาอยู่มั้ย เเต่ในเกมเพลย์พวกเราตัดออก
            string pageIndicator = "Page " + to_string(currentPage + 1) + " / " + to_string((dataSize / maxRowsPerPage) + 1);
            Vector2 pageIndicatorPos = GetCenteredTextPos(InterRegular, pageIndicator, 20, {screenCenterX, 700}, 700);
            DrawTextEx(InterRegular, pageIndicator.c_str(), pageIndicatorPos, 20, 0, Color{88, 99, 128, 255});
        }
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
