#pragma once
#include <raylib.h>

class Button
{
public:
  Button(const char *imagePath, Vector2 imagePosition, float scale);
  ~Button();
  void Draw();
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