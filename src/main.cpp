#include "raylib.h"

Color purple600 = {119, 49, 216, 255};
Color purple500 = {101, 20, 213, 255};

class Button
{
public:
    Rectangle rect;
    Color color;
    bool clicked;

    Button(Rectangle rect, Color color)
    {
        this->rect = rect;
        this->clicked = false;
        this->color = color;
    }

    void draw()
    {
        DrawRectangleRec(rect, color);
    }

    void draw2()
    {
        DrawRectangleRounded(rect, 0.3, 0, color);
    }

    void drawText(const char *text, int fontSize)
    {
        // Center text on button rectangle using text width and height
        int textWidth = MeasureText(text, fontSize);
        int textHeight = fontSize;

        float xCentered = rect.x + (rect.width - textWidth) / 2;
        float yCentered = rect.y + (rect.height - textHeight) / 2;
        DrawText(text, xCentered, yCentered, fontSize, WHITE);
    }

    void update()
    {
        if (CheckCollisionPointRec(GetMousePosition(), rect))
        {
            color = purple500;
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
            {
                clicked = true;
            }
        }
        else
        {
            color = purple600;
        }
    }
};

// Button deckBtn = Button({259, 100, 150, 50}, purple600);
// Button addBtn = Button({425, 100, 150, 50}, purple600);
// Button browseBtn = Button({591, 100, 150, 50}, purple600);

// deckBtn.draw2();
// deckBtn.drawText("H", 20);
// deckBtn.update();

// addBtn.draw2();
// addBtn.drawText("Add", 20);
// addBtn.update();

// browseBtn.draw2();
// browseBtn.drawText("Browse", 20);
// browseBtn.update();

int main()
{
    // Initialize window
    const float screenWidth = 1000;
    const float screenHeight = 750;

    InitWindow(screenWidth, screenHeight, "Die for you");
    SetTargetFPS(60);

    Texture2D textures[] = {
        LoadTexture("img/firstpage/topbar.png"),
        LoadTexture("img/firstpage/new-desk.png"),
        LoadTexture("img/firstpage/Foods.png"),
        LoadTexture("img/firstpage/animals.png"),
        LoadTexture("img/firstpage/flowers.png"),
        LoadTexture("img/firstpage/country.png"),
        LoadTexture("img/firstpage/jobs.png"),
        LoadTexture("img/firstpage/vegetable.png"),
    };

    Vector2 positions[] = {
        {0, 0}, {48, 95}, {278.12, 95}, {508.24, 95}, {738.36, 95}, {48, 384}, {278.12, 384}, {508.24, 384}};

    while (!WindowShouldClose())
    {
        // Start drawing
        BeginDrawing();
        ClearBackground(RAYWHITE);
        for (int i = 0; i < 8; i++)
        {
            DrawTexture(textures[i], positions[i].x, positions[i].y, WHITE);
        }
        EndDrawing();
    }
    for (int i = 0; i < 8; i++)
    {
        UnloadTexture(textures[i]);
    }

    CloseWindow();
    return 0;
}
