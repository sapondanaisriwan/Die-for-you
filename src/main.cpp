#include "raylib.h"
#include "func.hpp"

WindowState currentWindow = HOME_WINDOW;

int currentPage = 0;
int currentDeck = 0;
int currentHomepage = 0;
int currentEditPage = 0;
bool imageLoaded = false;
bool showAnswer = false;
bool isShuffled = false;
bool challengeMode = false;
bool isDeckHovered = false;
Texture2D wordImage;

// Time Variables
int startTime = 0;
int countdownTime = 60;
bool timeOut = false;
bool countdownStarted = false;

int CARDS_PER_PAGE = 8;
int CARDS_PER_ROW = 4;
vector<Button> deckButtons = {};
vector<Button> deckCovers = {};
vector<string> deckName = {};

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
    string timeStr = to_string(remainingTime);
    DrawTextEx(font, timeStr.c_str(), position, fontSize, 0, color);
    if (remainingTime == 0)
    {
        timeOut = true;
    }
}

// ADD & EDIT
int GetCursorPosition(Font font, Rectangle textBox, vector<char> text)
{
    int i = 0;
    int textLengthToIndex, currentCharLength;
    while (i < (int)text.size())
    {
        textLengthToIndex = MeasureTextEx(font, TextSubtext(&text[0], 0, i), 20, 0).x;
        currentCharLength = MeasureTextEx(font, TextSubtext(&text[0], i, 1), 20, 0).x;

        // less than half of current character
        if (GetMousePosition().x <= textBox.x + textLengthToIndex + currentCharLength / 2)
        {
            break;
        }

        // greater than half of current character
        if (GetMousePosition().x <= textBox.x + textLengthToIndex + currentCharLength)
        {
            break;
        }

        i++;
    }
    return i;
}

void deleteSelectedText(vector<char> &text, int &firstIndex, int &lastIndex)
{
    if (firstIndex > lastIndex)
        swap(firstIndex, lastIndex);
    text.erase(text.begin() + firstIndex, text.begin() + lastIndex);
}

void copySelectedText(vector<char> text, int firstIndex, int lastIndex)
{
    if (firstIndex > lastIndex)
        swap(firstIndex, lastIndex);
    vector<char> cpy(text.begin() + firstIndex, text.begin() + lastIndex);
    cpy.push_back('\0');
    SetClipboardText(&cpy[0]);
}

void drawDeckPage(Font InterMedium, Vector2 mousePosition, bool mousePressed)
{
    // int totalPages = (deckButtons.size() + CARDS_PER_PAGE - 1) / CARDS_PER_PAGE;
    int startIdx = currentHomepage * CARDS_PER_PAGE;
    int endIdx = min(startIdx + CARDS_PER_PAGE, (int)deckButtons.size());
    float xPos = 48, yPos = 95;
    float xImage = 88, yImage = 120;
    isDeckHovered = false;
    for (int i = startIdx; i < endIdx; i++)
    {

        int indexInPage = i % CARDS_PER_PAGE;
        int row = indexInPage / CARDS_PER_ROW;
        int col = indexInPage % CARDS_PER_ROW;

        float adjustedXPos = xPos + (col * 230);
        float adjustedYPos = yPos + (row * 289);
        float adjustedXImage = xImage + (col * 230);
        float adjustedYImage = yImage + (row * 287);

        deckButtons[i].SetPosition({adjustedXPos, adjustedYPos});
        deckCovers[i].SetPosition({adjustedXImage, adjustedYImage});

        deckButtons[i].Draw();
        deckCovers[i].Draw();

        if (deckButtons[i].isHovered(mousePosition) || deckCovers[i].isHovered(mousePosition))
        {
            isDeckHovered = true;
        }

        string displayedWord = TruncateText(deckName[i].c_str(), InterMedium, 24, 150);
        Vector2 centerText = MeasureTextEx(InterMedium, displayedWord.c_str(), 24, 0);
        Vector2 textPos = {(deckCovers[i].getPosition().x + (deckCovers[i].getImageSize().width / 2.0f) - (centerText.x / 2.0f)) - 2, deckCovers[i].getPosition().y + 190 - 20};

        DrawTextEx(InterMedium, displayedWord.c_str(), textPos, 24, 0, BLACK);

        bool isClicked = deckButtons[i].isPressed(mousePosition, mousePressed) || deckCovers[i].isPressed(mousePosition, mousePressed);
        if (isClicked)
        {
            currentWindow = START_WINDOW;
            currentDeck = i; // Keep `i` to track the correct deck
            currentEditPage = 0;
        }
    }
}

void handlePageNavigation(Font InterRegular, Vector2 mousePosition, bool mousePressed, Button &brPrevious, Button &brNext, Button &brPreviousFadeLeft, Button &brNextFade)
{
    int totalPages = (deckButtons.size() + CARDS_PER_PAGE - 1) / CARDS_PER_PAGE;
    bool isPrevPressed = brPrevious.isPressed(mousePosition, mousePressed);
    bool isNextPressed = brNext.isPressed(mousePosition, mousePressed);

    if (totalPages > 1)
    {

        if (currentHomepage < totalPages - 1)
            brNext.Draw();
        else
            brNextFade.Draw();

        if (currentHomepage == 0)
            brPreviousFadeLeft.Draw();

        string pageIndicator = to_string(currentHomepage + 1) + " / " + to_string(totalPages);
        Vector2 pageIndicatorPos = GetCenteredTextPos(InterRegular, pageIndicator, 20, {500 + 5, 660}, 660);
        DrawTextEx(InterRegular, pageIndicator.c_str(), pageIndicatorPos, 20, 0, Color{88, 99, 128, 255});
    }
    if (currentHomepage > 0)
        brPrevious.Draw();
    // else
    //     brPreviousFadeLeft.Draw();
    // Previous Page
    if (currentHomepage > 0 && isPrevPressed)
    {
        currentHomepage--;
    }

    // Next Page
    if (currentHomepage < totalPages - 1 && isNextPressed)
    {
        currentHomepage++;
    }
}

