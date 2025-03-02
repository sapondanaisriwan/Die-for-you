#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm> // For shuffle
#include <random>    // For random generator
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "type.hpp"
#include "button.hpp"

using namespace rapidjson;
using namespace std;

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

bool checkEndGame(int currentDeck = 0)
{
    // check if the game is over by checking if all the cards are approved
    Document document = getData();
    Value &currentData = document[currentDeck]["data"];
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

void addDeck(string deckName, string coverPath)
{
    Document document = getData();
    SizeType id = document.Size() + 1;

    Document::AllocatorType &allocator = document.GetAllocator();
    Value newDeck(kObjectType);
    Value data(kArrayType);
    Value name, imagePath;
    name.SetString(deckName.c_str(), allocator);
    imagePath.SetString(coverPath.c_str(), allocator);

    newDeck.AddMember("id", id, allocator);
    newDeck.AddMember("deck", name, allocator);
    newDeck.AddMember("cover", imagePath, allocator);
    newDeck.AddMember("data", data, allocator);
    document.PushBack(newDeck, allocator);

    updateJSONFile(document);
}

void removeDeck(int deckId)
{

    Document document = getData();
    if (deckId > static_cast<int>(document.Size()))
    {
        cout << "[DEBUG]: deck data > document" << endl;
        return;
    }
    document.Erase(document.Begin() + deckId);
    updateJSONFile(document);
}

void addDeckData(int deckId, string word, string meaning, string image)
{
    Document document = getData();
    Document::AllocatorType &allocator = document.GetAllocator();
    if (!document[deckId].HasMember("data"))
    {
        cout << "[DEBUG]: Impossible 💀" << endl;
        return;
    }

    Value &oldDataArr = document[deckId]["data"];
    Value newDataArr(kObjectType);

    Value newWord, newMeaning, newImage;
    newWord.SetString(word.c_str(), allocator);
    newMeaning.SetString(meaning.c_str(), allocator);
    newImage.SetString(image.c_str(), allocator);

    newDataArr.AddMember("approved", false, allocator);
    newDataArr.AddMember("word", newWord, allocator);
    newDataArr.AddMember("meaning", newMeaning, allocator);
    newDataArr.AddMember("image", newImage, allocator);
    oldDataArr.PushBack(newDataArr, allocator);

    updateJSONFile(document);
}

void editDeckData(int deckId, int dataIndex, string word, string meaning, string image)
{
    Document document = getData();
    Document::AllocatorType &allocator = document.GetAllocator();

    if (dataIndex > static_cast<int>(document[deckId]["data"].Size()))
    {
        cout << "[DEBUG]: Impossible 💀" << endl;
        return;
    }

    Value &data = document[deckId]["data"][dataIndex];
    data["word"].SetString(word.c_str(), allocator);
    data["meaning"].SetString(meaning.c_str(), allocator);
    data["image"].SetString(image.c_str(), allocator);

    updateJSONFile(document);
}

void dynamicDeck(vector<Button> &deckButtons, vector<Button> &deckCovers, vector<string> &deckName)
{
    deckName.clear();
    deckButtons.clear();
    deckCovers.clear();
    float xPos = 48;
    float yPos = 95;
    float xImage = 88;
    float yImage = 120;
    oldPos oldPos = {xPos, yPos, xImage, yImage};
    Document document = getData();
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
}

string TruncateText(const string &text, Font font, float fontSize, float maxWidth)
{
    if (MeasureTextEx(font, text.c_str(), fontSize, 0).x <= maxWidth)
    {
        return text; // Return original text if it fits
    }

    string truncated = text;
    while (!truncated.empty() && MeasureTextEx(font, (truncated + "...").c_str(), fontSize, 0).x > maxWidth)
    {
        truncated.pop_back(); // Remove last character
    }

    return truncated + "..."; // Append ellipsis only if truncated
}