#include "button.hpp"

// Constructor for button with default size
Button::Button(const char *imagePath, Vector2 imagePosition, float scale)
{
  Image image = LoadImage(imagePath);

  int originalWidth = image.width;
  int originalHeight = image.height;

  int newWidth = static_cast<int>(originalWidth * scale);
  int newHeight = static_cast<int>(originalHeight * scale);

  ImageResize(&image, newWidth, newHeight);
  texture = LoadTextureFromImage(image);
  UnloadImage(image);
  position = imagePosition;
}

// Constructor for button with custom size
Button::Button(const char *imagePath, Vector2 imagePosition, ImageSize imageSize)
{
  Image image = LoadImage(imagePath);

  ImageResize(&image, imageSize.width, imageSize.height);
  texture = LoadTextureFromImage(image);
  UnloadImage(image);
  position = imagePosition;
}

Button::~Button()
{
  UnloadTexture(texture);
}

void Button::Draw()
{
  DrawTextureV(texture, position, WHITE);
}

bool Button::isHovered(Vector2 mousePos)
{
  Rectangle rect = {position.x, position.y, (float)texture.width, (float)texture.height};
  return CheckCollisionPointRec(mousePos, rect);
}

bool Button::isPressed(Vector2 mousePos, bool mousePressed)
{
  Rectangle rect = {position.x, position.y, static_cast<float>(texture.width), static_cast<float>(texture.height)};
  return CheckCollisionPointRec(mousePos, rect) && mousePressed;
}

Button::Button(Button &&other) noexcept
{
  texture = other.texture;
  position = other.position;

  // Reset other to avoid double deletion
  other.texture.id = 0;
}

Button &Button::operator=(Button &&other) noexcept
{
  if (this != &other)
  {
    UnloadTexture(texture); // Free existing texture

    texture = other.texture;
    position = other.position;

    other.texture.id = 0; // Prevent destructor from unloading the texture
  }
  return *this;
}
Vector2 Button::getPosition()
{
  return Vector2{position.x, position.y};
}

TextureSize Button::getImageSize()
{
  return {texture.width, texture.height};
}

void Button::SetPosition(Vector2 newPosition)
{
  position = newPosition;
}

Texture2D Button::GetTexture()
{
  return texture;
}