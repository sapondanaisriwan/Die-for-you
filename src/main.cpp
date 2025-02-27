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


//delete func

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
    
    // แปลง JSON กลับเป็น string เพื่อบันทึกลงไฟล์
    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    document.Accept(writer);
    
    // บันทึกข้อมูลที่อัปเดตลงไฟล์
    ofstream outFile("resources/data.json");
    if (!outFile)
    {
        cerr << "Error: Cannot write to file!" << endl;
        return false;
    }
    outFile << buffer.GetString();
    outFile.close();
    
    cout << "Word deleted successfully!" << endl;
    return true;
}



int main()
{

    // Get the JSON document
    // shuffleDeck();
    Document document = getData();
    // Initialize window
    const float screenWidth = 1000;
    const float screenHeight = 750;
    int selectedIndex;

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
    Button gpEasyBtn{"img/gameplay/easy-btn.png", {340 + 4, 590}, 1};
    Button gpMedBtn{"img/gameplay/medium-btn.png", {452 + 4, 590}, 1};
    Button gpHardBtn{"img/gameplay/hard-btn.png", {564 + 4, 590}, 1};

    // start screen
    Button stStartBtn{"img/buttons/start.png", {415, 219}, 1};
    Button stChallengeBtn{"img/buttons/challenge.png", {415, 310}, 1};
    Button stBrowseBtn{"img/buttons/browse.png", {415, 396}, 1};
    Button stAddBtn{"img/buttons/add.png", {415, 482}, 1};

    //browse
    Button browseBackBtn{"img/browse/back.png", {50, 50}, 0.5};
    Button browseDeleteBtn{"img/browse/delete.png", {800, 50}, 0.5};
    Button browseEditBtn{"img/browse/edit.png", {900, 50}, 0.5};

    while(!WindowShouldClose())
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
                currentWindow = GAMEPLAY_WINDOW;
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
            Value &currentPageData = currentData[currentPage];

            int dataSize = currentData.Size() - 1;
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
        }else if(currentWindow == BROWSER_WINDOW){

            browseBackBtn.Draw();
            browseEditBtn.Draw();
            browseDeleteBtn.Draw();

            gpNext.Draw();
            gpPrevious.Draw();
            
    const Value &deskData = document[currentDesk]["data"];
    int dataSize = deskData.Size();

    const int maxRowsPerPage = 10;
    int startIndex = currentPage * maxRowsPerPage; //เพื่อที่จะเริ่มเเต่ละหน้า
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


    //บิวเขียนเซ็ตปุ่มต่อตรงนี้
    if (browseBackBtn.isPressed(mousePosition, mousePressed))
            {
                currentWindow = START_WINDOW;
            }

            //ทำDelete

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

        if(gpNext.isPressed(mousePosition, mousePressed) && currentPage < dataSize/10)
            {
                currentPage++;
               
            }
            else if ((gpPrevious.isPressed(mousePosition, mousePressed) || gpPreviousFade.isPressed(mousePosition, mousePressed)) && currentPage > 0)
            {
                currentPage--;
             
            }

        //แสดงหมายเลขหน้าปัจจุบัน ไม่รู้จะเอาอยู่มั้ย เเต่ในเกมเพลย์พวกเราตัดออก
    string pageIndicator = "Page " + to_string(currentPage + 1) + " / " + to_string((dataSize / maxRowsPerPage) + 1);
    Vector2 pageIndicatorPos = GetCenteredTextPos(InterRegular, pageIndicator, 20, {screenCenterX, 700}, 700);
    DrawTextEx(InterRegular, pageIndicator.c_str(), pageIndicatorPos, 20, 0, Color{88, 99, 128, 255});



        }
        EndDrawing();
    }

    CloseWindow();
    return 0;
}

