#pragma once
#include <raylib.h>

struct ImageSize
{
  float width;
  float height;
};

struct TextureSize
{
  int width;
  int height;
};

class Button
{
public:
  Button(const char *imagePath, Vector2 imagePosition, float scale = 1);
  Button(const char *imagePath, Vector2 imagePosition, ImageSize imageSize);
  ~Button();
  void Draw();
  void Draw2();

  Vector2 getPosition();
  TextureSize getImageSize();
  bool isPressed(Vector2 mousePos, bool mousePressed);

  // set new position
  void SetPosition(Vector2 newPosition);

  // เก็บข้อมูลของภาพที่โหลดมา
  Texture2D GetTexture();

  // Disable copy constructor and copy assignment
  Button(const Button &) = delete;
  Button &operator=(const Button &) = delete;

  // Enable move constructor and move assignment
  Button(Button &&other) noexcept;
  Button &operator=(Button &&other) noexcept;

private:
  Texture2D texture;
  Vector2 position;
  float scaleX = 1.0f; // Scale factor for width
  float scaleY = 1.0f; // Scale factor for height
};