void updateCursor(bool isHovered)
{
    if (isHovered)
        SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
    else
        SetMouseCursor(MOUSE_CURSOR_DEFAULT);
}

// MAIN
int main()
{

    // Get the JSON document
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

    // homepage
    Button hpTitle{"img/homepage/mydecks.png", {10, 20}};
    Button createDeck{"img/buttons/create2.png", {245, 35}};

    // gameplay
    Button gpBG{"img/gameplay/bg.png", {43, 48}};
    Button gpBack{"img/buttons/back.png", {80, 637}};
    Button gpShowAns{"img/gameplay/show-ans-btn.png", {795, 640}};
    Button gpHideAns{"img/gameplay/hide-ans-btn.png", {795, 640}};
    Button gpEasyBtn{"img/gameplay/easy-btn.png", {397, 590}};
    Button gpHardBtn{"img/gameplay/hard-btn.png", {507, 590}};

    // start screen
    Button stStartBtn{"img/start/start-btn.png", {537, 245}};
    Button stHome{"img/buttons/home.png", {80, 637}};
    Button stChallengeBtn{"img/start/challenge-btn.png", {537, 326}};
    Button stBrowseBtn{"img/start/browse-btn.png", {692, 412}};
    Button stAddBtn{"img/start/add-btn.png", {537, 412}};
    Button startDeleteBtn{"img/start/delete-btn.png", {808, 637}};

    // browse
    Button browseBackBtn{"img/buttons/back.png", {65, 650}};
    Button browseDeleteBtn{"img/start/delete-btn.png", {725, 650}};
    Button browseEditBtn{"img/buttons/edit.png", {835, 650}};
    Button brPreviousFadeLeft{"img/gameplay/previous-btn.png", {443 - 4, 650}};
    Button brPrevious{"img/gameplay/previous-btn2.png", {443 - 4, 650}};
    Button brNext{"img/gameplay/next-btn.png", {529 + 4, 650}};
    Button brNextFade{"img/gameplay/next-btn-fade.png", {529 + 4, 650}};
    Button browseBG{"img/gameplay/bg.png", {43, 48}};
    Button browseTitle{"img/browse/word-meaning.png", {70, 60}};

    // add & edit
    Rectangle wordBox = {100, 100, 800, 30};
    Rectangle imageBox = {100, 200, 800, 300};
    Rectangle meaningBox = {100, 550, 800, 30};
    Button editSaveBtn{"img/buttons/save.png", {screenWidth - 200, screenHeight - 100}};
    Button editBackBtn{"img/buttons/back.png", {100, screenHeight - 100}};
    Button imageDeleteBtn{"img/buttons/delete.png", {imageBox.x + imageBox.width - 110, imageBox.y + 10}};

    vector<char> word, meaning;

    FilePathList droppedImages;
    Image img;
    Texture2D txt;
    string imgPath = "";
    // char *imgPath;

    bool mouseOnWordBox = false;
    bool clickOnWordBox = false;
    bool mouseOnMeaningBox = false;
    bool clickOnMeaningBox = false;
    bool select = false;
    bool selectKeyPressed = false;
    bool reachMaxInput = false;
    bool isImageLoad = false;
    bool isCoverLoad = false;

    int framesCounter = 0;
    int framesDelete = 0;
    int letterCount = 0;
    int index = 0;
    int firstSelectedIndex = 0;
    int lastSeclectedIndex = 0;
    int editCardIndex;

    // endgame
    Button endHomeBtn{"img/buttons/home.png", {522, 593}};
    Button endRestartBtn{"img/buttons/retry2.png", {378, 593}};
    Button congratscat{"img/end/congrat.png", {60, 70}};
    Button timesupcat{"img/end/timeup.png", {65, 95}};

    Color backgroundColor = Color{221, 245, 253, 255};

    dynamicDeck(deckButtons, deckCovers, deckName);
    while (!WindowShouldClose())
    {
        Vector2 mousePosition = GetMousePosition();
        bool mousePressed = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);

        if (currentWindow != HOME_WINDOW)
            isDeckHovered = false;

        bool isBtnHovered =
            isDeckHovered || gpBack.isHovered(mousePosition) ||
            gpShowAns.isHovered(mousePosition) ||
            gpHideAns.isHovered(mousePosition) ||
            gpEasyBtn.isHovered(mousePosition) ||
            gpHardBtn.isHovered(mousePosition) ||
            createDeck.isHovered(mousePosition) ||
            browseBackBtn.isHovered(mousePosition) ||
            browseDeleteBtn.isHovered(mousePosition) ||
            browseEditBtn.isHovered(mousePosition) ||
            brPrevious.isHovered(mousePosition) ||
            brNext.isHovered(mousePosition) ||
            editSaveBtn.isHovered(mousePosition) ||
            editBackBtn.isHovered(mousePosition) ||
            imageDeleteBtn.isHovered(mousePosition) ||
            stStartBtn.isHovered(mousePosition) ||
            stChallengeBtn.isHovered(mousePosition) ||
            stBrowseBtn.isHovered(mousePosition) ||
            stAddBtn.isHovered(mousePosition) ||
            stHome.isHovered(mousePosition) ||
            startDeleteBtn.isHovered(mousePosition);

        if (currentWindow == ADD_WINDOW || currentWindow == EDIT_WINDOW || currentWindow == ADD_DECK_WINDOW)
        {
            Font editTextFont = InterRegular;

            // wordBox Check
            if (CheckCollisionPointRec(GetMousePosition(), wordBox))
            {
                mouseOnWordBox = true;
                if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
                    clickOnWordBox = true;
            }
            else
            {
                mouseOnWordBox = false;
                if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
                {
                    clickOnWordBox = false;
                }
            }

            // meaningBox Check
            if (CheckCollisionPointRec(GetMousePosition(), meaningBox))
            {
                mouseOnMeaningBox = true;
                if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
                    clickOnMeaningBox = true;
            }
            else
            {
                mouseOnMeaningBox = false;
                if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
                {
                    clickOnMeaningBox = false;
                }
            }

            // editSave
            if (editSaveBtn.isPressed(mousePosition, mousePressed))
            {
                if (currentWindow == ADD_WINDOW)
                {
                    word.push_back('\0');
                    string saveWord(word.begin(), word.end()); // word
                    word.pop_back();

                    meaning.push_back('\0');
                    string saveMeaning(meaning.begin(), meaning.end()); // meaning
                    meaning.pop_back();

                    // save code
                    // imgPath for Image
                    addDeckData(currentDeck, saveWord, saveMeaning, imgPath);
                    document = getData();
                }

                if (currentWindow == EDIT_WINDOW)
                {
                    word.push_back('\0');
                    string saveWord(word.begin(), word.end()); // word
                    word.pop_back();

                    meaning.push_back('\0');
                    string saveMeaning(meaning.begin(), meaning.end()); // meaning
                    meaning.pop_back();

                    // save code
                    // imgPath for Image
                    editDeckData(currentDeck, editCardIndex, saveWord, saveMeaning, imgPath);
                    document = getData();
                }

                if (currentWindow == ADD_DECK_WINDOW)
                {
                    word.push_back('\0');
                    string saveWord(word.begin(), word.end()); // Deck Name
                    word.pop_back();

                    // save code
                    // imgPath for Image
                    addDeck(saveWord, imgPath);
                    dynamicDeck(deckButtons, deckCovers, deckName);
                }

                word.clear();
                meaning.clear();
                if (isImageLoad)
                {
                    imgPath = "";
                    // imgPath = NULL;
                    UnloadImage(img);
                    UnloadTexture(txt);
                    isImageLoad = false;
                }

                if (currentWindow == EDIT_WINDOW)
                    currentWindow = BROWSER_WINDOW;
                if (currentWindow == ADD_DECK_WINDOW)
                    currentWindow = HOME_WINDOW;
            }

            // editBack
            if (editBackBtn.isPressed(mousePosition, mousePressed))
            {
                word.clear();
                meaning.clear();
                if (isImageLoad)
                {
                    UnloadImage(img);
                    UnloadTexture(txt);
                    isImageLoad = false;
                }

                if (currentWindow == ADD_WINDOW)
                    currentWindow = START_WINDOW;
                if (currentWindow == EDIT_WINDOW)
                    currentWindow = BROWSER_WINDOW;
                if (currentWindow == ADD_DECK_WINDOW)
                    currentWindow = HOME_WINDOW;
            }

            // imageBox
            if (IsFileDropped())
            {
                // dropImage
                droppedImages = LoadDroppedFiles();
                if (CheckCollisionPointRec(GetMousePosition(), imageBox))
                {
                    // drop again
                    if (isImageLoad)
                    {
                        imgPath = "";
                        // imgPath = NULL;
                        UnloadImage(img);
                        UnloadTexture(txt);
                        isImageLoad = false;
                    }
                    if (!isImageLoad)
                    {
                        // loadImage
                        imgPath = droppedImages.paths[0];
                        img = LoadImage(droppedImages.paths[0]);
                        int newWidth, newHeight;

                        // imageResize
                        if (img.height != imageBox.height - 4)
                        {
                            newHeight = imageBox.height - 4;
                            newWidth = imageBox.height / img.height * img.width;
                            ImageResize(&img, newWidth, newHeight);
                        }
                        /*if(currentWindow==ADD_DECK_WINDOW){
                            newHeight = imageBox.height - 4;
                            newWidth = imageBox.height / img.height * img.width;
                            ImageResize(&img, newWidth, newHeight);
                        }
                        if(currentWindow==ADD_WINDOW || currentWindow==EDIT_WINDOW){
                            newHeight = 300;
                            newWidth = 400;
                            ImageResize(&img, newWidth, newHeight);
                        }*/
                        txt = LoadTextureFromImage(img);
                        isImageLoad = true;
                    }
                }
                UnloadDroppedFiles(droppedImages);
            }

            // imageDeleteBotton
            if (imageDeleteBtn.isPressed(mousePosition, mousePressed))
            {
                imgPath = "";
                // imgPath = NULL;
                UnloadImage(img);
                UnloadTexture(txt);
                isImageLoad = false;
            }

            // text editor
            if (clickOnWordBox || clickOnMeaningBox)
            {
                vector<char> text;
                if (clickOnWordBox)
                    text = word;
                else
                    text = meaning;

                Rectangle editBox;
                if (clickOnWordBox)
                    editBox = wordBox;
                else
                    editBox = meaningBox;

                // MOUSE

                // Set the window's cursor to the I-Beam
                SetMouseCursor(MOUSE_CURSOR_IBEAM);

                // MOUSE_BUTTON_LEFT
                if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
                {
                    index = GetCursorPosition(editTextFont, editBox, text);
                    select = false;
                }
                if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
                {
                    index = GetCursorPosition(editTextFont, editBox, text);
                    // selected index
                    if (!select)
                        firstSelectedIndex = index;
                    lastSeclectedIndex = index;
                    select = true;
                }

                // Get char pressed (unicode character) on the queue
                int key = GetCharPressed();
                // add after cursor
                while (key > 0)
                {
                    if (select)
                    {
                        deleteSelectedText(text, firstSelectedIndex, lastSeclectedIndex);
                        index = firstSelectedIndex;
                    }

                    // check reachMaxInput
                    text.push_back('\0');
                    if (MeasureTextEx(editTextFont, &text[0], 20, 0).x > 780)
                        reachMaxInput = true;
                    else
                        reachMaxInput = false;
                    text.pop_back();

                    if (!reachMaxInput)
                    {
                        text.insert(text.begin() + index, key);
                        index++;
                    }
                    select = false;
                    key = GetCharPressed(); // Check next character in the queue
                }

                // BACKSPACE
                if (IsKeyDown(KEY_BACKSPACE))
                {

                    if (firstSelectedIndex != lastSeclectedIndex)
                    {
                        deleteSelectedText(text, firstSelectedIndex, lastSeclectedIndex);
                        index = firstSelectedIndex;
                        select = false;
                    }
                    else if (framesDelete == 0 || (framesDelete % 2 == 0 && framesDelete >= 40))
                    {
                        if (letterCount > 0 && index > 0)
                        {
                            text.erase(text.begin() + index - 1);
                            index--;
                        }
                    }
                    framesDelete++;
                }

                // Frames BACKSPACE
                if (IsKeyUp(KEY_BACKSPACE))
                    framesDelete = 0;

                // LEFT
                if (IsKeyPressed(KEY_LEFT))
                {
                    if (selectKeyPressed)
                    {
                        if (index > 0)
                        {
                            index--;
                            lastSeclectedIndex = index;
                        }
                    }
                    else
                    {
                        if (select)
                        {
                            if (firstSelectedIndex > lastSeclectedIndex)
                                swap(firstSelectedIndex, lastSeclectedIndex);
                            index = firstSelectedIndex;
                            select = false;
                        }
                        else
                        {
                            if (index > 0)
                                index--;
                        }
                    }
                }

                // RIGHT
                if (IsKeyPressed(KEY_RIGHT))
                {
                    if (selectKeyPressed)
                    {
                        if (index < (int)text.size())
                        {
                            index++;
                            lastSeclectedIndex = index;
                        }
                    }
                    else
                    {
                        if (select)
                        {
                            if (firstSelectedIndex > lastSeclectedIndex)
                                swap(firstSelectedIndex, lastSeclectedIndex);
                            index = lastSeclectedIndex;
                            select = false;
                        }
                        else
                        {
                            if (index < (int)text.size())
                                index++;
                        }
                    }
                }

                // UP
                if (IsKeyPressed(KEY_UP))
                {
                    index = 0;
                    if (selectKeyPressed)
                    {
                        lastSeclectedIndex = index;
                    }
                    else
                    {
                        if (select)
                        {
                            select = false;
                        }
                    }
                }

                // DOWN
                if (IsKeyPressed(KEY_DOWN))
                {
                    index = (int)text.size();
                    if (selectKeyPressed)
                    {
                        lastSeclectedIndex = index;
                    }
                    else
                    {
                        if (select)
                        {
                            select = false;
                        }
                    }
                }

                // CTRL+A
                if ((IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)) && IsKeyPressed(KEY_A))
                {
                    index = (int)text.size();
                    firstSelectedIndex = 0;
                    lastSeclectedIndex = index;
                    select = true;
                }

                // CTRL+C
                if ((IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)) && IsKeyPressed(KEY_C))
                {
                    copySelectedText(text, firstSelectedIndex, lastSeclectedIndex);
                }

                // CTRL+V
                if ((IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)) && IsKeyPressed(KEY_V))
                {
                    if (select)
                    {
                        deleteSelectedText(text, firstSelectedIndex, lastSeclectedIndex);
                        index = firstSelectedIndex;
                        letterCount = text.size();
                        select = false;
                    }

                    const char *copyText = GetClipboardText();
                    if (copyText == NULL)
                        continue;

                    for (int i = 0; copyText[i] != '\0'; i++)
                    {
                        // check reach max input
                        text.push_back('\0');
                        if (MeasureTextEx(editTextFont, &text[0], 20, 0).x > 780)
                        {
                            reachMaxInput = true;
                            break;
                        }
                        text.pop_back();

                        text.insert(text.begin() + index, copyText[i]);
                        index++;
                    }
                }

                // CTRL+X
                if ((IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)) && IsKeyPressed(KEY_X))
                {
                    copySelectedText(text, firstSelectedIndex, lastSeclectedIndex);

                    deleteSelectedText(text, firstSelectedIndex, lastSeclectedIndex);
                    index = firstSelectedIndex;
                    select = false;
                }

                // SHIFT
                if ((IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)) && !GetCharPressed())
                {
                    // selected index
                    if (!select)
                        firstSelectedIndex = index;
                    lastSeclectedIndex = index;

                    // select
                    select = true;
                    selectKeyPressed = true;
                }
                else
                {
                    selectKeyPressed = false;
                }

                // selected index
                if (!select)
                {
                    firstSelectedIndex = index;
                    lastSeclectedIndex = index;
                }

                // check reach max input
                text.push_back('\0');
                if (MeasureTextEx(editTextFont, &text[0], 20, 0).x > 780)
                    reachMaxInput = true;
                else
                    reachMaxInput = false;
                text.pop_back();

                letterCount = text.size();
                if (clickOnWordBox)
                    word = text;
                else
                    meaning = text;
            }
            else if (mouseOnWordBox || mouseOnMeaningBox)
                SetMouseCursor(MOUSE_CURSOR_IBEAM);
            else
                SetMouseCursor(MOUSE_CURSOR_DEFAULT);

            if (mouseOnWordBox || clickOnWordBox || mouseOnMeaningBox || clickOnMeaningBox)
                framesCounter++;
            else
                framesCounter = 0;
        }

        BeginDrawing();
        ClearBackground(backgroundColor);

        if (currentWindow == HOME_WINDOW)
        {
            hpTitle.Draw();
            createDeck.Draw();
            currentPage = 0;
            imageLoaded = false;
            showAnswer = false;
            challengeMode = false;
            countdownStarted = false;
            timeOut = false;
            isCoverLoad = false;
            drawDeckPage(InterMedium, mousePosition, mousePressed);
            handlePageNavigation(InterRegular, mousePosition, mousePressed, brPrevious, brNext, brPreviousFadeLeft, brNextFade);
            bool isCreateDPressed = createDeck.isPressed(mousePosition, mousePressed);
            if (isCreateDPressed)
            {
                currentWindow = ADD_DECK_WINDOW;
            }
        }
        else if (currentWindow == START_WINDOW)
        {
            bool isStartPressed = stStartBtn.isPressed(mousePosition, mousePressed);
            bool isChallengePressed = stChallengeBtn.isPressed(mousePosition, mousePressed);
            bool isBrowsePressed = stBrowseBtn.isPressed(mousePosition, mousePressed);
            bool isAddPressed = stAddBtn.isPressed(mousePosition, mousePressed);
            bool isHomePressed = stHome.isPressed(mousePosition, mousePressed);
            bool isDeletePressed = startDeleteBtn.isPressed(mousePosition, mousePressed);

            gpBG.Draw();

            if (isStartPressed)
            {
                document = getData();
                if (document[currentDeck]["data"].Size() > 0)
                {
                    currentWindow = GAMEPLAY_WINDOW;
                }
            }
            else if (isChallengePressed)
            {
                document = getData();
                if (document[currentDeck]["data"].Size() > 0)
                {
                    challengeMode = true;
                    countdownStarted = true;
                    currentWindow = GAMEPLAY_WINDOW;
                    startTime = static_cast<int>(GetTime());
                }
            }
            else if (isBrowsePressed)
            {
                document = getData();
                currentWindow = BROWSER_WINDOW;
            }
            else if (isAddPressed)
            {
                word.clear();
                meaning.clear();
                currentWindow = ADD_WINDOW;
            }
            else if (isHomePressed)
            {
                currentWindow = HOME_WINDOW;
            }
            else if (isDeletePressed)
            {
                removeDeck(currentDeck);
                dynamicDeck(deckButtons, deckCovers, deckName);
                currentWindow = HOME_WINDOW;
            }

            if (!isCoverLoad)
            {
                document = getData();
                string deckCover = document[currentDeck]["cover"].GetString();
                // ตรวจสอบว่าไฟล์ภาพสามารถโหลดได้หรือไม่
                if (FileExists(deckCover.c_str()))
                {
                    // โหลดภาพเป็น Image ก่อน
                    Image image = LoadImage(deckCover.c_str());

                    // คำนวณขนาดใหม่ตามข้อกำหนด
                    int newWidth = image.width;
                    int newHeight = image.height;

                    // คำนวณอัตราส่วนของภาพ
                    float aspectRatio = (float)image.width / (float)image.height;

                    // หากขนาดของภาพเกินขีดจำกัดที่กำหนด (สูงสุด 300px หรือกว้างสุด 400px)

                    if (image.width < 150 || image.height < 150)
                    {
                        newWidth = image.width * 2;
                        newHeight = image.height * 2;
                    }

                    if (image.height > 300 || image.width > 400)
                    {
                        if (image.width > image.height)
                        {
                            // หากความกว้างมากกว่าความสูง, ปรับขนาดตามความกว้าง
                            newWidth = 400;
                            newHeight = (int)(newWidth / aspectRatio); // คำนวณความสูงใหม่
                        }
                        else
                        {
                            // หากความสูงมากกว่าความกว้าง, ปรับขนาดตามความสูง
                            newHeight = 300;
                            newWidth = (int)(newHeight * aspectRatio); // คำนวณความกว้างใหม่
                        }
                    }

                    // รีไซซ์ภาพตามขนาดใหม่
                    ImageResize(&image, newWidth, newHeight);

                    // โหลด Texture ใหม่จาก Image ที่ถูกรีไซซ์
                    wordImage = LoadTextureFromImage(image);

                    // ปล่อยทรัพยากร Image ที่ไม่ใช้งานแล้ว
                    UnloadImage(image);
                    isCoverLoad = true; // ตั้งค่า imageLoaded เป็น true หมายความว่าได้โหลดภาพแล้ว
                }
                else
                {
                    cout << "Error: Image path does not exist!" << endl;
                }
            }

            if (isCoverLoad)
            {
                Rectangle imageRec = {0, 0, (float)wordImage.width, (float)wordImage.height};
                Vector2 imageCenter = {wordImage.width / 2.0f, wordImage.height / 2.0f};
                DrawTexturePro(wordImage, imageRec, (Rectangle){282, 330, imageRec.width, imageRec.height}, imageCenter, 0, WHITE);
            }
            stHome.Draw();
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

            if (checkEndGame(currentDeck))
            {
                currentWindow = ENDGAME_WINDOW;
                UnloadTexture(wordImage);
            }
            else if (challengeMode && timeOut)
            {
                imageLoaded = false;
                challengeMode = false;
                countdownStarted = false;
                timeOut = false;
                currentWindow = TIMEOUT_WINDOW;
                UnloadTexture(wordImage);
            }

            Value &currentData = document[currentDeck]["data"];

            int dataSize = currentData.Size() - 1;
            bool isApproved = currentData[currentPage]["approved"].GetBool();

            while (isApproved == true && checkEndGame(currentDeck) == false)
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

            bool isBackPressed = gpBack.isPressed(mousePosition, mousePressed);
            bool isShowAnsPressed = gpShowAns.isPressed(mousePosition, mousePressed);
            bool isEasyPressed = gpEasyBtn.isPressed(mousePosition, mousePressed);
            bool isHardPressed = gpHardBtn.isPressed(mousePosition, mousePressed);

            /*if (!imageLoaded && showAnswer)
            {
                wordImage = LoadTexture(imgPath.c_str());
                imageLoaded = true;
                cout << imgPath << endl;
            }
            if (!showAnswer && imageLoaded)
            {
                UnloadTexture(wordImage);
                imageLoaded = false;
            }*/

            if (!imageLoaded && showAnswer)
            {
                // ตรวจสอบว่าไฟล์ภาพสามารถโหลดได้หรือไม่
                if (FileExists(imgPath.c_str()))
                {
                    // โหลดภาพเป็น Image ก่อน
                    Image image = LoadImage(imgPath.c_str());

                    // คำนวณขนาดใหม่ตามข้อกำหนด
                    int newWidth = image.width;
                    int newHeight = image.height;

                    // คำนวณอัตราส่วนของภาพ
                    float aspectRatio = (float)image.width / (float)image.height;

                    // หากขนาดของภาพเกินขีดจำกัดที่กำหนด (สูงสุด 300px หรือกว้างสุด 400px)
                    if (image.height > 300 || image.width > 400)
                    {
                        if (image.width > image.height)
                        {
                            // หากความกว้างมากกว่าความสูง, ปรับขนาดตามความกว้าง
                            newWidth = 400;
                            newHeight = (int)(newWidth / aspectRatio); // คำนวณความสูงใหม่
                        }
                        else
                        {
                            // หากความสูงมากกว่าความกว้าง, ปรับขนาดตามความสูง
                            newHeight = 300;
                            newWidth = (int)(newHeight * aspectRatio); // คำนวณความกว้างใหม่
                        }
                    }

                    // รีไซซ์ภาพตามขนาดใหม่
                    ImageResize(&image, newWidth, newHeight);

                    // โหลด Texture ใหม่จาก Image ที่ถูกรีไซซ์
                    wordImage = LoadTextureFromImage(image);

                    // ปล่อยทรัพยากร Image ที่ไม่ใช้งานแล้ว
                    UnloadImage(image);

                    imageLoaded = true;      // ตั้งค่า imageLoaded เป็น true หมายความว่าได้โหลดภาพแล้ว
                    cout << imgPath << endl; // แสดง path ของไฟล์ที่โหลด
                }
                else
                {
                    cout << "Error: Image path does not exist!" << endl;
                }
            }

            if (!showAnswer && imageLoaded)
            {
                UnloadTexture(wordImage);
                imageLoaded = false;
            }

            if (isBackPressed)
            {
                resetApproved();
                showAnswer = false;
                UnloadTexture(wordImage);
                imageLoaded = false;
                isShuffled = !isShuffled;
                currentWindow = START_WINDOW;
            }
            else if (isShowAnsPressed)
            {
                showAnswer = !showAnswer;
            }
            else if (isEasyPressed)
            {
                updateApproved(currentDeck, currentPage, true);
                currentPage++;
                UnloadTexture(wordImage);
                imageLoaded = false;
                showAnswer = false;
                document = getData();
            }
            else if (isHardPressed)
            {
                updateApproved(currentDeck, currentPage, false);
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
            gpBack.Draw();
            gpShowAns.Draw();
            // show the deks's word at the top of the screen
            Vector2 textPos = GetCenteredTextPos(InterSemiBold, wordDesk, 36, screenCenterPos, 80 + 6);
            DrawTextEx(InterSemiBold, wordDesk.c_str(), textPos, 36, 0, BLACK);

            // time challange mode
            if (challengeMode)
            {
                DrawCountdown(startTime, countdownTime, Upperclock, Vector2{880, 71}, 64, BLACK, timeOut);
            }

            if (showAnswer)
            {
                gpEasyBtn.Draw();

                gpHardBtn.Draw();
                gpHideAns.Draw();

                // Show answer text
                Vector2 answerPos = GetCenteredTextPos(InterSemiBold, meaning, 40, screenCenterPos, 529 + 6);
                DrawTextEx(InterMedium, meaning.c_str(), answerPos, 40, 0, BLACK);
            }
            else
            {
                gpShowAns.Draw();
            }

            if (imageLoaded)
            {
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
        }
        else if (currentWindow == BROWSER_WINDOW)
        {
            browseBG.Draw();
            browseTitle.Draw();
            browseBackBtn.Draw();
            browseEditBtn.Draw();
            browseDeleteBtn.Draw();
            // brPrevious.Draw();

            const Value &deskData = document[currentDeck]["data"];
            int dataSize = deskData.Size();

            const int maxRowsPerPage = 10;
            int startIndex = currentEditPage * maxRowsPerPage; // เพื่อที่จะเริ่มเเต่ละหน้า
            int endIndex = min((currentEditPage + 1) * maxRowsPerPage, dataSize);

            // คำนวณจุดศูนย์กลางจอ
            const float screenWidth = 1000.0f;
            const float screenCenterX = screenWidth / 2.0f;

            float yOffset = 150.0f;
            float columnSpacing = 100.0f;
            float xWord = screenCenterX - columnSpacing - 150.0f;
            float xMeaning = screenCenterX + columnSpacing;

            /* หัวข้อตาราง
            DrawTextEx(InterSemiBold, "Word", {xWord, yOffset}, 26, 0, BLACK);
            DrawTextEx(InterSemiBold, "Meaning", {xMeaning, yOffset}, 26, 0, BLACK); */

            if (currentEditPage < dataSize / 10)
                brNext.Draw();
            else
                brNextFade.Draw();

            if (currentEditPage > 0)
                brPrevious.Draw();
            else
                brPreviousFadeLeft.Draw();

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
                if (deleteWord(currentDeck, actualIndex))
                {
                    // อัปเดตข้อมูลหลังจากลบ
                    document = getData();

                    // ตรวจสอบว่าหลังจากลบแล้ว หน้าปัจจุบันยังมีข้อมูลหรือไม่
                    int newDataSize = document[currentDeck]["data"].Size();
                    if (startIndex >= newDataSize && currentEditPage > 0)
                    {
                        // ถ้าหน้าปัจจุบันไม่มีข้อมูลแล้ว ให้ย้อนกลับไปหน้าก่อนหน้า
                        currentEditPage--;
                    }

                    selectedIndex = -1; // รีเซ็ตการเลือก
                    continue;
                }
            }

            if (browseEditBtn.isPressed(mousePosition, mousePressed))
            {
                string getWord, getMeaning;

                editCardIndex = selectedIndex;

                getWord = deskData[editCardIndex]["word"].GetString();
                word.clear();
                for (auto c : getWord)
                {
                    if (c == '\0')
                        break;
                    word.push_back(c);
                }

                getMeaning = deskData[editCardIndex]["meaning"].GetString();
                meaning.clear();
                for (auto c : getMeaning)
                {
                    if (c == '\0')
                        break;
                    meaning.push_back(c);
                }

                imgPath = deskData[editCardIndex]["image"].GetString();
                if (imgPath != "")
                {
                    img = LoadImage(imgPath.c_str());
                    int newWidth, newHeight;

                    // imageResize
                    if (img.height != imageBox.height - 4)
                    {
                        newHeight = imageBox.height - 4;
                        newWidth = imageBox.height / img.height * img.width;
                        ImageResize(&img, newWidth, newHeight);
                    }

                    txt = LoadTextureFromImage(img);
                    isImageLoad = true;
                }

                currentWindow = EDIT_WINDOW;
            }

            for (int i = startIndex; i < endIndex; ++i)
            {
                string browseWord = deskData[i]["word"].GetString();
                string browseMeaning = deskData[i]["meaning"].GetString();

                float rowWidth = (screenWidth * 0.85f) - 20;                                             // 600
                Rectangle rowBox = {screenCenterX - (rowWidth / 2) + 20, yOffset - 12, rowWidth, 30.0f}; // x,y,w,h

                // Highlight เเถวที่กดก่อนที่จะพิมพ์ตัวหนังสือออกมานะ ไม่งั้นมันจะทับกัน
                if (i == selectedIndex)
                {
                    DrawRectangleRec(rowBox, Color{255, 225, 230, 255});
                }

                // แสดง Word
                string displayedWord = TruncateText(browseWord, InterRegular, 24, 250);
                string displayedMeaning = TruncateText(browseMeaning, InterRegular, 24, 500);

                DrawTextEx(InterRegular, displayedWord.c_str(), {xWord - 120, yOffset - 10}, 24, 0, DARKGRAY);

                // แสดง Meaning
                DrawTextEx(InterRegular, displayedMeaning.c_str(), {xMeaning - 200, yOffset - 10}, 24, 0, DARKGRAY);

                // เช็คที่คลิก
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(GetMousePosition(), rowBox))
                {
                    selectedIndex = i; // ตั้งค่าแถวที่ถูกเลือก
                }

                yOffset += 50.0f;
            }

            // อันนี้ปุ่มเปลี่ยนหน้าเฉยๆ
            if (brNext.isPressed(mousePosition, mousePressed) && currentEditPage < dataSize / 10)
            {
                currentEditPage++;
            }
            else if ((brPrevious.isPressed(mousePosition, mousePressed) || brPreviousFadeLeft.isPressed(mousePosition, mousePressed)) && currentEditPage > 0)
            {
                currentEditPage--;
            }

            // แสดงหมายเลขหน้าปัจจุบัน ไม่รู้จะเอาอยู่มั้ย เเต่ในเกมเพลย์พวกเราตัดออก
            string pageIndicator = to_string(currentEditPage + 1) + " / " + to_string((dataSize / maxRowsPerPage) + 1);
            Vector2 pageIndicatorPos = GetCenteredTextPos(InterRegular, pageIndicator, 20, {screenCenterX + 5, 660}, 660);
            DrawTextEx(InterRegular, pageIndicator.c_str(), pageIndicatorPos, 20, 0, Color{88, 99, 128, 255});
        }
        else if (currentWindow == ADD_WINDOW || currentWindow == EDIT_WINDOW || currentWindow == ADD_DECK_WINDOW)
        {
            // editBox
            Rectangle editBox;
            if (clickOnWordBox)
                editBox = wordBox;
            if (clickOnMeaningBox)
                editBox = meaningBox;

            // text
            word.push_back('\0');
            meaning.push_back('\0');
            vector<char> text;
            if (clickOnWordBox)
                text = word;
            else
                text = meaning;

            // font
            Font editTextFont = InterRegular;
            int fontSize = 20;

            // test
            DrawTextEx(editTextFont, imgPath.c_str(), {0, 0}, fontSize + 4, 0, BLACK);

            // wordBox
            if (currentWindow != ADD_DECK_WINDOW)
                DrawTextEx(editTextFont, "Word", {(wordBox.x + 5), (wordBox.y - 25)}, fontSize + 4, 0, BLACK);
            else
                DrawTextEx(editTextFont, "Deck Name", {(wordBox.x + 5), (wordBox.y - 25)}, fontSize + 4, 0, BLACK);
            DrawRectangleRec(wordBox, WHITE);
            if (mouseOnWordBox || clickOnWordBox)
                DrawRectangleLinesEx(wordBox, 2, BLACK);
            else
                DrawRectangleLinesEx(wordBox, 2, GRAY);

            // meaningBox
            if (!(currentWindow == ADD_DECK_WINDOW))
            {
                DrawTextEx(editTextFont, "Meaning", {(meaningBox.x + 5), (meaningBox.y - 25)}, fontSize + 4, 0, BLACK);
                DrawRectangleRec(meaningBox, WHITE);
                if (mouseOnMeaningBox || clickOnMeaningBox)
                    DrawRectangleLinesEx(meaningBox, 2, BLACK);
                else
                    DrawRectangleLinesEx(meaningBox, 2, GRAY);
            }

            // imageBox
            DrawTextEx(editTextFont, "Image", {(imageBox.x + 5), (imageBox.y - 25)}, fontSize + 4, 0, BLACK);
            DrawRectangleRec(imageBox, WHITE);
            DrawRectangleLinesEx(imageBox, 2, GRAY);

            // selectedBox
            if (select)
            {
                int f = firstSelectedIndex;
                int l = lastSeclectedIndex;
                if (f > l)
                    swap(f, l);

                DrawRectangle(editBox.x + 5 + MeasureTextEx(editTextFont, TextSubtext(&text[0], 0, f + 1), fontSize, 0).x - MeasureTextEx(editTextFont, TextSubtext(&text[0], f, 1), fontSize, 0).x // first selected index position
                              ,
                              editBox.y + 2, MeasureTextEx(editTextFont, TextSubtext(&text[0], f, l - f), fontSize, 0).x // selected index length
                              ,
                              editBox.height - 4, BLUE);
            }

            // reach max input
            if (reachMaxInput)
            {
                DrawTextEx(editTextFont, "Reach Max Input", {(editBox.x + 5), (editBox.y + 32)}, fontSize, 0, MAROON);
            }

            // word
            Vector2 wordPos = {wordBox.x + 5.0f, wordBox.y + 6.0f};
            DrawTextEx(editTextFont, &word[0], wordPos, fontSize, 0, BLACK);

            // meaning
            Vector2 meaningPos = {meaningBox.x + 5.0f, meaningBox.y + 6.0f};
            if (!(currentWindow == ADD_DECK_WINDOW))
            {
                DrawTextEx(editTextFont, &meaning[0], meaningPos, fontSize, 0, BLACK);
            }

            // cursor
            if (clickOnWordBox || clickOnMeaningBox)
            {
                if (((framesCounter / 20) % 2) == 0)
                    DrawText("|", (int)editBox.x + 4 + MeasureTextEx(editTextFont, TextSubtext(&text[0], 0, index), fontSize, 0).x, (int)editBox.y + 8, fontSize, BLACK);
            }

            // image
            Vector2 centerPos;
            centerPos.x = (imageBox.x + 2) + (imageBox.width - img.width) / 2;
            centerPos.y = imageBox.y + 2;
            if (isImageLoad)
                DrawTexture(txt, centerPos.x, centerPos.y, WHITE);

            // text
            word.pop_back();
            meaning.pop_back();

            // button
            if (currentWindow == ADD_DECK_WINDOW)
            {
                if (!word.empty() && isImageLoad)
                    editSaveBtn.Draw();
            }
            else if (currentWindow == ADD_WINDOW || currentWindow == EDIT_WINDOW)
            {
                if (!word.empty() && !meaning.empty())
                    editSaveBtn.Draw();
            }
            editBackBtn.Draw();
            if (isImageLoad)
                imageDeleteBtn.Draw();

            if (word.empty() && !clickOnWordBox)
            {
                if (currentWindow != ADD_DECK_WINDOW)
                    DrawTextEx(editTextFont, "Please Input Word (At least 1 character)", wordPos, fontSize, 0, DARKGRAY);
                else
                    DrawTextEx(editTextFont, "Please Input Deck Name (At least 1 character)", wordPos, fontSize, 0, DARKGRAY);
            }
            if (meaning.empty() && !clickOnMeaningBox && !(currentWindow == ADD_DECK_WINDOW))
                DrawTextEx(editTextFont, "Please Input Meaning", meaningPos, fontSize, 0, DARKGRAY);
            if (!isImageLoad)
                DrawTextEx(editTextFont, "Drag and Drop Image here", {(imageBox.x + 5), (imageBox.y + 8)}, fontSize, 0, DARKGRAY);
        }
        updateCursor(isBtnHovered);

        // SetMouseCursor(MOUSE_CURSOR_DEFAULT);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
