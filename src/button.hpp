#pragma once
#include <raylib.h>

struct ImageSize
{
  float width;
  float height;
};

class Button
{
public:
  Button(const char *imagePath, Vector2 imagePosition, float scale = 1);
  Button(const char *imagePath, Vector2 imagePosition, ImageSize imageSize);
  ~Button();
  void Draw();
  void Draw2();
  bool isPressed(Vector2 mousePos, bool mousePressed);

  // Disable copy constructor and copy assignment
  Button(const Button &) = delete;
  Button &operator=(const Button &) = delete;

  // Enable move constructor and move assignment
  Button(Button &&other) noexcept;
  Button &operator=(Button &&other) noexcept;

private:
  Texture2D texture;
  Vector2 position;
};