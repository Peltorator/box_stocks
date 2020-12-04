#include <string>
#include <ctime>
#include <string_view>
#include <iostream>
#include <fstream>
#include <sstream>
#include <SFML/Graphics.hpp>

#include "shop.cpp"
#include "order.cpp"
#include "button.cpp"
#include "text_field.cpp"
#include "item_tile.cpp"
#include "box_tile.cpp"
#include "filled_box_tile.cpp"
#include "database.cpp"
#include "font.cpp"

using namespace std;

void AddTitle(sf::RenderWindow& window, const string& title, const float px = 50.f, const float py = 20.f, const int charSize = 24) {
    sf::Text text;
    text.setFont(NFont::font);
    text.setString(title);
    text.setCharacterSize(charSize);
    text.setFillColor(sf::Color::White);
    text.setPosition(px, py);
    window.draw(text);
}

uint64_t ToInt(const string& s) {
    uint64_t ans = 0;
    for (const char c : s) {
        if (c >= '0' && c <= '9') {
            ans = ans * 10LL + (c - '0');
        } else {
            return 0;
        }
    }
    return ans;
}

void UpdateItems(sf::RenderWindow& window, const vector<pair<TItem, int32_t>>& items, TDataBase& dataBase) {
    for (const auto& [item, amount] : items) {
        string updateQuery = "update Item set amount = amount + " + to_string(amount) + " where itemID = " + to_string(item.GetItemID()) + ";";
        dataBase.Query(updateQuery);
    }
}

void UpdateBoxes(sf::RenderWindow& window, const vector<pair<TBox, int32_t>>& boxes, TDataBase& dataBase) {
    for (const auto& [box, amount] : boxes) {
        string updateQuery = "update Box set available = available + " + to_string(amount) + " where boxID = " + to_string(box.GetBoxID()) + ";";
        dataBase.Query(updateQuery);
    }
}

string GetImageFromDB(const string &s) {
    string bytes;
    bytes.reserve(s.size() >> 3);
    for (size_t i = 0; i < s.size(); i += 8) {
        char c = 0;
        for (int j = i; j < i + 8; j++) {
            c = (c << 1) | (s[j] - '0');
        }
        bytes.push_back(c);
    }
    return bytes;
}

pair<vector<pair<TItem, uint32_t>>, vector<pair<TBox, uint32_t>>> GetSettings(TDataBase& dataBase) {
    string getItemsQuery = "select * from Item;";
    vector<pair<TItem, uint32_t>> items;
    auto itemsRaw = dataBase.Query(getItemsQuery);
    for (auto& dict : itemsRaw) {
        items.push_back({TItem(ToInt(dict["itemID"]), dict["itemName"], ToInt(dict["weight"]), ToInt(dict["volume"]), GetImageFromDB(dict["image"])), ToInt(dict["amount"])});
    }

    string getBoxesQuery = "select * from Box;";
    vector<pair<TBox, uint32_t>> boxes;
    auto boxesRaw = dataBase.Query(getBoxesQuery);
    for (auto& dict : boxesRaw) {
        boxes.push_back({TBox(ToInt(dict["boxID"]), dict["boxName"], ToInt(dict["maxWeight"]), ToInt(dict["maxVolume"]), ToInt(dict["cost"]), GetImageFromDB(dict["image"])), ToInt(dict["available"])});
    }

    return {items, boxes};
}

void ShowOrder(sf::RenderWindow& window, const vector<TFilledBox>& filledBoxes, const vector<TBox>& boxes, string title = "") {
    if (title.empty()) {
        title = "You finished your purchase successfully. Thank you for using our shop. Your order will come to you in the following form:";
    }
    vector<FilledBoxTile> filledBoxTiles;
    for (size_t i = 0; i < filledBoxes.size(); i++) {
        string curImage;
        for (const auto& box : boxes) {
            if (box.GetBoxID() == filledBoxes[i].GetBox().GetBoxID()) {
                curImage = box.GetImage();
            }
        }
        filledBoxTiles.push_back(FilledBoxTile(250.f, 550.f, filledBoxes[i].GetBox().GetBoxName(), curImage, filledBoxes[i].GetItems()));
    }

    Button goBackButton(50.f, 700.f, 100.f, 50.f, "Go Back");
    Button leftButton(25.f, 75.f, 25.f, 25.f, "<");
    Button rightButton(1350.f, 75.f, 25.f, 25.f, ">");

    size_t pageIndex = 0;
    const size_t columns = 5;

    bool quit = false;
    while (window.isOpen() && !quit) {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed) {
                window.close();
            } else if (event.type == sf::Event::MouseButtonPressed) {
                float px = event.mouseButton.x;
                float py = event.mouseButton.y;

                if (goBackButton.IsIn(px, py)) {
                    return;
                } else if (leftButton.IsIn(px, py)) {
                    if (pageIndex > 0) {
                        pageIndex--;
                    }
                } else if (rightButton.IsIn(px, py)) {
                    if ((pageIndex + 1) * columns < filledBoxTiles.size()) {
                        pageIndex++;
                    }
                }
            }
        }

        window.clear();
        AddTitle(window, title);
        goBackButton.Draw(window);
        if (pageIndex != 0) {
            leftButton.Draw(window);
        }
        size_t curIndex = 0;
        for (size_t i = 0; i < filledBoxTiles.size(); i++) {
            filledBoxTiles[i].IsPresent = false;
            if (curIndex / columns == pageIndex) {
                size_t innerIndex = curIndex % columns;
                filledBoxTiles[i].SetPosition(50.f + innerIndex * 265.f, 110.f);
                filledBoxTiles[i].IsPresent = true;
            }
            curIndex++;
        }
        if ((pageIndex + 1) * columns < curIndex) {
            rightButton.Draw(window);
        }
        for (FilledBoxTile& filledBoxTile : filledBoxTiles) {
            if (filledBoxTile.IsPresent) {
                filledBoxTile.Draw(window);
            }
        }
        window.display();
    }
}

void PrintBoxes(sf::RenderWindow& window, const vector<TFilledBox>& filledBoxes, const vector<TBox>& boxes) {
    if (filledBoxes.size() == 0) {
        Button goBackButton(50.f, 700.f, 100.f, 50.f, "Go Back");
        while (window.isOpen()) {
            sf::Event event;
            while (window.pollEvent(event))
            {
                if (event.type == sf::Event::Closed) {
                    window.close();
                } else if (event.type == sf::Event::MouseButtonPressed) {
                    float px = event.mouseButton.x;
                    float py = event.mouseButton.y;
                    if (goBackButton.IsIn(px, py)) {
                        return;
                    }
                }
            }

            window.clear();
            AddTitle(window, "Your order can't be packed because one of your items doesn't fit in any of our boxes. We are sorry.");
            goBackButton.Draw(window);
            window.display();
        }
    } else {
        ShowOrder(window, filledBoxes, boxes);
    }
}

void DidntBuyAnything(sf::RenderWindow& window) {
    Button goBackButton(50.f, 700.f, 100.f, 50.f, "Go Back");
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed) {
                window.close();
            } else if (event.type == sf::Event::MouseButtonPressed) {
                float px = event.mouseButton.x;
                float py = event.mouseButton.y;
                if (goBackButton.IsIn(px, py)) {
                    return;
                }
            }
        }
        window.clear();
        AddTitle(window, "You didn't buy anything. We hope you'll like something next time!");
        goBackButton.Draw(window);
        window.display();
    }
}

bool StartsWith(const string& str, const string& pref) {
    if (pref.size() > str.size()) {
        return false;
    }
    for (size_t i = 0; i < pref.size(); i++) {
        if (str[i] != pref[i]) {
            return false;
        }
    }
    return true;
}

const string currentDate() {
    time_t now = time(0);
    struct tm tstruct;
    char buf[80];
    tstruct = *localtime(&now);
    strftime(buf, sizeof(buf), "%Y-%m-%d", &tstruct);
    return buf;
}

void SaveOrder(const vector<TFilledBox>& filledBoxes, TDataBase& dataBase) {
    const string insertOrderQuery = "insert into Orders(userID, orderDate) values (1, '" + currentDate() + "');";
    dataBase.Query(insertOrderQuery);
    int64_t orderID = dataBase.GetLastInsertID();
    
    for (const TFilledBox& filledBox : filledBoxes) {
        const string insertFilledBoxQuery = "insert into FilledBox(boxID, orderID) values (" + to_string(filledBox.GetBox().GetBoxID()) + ", " + to_string(orderID) + ");";
        dataBase.Query(insertFilledBoxQuery);
        int64_t filledBoxID = dataBase.GetLastInsertID();

        for (const TItem& item : filledBox.GetItems()) {
            const string insertItemsForFilledBoxQuery = "insert into ItemsForFilledBox(itemID, filledBoxID) values (" + to_string(item.GetItemID()) + ", " + to_string(filledBoxID) + ");";
            dataBase.Query(insertItemsForFilledBoxQuery);
        }
    }
}

void UserMode(sf::RenderWindow& window, TDataBase& dataBase) {
    auto [items, boxes] = GetSettings(dataBase);
    vector<TBox> availableBoxes;
    for (const auto& box : boxes) {
        if (box.second) {
            availableBoxes.push_back(box.first);
        }
    }
    TShop shop(items, availableBoxes);

    vector<ItemTile> itemTiles;
    for (size_t i = 0; i < items.size(); i++) {
        itemTiles.push_back(ItemTile(250.f, 250.f, items[i].first.GetItemID(), items[i].first.GetItemName(), items[i].second, items[i].first.GetImage(), true));
    }
    Button finishButton(1200.f, 700.f, 100.f, 50.f, "Finish Order");
    Button goBackButton(50.f, 700.f, 100.f, 50.f, "Go Back");
    Button leftButton(25.f, 75.f, 25.f, 25.f, "<");
    Button rightButton(1350.f, 75.f, 25.f, 25.f, ">");

    TextField searchField(550.f, 50.f, 300.f, 50.f, "Search");

    size_t pageIndex = 0;
    const size_t rows = 2, columns = 5;
    const size_t pageSize = rows * columns;

    bool quit = false;
    while (window.isOpen() && !quit) {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed) {
                window.close();
            } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Backspace)) {
                searchField.PopChar();
                pageIndex = 0;
            } else if (event.type == sf::Event::TextEntered) {
                if (event.text.unicode < 128) {
                    char c = static_cast<char>(event.text.unicode);
                    searchField.AddChar(c);
                    pageIndex = 0;
                }
            } else if (event.type == sf::Event::MouseButtonPressed) {
                float px = event.mouseButton.x;
                float py = event.mouseButton.y;

                if (goBackButton.IsIn(px, py)) {
                    return;
                } else if (finishButton.IsIn(px, py)) {
                    quit = true;
                    break;
                } else if (leftButton.IsIn(px, py)) {
                    if (pageIndex > 0) {
                        pageIndex--;
                    }
                } else if (rightButton.IsIn(px, py)) {
                    if ((pageIndex + 1) * pageSize < itemTiles.size()) {
                        pageIndex++;
                    }
                } else {
                    for (size_t i = 0; i < items.size(); i++) {
                        if (itemTiles[i].IsPresent) {
                            if (itemTiles[i].minusButton.IsIn(px, py)) {
                                if (itemTiles[i].cnt > 0) {
                                    itemTiles[i].cnt--;
                                    shop.DeleteItem(items[i].first.GetItemID());
                                }
                            } else if (itemTiles[i].plusButton.IsIn(px, py)) {
                                if (itemTiles[i].cnt < itemTiles[i].maxcnt) {
                                    itemTiles[i].cnt++;
                                    shop.AddItem(items[i].first.GetItemID());
                                }
                            }
                        }
                    }
                }
            }
        }

        window.clear();
        AddTitle(window, "Select Items you want to buy:");
        finishButton.Draw(window);
        searchField.Draw(window);
        goBackButton.Draw(window);
        if (pageIndex != 0) {
            leftButton.Draw(window);
        }
        size_t curIndex = 0;
        for (size_t i = 0; i < itemTiles.size(); i++) {
            itemTiles[i].IsPresent = false;
            if (!StartsWith(itemTiles[i].name, searchField.label)) {
                continue;
            }
            if (curIndex / pageSize == pageIndex) {
                size_t innerIndex = curIndex % pageSize;
                itemTiles[i].SetPosition(50.f + (innerIndex % columns) * 265.f, 110.f + (innerIndex / columns) * 265.f);
                itemTiles[i].IsPresent = true;
            }
            curIndex++;
        }
        if ((pageIndex + 1) * pageSize < curIndex) {
            rightButton.Draw(window);
        }
        for (ItemTile& itemTile : itemTiles) {
            if (itemTile.IsPresent) {
                itemTile.Draw(window);
            }
        }
        window.display();
    }

    
    if (shop.OrderIsEmpty()) {
        DidntBuyAnything(window);
    } else {
        vector<TFilledBox> filledBoxes = shop.Buy();
        vector<pair<TItem, int32_t>> boughtItems;
        for (const TFilledBox& filledBox : filledBoxes) {
            for (const TItem& item : filledBox.GetItems()) {
                boughtItems.emplace_back(item, -1);
            }
        }
        SaveOrder(filledBoxes, dataBase);
        UpdateItems(window, boughtItems, dataBase);
        PrintBoxes(window, filledBoxes, availableBoxes);
    }
}

void AdminAddDeleteItem(sf::RenderWindow& window, TDataBase& dataBase) {
    auto [items, boxes] = GetSettings(dataBase);
    vector<pair<TItem, int32_t>> newItems(items.size());
    for (size_t i = 0; i < items.size(); i++) {
        newItems[i] = {items[i].first, 0};
    }

    vector<ItemTile> itemTiles;
    for (size_t i = 0; i < items.size(); i++) {
        itemTiles.push_back(ItemTile(250.f, 250.f, items[i].first.GetItemID(), items[i].first.GetItemName(), items[i].second, items[i].first.GetImage(), false));
    }
    Button goBackButton(50.f, 700.f, 100.f, 50.f, "Finish And Go Back");
    Button leftButton(25.f, 75.f, 25.f, 25.f, "<");
    Button rightButton(1350.f, 75.f, 25.f, 25.f, ">");
    
    TextField searchField(550.f, 50.f, 300.f, 50.f, "Search");

    size_t pageIndex = 0;
    const size_t rows = 2, columns = 5;
    const size_t pageSize = rows * columns;

    bool quit = false;
    while (window.isOpen() && !quit) {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed) {
                window.close();
            } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Backspace)) {
                searchField.PopChar();
                pageIndex = 0;
            } else if (event.type == sf::Event::TextEntered) {
                if (event.text.unicode < 128) {
                    char c = static_cast<char>(event.text.unicode);
                    searchField.AddChar(c);
                    pageIndex = 0;
                }
            } else if (event.type == sf::Event::MouseButtonPressed) {
                float px = event.mouseButton.x;
                float py = event.mouseButton.y;

                if (goBackButton.IsIn(px, py)) {
                    quit = true;
                    break;
                } else if (leftButton.IsIn(px, py)) {
                    if (pageIndex > 0) {
                        pageIndex--;
                    }
                } else if (rightButton.IsIn(px, py)) {
                    if ((pageIndex + 1) * pageSize < itemTiles.size()) {
                        pageIndex++;
                    }
                } else {
                    for (size_t i = 0; i < items.size(); i++) {
                        if (itemTiles[i].IsPresent) {
                            if (itemTiles[i].minusButton.IsIn(px, py)) {
                                if (itemTiles[i].maxcnt > 0) {
                                    itemTiles[i].maxcnt--;
                                    newItems[i].second--;
                                }
                            } else if (itemTiles[i].plusButton.IsIn(px, py)) {
                                itemTiles[i].maxcnt++;
                                newItems[i].second++;
                            }
                        }
                    }
                }
            }
        }

        window.clear();
        AddTitle(window, "Change items amounts:");
        goBackButton.Draw(window);
        searchField.Draw(window);
        if (pageIndex != 0) {
            leftButton.Draw(window);
        }
        size_t curIndex = 0;
        for (size_t i = 0; i < itemTiles.size(); i++) {
            itemTiles[i].IsPresent = false;
            if (!StartsWith(itemTiles[i].name, searchField.label)) {
                continue;
            }
            if (curIndex / pageSize == pageIndex) {
                size_t innerIndex = curIndex % pageSize;
                itemTiles[i].SetPosition(50.f + (innerIndex % columns) * 265.f, 110.f + (innerIndex / columns) * 265.f);
                itemTiles[i].IsPresent = true;
            }
            curIndex++;
        }
        if ((pageIndex + 1) * pageSize < curIndex) {
            rightButton.Draw(window);
        }
        for (ItemTile& itemTile : itemTiles) {
            if (itemTile.IsPresent) {
                itemTile.Draw(window);
            }
        }
        window.display();
    }

    UpdateItems(window, newItems, dataBase);
}

string GetItemsString(const vector<TItem>& items) {
    string ans = "Your new items:\nName\tWeight\tVolume";
    for (const TItem& item : items) {
        ans += "\n" + item.GetItemName() + "\t\t\t" + to_string(item.GetWeight()) + "\t\t\t" + to_string(item.GetVolume());
    }
    return ans;
}

string GetImageBytes(const string &filename) {
    ifstream file(filename, ios::binary);
    string bytes((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    string ans;
    ans.reserve(bytes.size() << 3);
    for (size_t i = 0; i < bytes.size(); i++) {
        for (int j = 7; j >= 0; j--) {
            ans.push_back(((bytes[i] >> j) & 1) + '0');
        }
    }
    return ans;
}

void AdminCreateItem(sf::RenderWindow& window, TDataBase& dataBase) {
    static TItem fakeItem(0, "already exists!", 0, 0);
    Button goBackButton(50.f, 700.f, 100.f, 50.f, "Go Back");
    TextField nameField(100.f, 550.f, 100.f, 50.f, "Name");
    TextField weightField(400.f, 550.f, 100.f, 50.f, "Weight");
    TextField volumeField(700.f, 550.f, 100.f, 50.f, "Volume");
    TextField imageField(1000.f, 550.f, 100.f, 50.f, "Image Path");
    Button addButton(1250.f, 550.f, 100.f, 50.f, "Add");
    string selected = "name";
    vector<TItem> items;
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed) {
                window.close();
            } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Backspace)) {
                if (selected == "name") {
                    nameField.PopChar();
                } else if (selected == "weight") {
                    weightField.PopChar();
                } else if (selected == "volume") {
                    volumeField.PopChar();
                } else if (selected == "image") {
                    imageField.PopChar();
                }
            } else if (event.type == sf::Event::TextEntered) {
                if (event.text.unicode < 128) {
                    char c = static_cast<char>(event.text.unicode);
                    if (selected == "name") {
                        nameField.AddChar(c);
                    } else if (selected == "weight") {
                        weightField.AddChar(c);
                    } else if (selected == "volume") {
                        volumeField.AddChar(c);
                    } else if (selected == "image") {
                        imageField.AddChar(c);
                    }
                }
            } else if (event.type == sf::Event::MouseButtonPressed) {
                float px = event.mouseButton.x;
                float py = event.mouseButton.y;
                if (goBackButton.IsIn(px, py)) {
                    return;
                } else if (addButton.IsIn(px, py)) {
                    string selectQuery = "select itemName from Item where itemName = '" + nameField.label + "';";
                    auto selectResponse = dataBase.Query(selectQuery);
                    if (!selectResponse.empty()) {
                        items.push_back(fakeItem);
                    } else {
                        TItem newItem(0, nameField.label, ToInt(weightField.label), ToInt(volumeField.label));
                        items.push_back(newItem);
                        string insertQuery = "insert into Item(itemName, weight, volume, amount, image) values ('" + newItem.GetItemName() + "', " + to_string(newItem.GetWeight()) + ", " + to_string(newItem.GetVolume()) + ", 0, '" + GetImageBytes(imageField.label) + "');";
                        dataBase.Query(insertQuery);
                    }
                    nameField.Clear();
                    weightField.Clear();
                    volumeField.Clear();
                    imageField.Clear();
                } else if (nameField.IsIn(px, py)) {
                    selected = "name";
                } else if (weightField.IsIn(px, py)) {
                    selected = "weight";
                } else if (volumeField.IsIn(px, py)) {
                    selected = "volume";
                } else if (imageField.IsIn(px, py)) {
                    selected = "image";
                }
            }
        }

        window.clear();
        AddTitle(window, "Enter items:");
        AddTitle(window, GetItemsString(items), 100.f, 100.f, 20);
        goBackButton.Draw(window);
        addButton.Draw(window);
        nameField.Draw(window);
        weightField.Draw(window);
        volumeField.Draw(window);
        imageField.Draw(window);
        window.display();
    }
}

void AdminAddDeleteBox(sf::RenderWindow& window, TDataBase& dataBase) {
    auto [items, boxes] = GetSettings(dataBase);
    vector<pair<TBox, int32_t>> newBoxes(boxes.size());
    for (size_t i = 0; i < boxes.size(); i++) {
        newBoxes[i] = {boxes[i].first, 0};
    }

    vector<BoxTile> boxTiles;
    for (size_t i = 0; i < boxes.size(); i++) {
        boxTiles.push_back(BoxTile(250.f, 250.f, boxes[i].first.GetBoxID(), boxes[i].first.GetBoxName(), boxes[i].second, boxes[i].first.GetImage()));
    }
    Button goBackButton(50.f, 700.f, 100.f, 50.f, "Finish And Go Back");
    Button leftButton(25.f, 75.f, 25.f, 25.f, "<");
    Button rightButton(1350.f, 75.f, 25.f, 25.f, ">");
    
    TextField searchField(550.f, 50.f, 300.f, 50.f, "Search");

    size_t pageIndex = 0;
    const size_t rows = 2, columns = 5;
    const size_t pageSize = rows * columns;

    bool quit = false;
    while (window.isOpen() && !quit) {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed) {
                window.close();
            } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Backspace)) {
                searchField.PopChar();
                pageIndex = 0;
            } else if (event.type == sf::Event::TextEntered) {
                if (event.text.unicode < 128) {
                    char c = static_cast<char>(event.text.unicode);
                    searchField.AddChar(c);
                    pageIndex = 0;
                }
            } else if (event.type == sf::Event::MouseButtonPressed) {
                float px = event.mouseButton.x;
                float py = event.mouseButton.y;

                if (goBackButton.IsIn(px, py)) {
                    quit = true;
                    break;
                } else if (leftButton.IsIn(px, py)) {
                    if (pageIndex > 0) {
                        pageIndex--;
                    }
                } else if (rightButton.IsIn(px, py)) {
                    if ((pageIndex + 1) * pageSize < boxTiles.size()) {
                        pageIndex++;
                    }
                } else {
                    for (size_t i = 0; i < boxes.size(); i++) {
                        if (boxTiles[i].IsPresent) {
                            if (boxTiles[i].availableButton.IsIn(px, py)) {
                                if (boxTiles[i].available) {
                                    boxTiles[i].available = false;
                                    newBoxes[i].second--;
                                } else {
                                    boxTiles[i].available = true;
                                    newBoxes[i].second++;
                                }
                            }
                        }
                    }
                }
            }
        }

        window.clear();
        AddTitle(window, "Change Boxes Availability:");
        goBackButton.Draw(window);
        searchField.Draw(window);
        if (pageIndex != 0) {
            leftButton.Draw(window);
        }
        size_t curIndex = 0;
        for (size_t i = 0; i < boxTiles.size(); i++) {
            boxTiles[i].IsPresent = false;
            if (!StartsWith(boxTiles[i].name, searchField.label)) {
                continue;
            }
            if (curIndex / pageSize == pageIndex) {
                size_t innerIndex = curIndex % pageSize;
                boxTiles[i].SetPosition(50.f + (innerIndex % columns) * 265.f, 110.f + (innerIndex / columns) * 265.f);
                boxTiles[i].IsPresent = true;
            }
            curIndex++;
        }
        if ((pageIndex + 1) * pageSize < curIndex) {
            rightButton.Draw(window);
        }
        for (BoxTile& boxTile : boxTiles) {
            if (boxTile.IsPresent) {
                boxTile.Draw(window);
            }
        }
        window.display();
    }

    UpdateBoxes(window, newBoxes, dataBase);
}

string GetBoxesString(const vector<TBox>& boxes) {
    string ans = "Your new boxes:\nName\tMaxWeight\tMaxVolume\tCost";
    for (const TBox& box : boxes) {
        ans += "\n" + box.GetBoxName() + "\t\t\t" + to_string(box.GetMaxWeight()) + "\t\t\t" + to_string(box.GetMaxVolume()) + "\t\t\t" + to_string(box.GetCost());
    }
    return ans;
}

void AdminCreateBox(sf::RenderWindow& window, TDataBase& dataBase) {
    static TBox fakeBox(0, "already exists!", 0, 0, 0);
    Button goBackButton(50.f, 700.f, 100.f, 50.f, "Go Back");
    TextField nameField(100.f, 550.f, 100.f, 50.f, "Name");
    TextField weightField(300.f, 550.f, 100.f, 50.f, "MaxWeight");
    TextField volumeField(500.f, 550.f, 100.f, 50.f, "MaxVolume");
    TextField costField(700.f, 550.f, 100.f, 50.f, "Cost");
    TextField imageField(900.f, 550.f, 100.f, 50.f, "Image Path");
    Button addButton(1100.f, 550.f, 100.f, 50.f, "Add");
    string selected = "name";
    vector<TBox> boxes;
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed) {
                window.close();
            } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Backspace)) {
                if (selected == "name") {
                    nameField.PopChar();
                } else if (selected == "weight") {
                    weightField.PopChar();
                } else if (selected == "volume") {
                    volumeField.PopChar();
                } else if (selected == "cost") {
                    costField.PopChar();
                } else if (selected == "image") {
                    imageField.PopChar();
                }
            } else if (event.type == sf::Event::TextEntered) {
                if (event.text.unicode < 128) {
                    char c = static_cast<char>(event.text.unicode);
                    if (selected == "name") {
                        nameField.AddChar(c);
                    } else if (selected == "weight") {
                        weightField.AddChar(c);
                    } else if (selected == "volume") {
                        volumeField.AddChar(c);
                    } else if (selected == "cost") {
                        costField.AddChar(c);
                    } else if (selected == "image") {
                        imageField.AddChar(c);
                    }
                }
            } else if (event.type == sf::Event::MouseButtonPressed) {
                float px = event.mouseButton.x;
                float py = event.mouseButton.y;
                if (goBackButton.IsIn(px, py)) {
                    return;
                } else if (addButton.IsIn(px, py)) {
                    string selectQuery = "select boxName from Box where boxName = '" + nameField.label + "';";
                    auto selectResponse = dataBase.Query(selectQuery);
                    if (!selectResponse.empty()) {
                        boxes.push_back(fakeBox);
                    } else {
                        TBox newBox(0, nameField.label, ToInt(weightField.label), ToInt(volumeField.label), ToInt(costField.label));
                        boxes.push_back(newBox);
                        string insertQuery = "insert into Box(boxName, maxWeight, maxVolume, cost, image) values ('" + newBox.GetBoxName() + "', " + to_string(newBox.GetMaxWeight()) + ", " + to_string(newBox.GetMaxVolume()) + ", " + to_string(newBox.GetCost()) + ", 1, '" + GetImageBytes(imageField.label) + "');";
                        dataBase.Query(insertQuery);
                    }
                    nameField.Clear();
                    weightField.Clear();
                    volumeField.Clear();
                    costField.Clear();
                    imageField.Clear();
                } else if (nameField.IsIn(px, py)) {
                    selected = "name";
                } else if (weightField.IsIn(px, py)) {
                    selected = "weight";
                } else if (volumeField.IsIn(px, py)) {
                    selected = "volume";
                } else if (costField.IsIn(px, py)) {
                    selected = "cost";
                } else if (imageField.IsIn(px, py)) {
                    selected = "image";
                }
            }
        }

        window.clear();
        AddTitle(window, "Enter boxes:");
        AddTitle(window, GetBoxesString(boxes), 100.f, 100.f, 20);
        goBackButton.Draw(window);
        addButton.Draw(window);
        nameField.Draw(window);
        weightField.Draw(window);
        volumeField.Draw(window);
        costField.Draw(window);
        imageField.Draw(window);
        window.display();
    }

}

void AdminMode(sf::RenderWindow& window, TDataBase& dataBase) {
    Button addItemButton(350.f, 100.f, 200.f, 100.f, "Add/Delete Items");
    Button createItemButton(350.f, 300.f, 200.f, 100.f, "Create New Item");
    Button addBoxButton(850.f, 100.f, 200.f, 100.f, "Add/Delete Boxes");
    Button createBoxButton(850.f, 300.f, 200.f, 100.f, "Create New Box");
    Button goBackButton(50.f, 700.f, 100.f, 50.f, "Go Back");

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            } else if (event.type == sf::Event::MouseButtonPressed) {
                float px = event.mouseButton.x;
                float py = event.mouseButton.y;

                if (addItemButton.IsIn(px, py)) {
                    AdminAddDeleteItem(window, dataBase);
                } else if (createItemButton.IsIn(px, py)) {
                    AdminCreateItem(window, dataBase);
                } else if (addBoxButton.IsIn(px, py)) {
                    AdminAddDeleteBox(window, dataBase);
                } else if (createBoxButton.IsIn(px, py)) {
                    AdminCreateBox(window, dataBase);
                } else if (goBackButton.IsIn(px, py)) {
                    return;
                }
            }
        }

        window.clear();
        AddTitle(window, "Choose action:");
        addItemButton.Draw(window);
        createItemButton.Draw(window);
        addBoxButton.Draw(window);
        createBoxButton.Draw(window);
        goBackButton.Draw(window);
        window.display();
    }
}

vector<TOrder> GetOrders(TDataBase& dataBase) {
    const string selectUsersQuery = "select * from Users;";
    auto usersVector = dataBase.Query(selectUsersQuery);
    map<uint64_t, string> userNames;
    for (auto& row : usersVector) {
        userNames[ToInt(row["userID"])] = row["userName"];
    }

    const string selectOrdersQuery = "select * from Orders;";
    auto ordersVector = dataBase.Query(selectOrdersQuery);
    map<uint64_t, TOrder> orders;
    for (auto& row : ordersVector) {
        const uint64_t orderID = ToInt(row["orderID"]);
        const uint64_t userID = ToInt(row["userID"]);
        orders[orderID] = TOrder(orderID, userID, userNames[userID], row["orderDate"]);
    }

    const auto& [itemsVector, boxesVector] = GetSettings(dataBase);

    map<uint64_t, TItem> items;
    for (const auto& [item, amount] : itemsVector) {
        items[item.GetItemID()] = item;
    }

    map<uint64_t, TBox> boxes;
    for (const auto& [box, amount] : boxesVector) {
        boxes[box.GetBoxID()] = box;
    }

    const string selectFilledBoxesQuery = "select * from FilledBox;";
    auto FilledBoxesVector = dataBase.Query(selectFilledBoxesQuery);
    map<uint64_t, pair<TFilledBox, uint64_t>> filledBoxes;
    for (auto& row : FilledBoxesVector) {
        filledBoxes[ToInt(row["filledBoxID"])] = {TFilledBox(boxes[ToInt(row["boxID"])]), ToInt(row["orderID"])};
    }

    const string selectItemsForFilledBoxQuery = "select * from ItemsForFilledBox;";
    auto ItemsForFilledBoxVector = dataBase.Query(selectItemsForFilledBoxQuery);
    for (auto& row : ItemsForFilledBoxVector) {
        filledBoxes[ToInt(row["filledBoxID"])].first.Items.push_back(items[ToInt(row["itemID"])]);
    }

    for (const auto& [id, filledBox] : filledBoxes) {
        orders[filledBox.second].FilledBoxes.push_back(filledBox.first);
    }

    vector<TOrder> ans;
    ans.reserve(orders.size());
    for (const auto& order : orders) {
        ans.push_back(order.second);
    }
    return ans;
}

void ShowHistory(sf::RenderWindow& window, TDataBase& dataBase) {
    vector<TOrder> orders = GetOrders(dataBase);
    vector<Button> orderButtons(orders.size());
    for (size_t i = 0; i < orders.size(); i++) {
        orderButtons[i] = Button(0.f, 0.f, 1300.f, 50.f, "Order # " + to_string(orders[i].OrderID) + "\t\tUser: " + orders[i].UserName + "\t\tOrder Date: " + orders[i].OrderDate);
    }

    auto [items, boxes] = GetSettings(dataBase);
    vector<TBox> availableBoxes;
    for (auto [box, available] : boxes) {
        if (available) {
            availableBoxes.push_back(box);
        }
    }

    Button goBackButton(50.f, 700.f, 100.f, 50.f, "Go Back");
    Button leftButton(25.f, 75.f, 25.f, 25.f, "<");
    Button rightButton(1350.f, 75.f, 25.f, 25.f, ">");

    size_t pageIndex = 0;
    const size_t rows = 13;

    bool quit = false;
    while (window.isOpen() && !quit) {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed) {
                window.close();
            } else if (event.type == sf::Event::MouseButtonPressed) {
                float px = event.mouseButton.x;
                float py = event.mouseButton.y;

                if (goBackButton.IsIn(px, py)) {
                    return;
                } else if (leftButton.IsIn(px, py)) {
                    if (pageIndex > 0) {
                        pageIndex--;
                    }
                } else if (rightButton.IsIn(px, py)) {
                    if ((pageIndex + 1) * rows < orderButtons.size()) {
                        pageIndex++;
                    }
                } else {
                    for (size_t i = 0; i < orders.size(); i++) {
                        if (orderButtons[i].IsPresent && orderButtons[i].IsIn(px, py)) {
                            ShowOrder(window, orders[i].FilledBoxes, availableBoxes, orderButtons[i].label);
                            break;
                        }
                    }
                }
            }
        }

        window.clear();
        AddTitle(window, "This Is History Of Your Orders. You Can Select Any Of Them.");
        goBackButton.Draw(window);
        if (pageIndex != 0) {
            leftButton.Draw(window);
        }
        size_t curIndex = 0;
        for (size_t i = 0; i < orderButtons.size(); i++) {
            orderButtons[i].IsPresent = false;
            if (curIndex / rows == pageIndex) {
                size_t innerIndex = curIndex % rows;
                orderButtons[i].SetPosition(50.f, 100.f + innerIndex * 75.f);
                orderButtons[i].IsPresent = true;
            }
            curIndex++;
        }
        if ((pageIndex + 1) * rows < curIndex) {
            rightButton.Draw(window);
        }
        for (Button& orderButton : orderButtons) {
            if (orderButton.IsPresent) {
                orderButton.Draw(window);
            }
        }
        window.display();
    }
}

void ChooseMode(sf::RenderWindow& window, TDataBase& dataBase) {
    Button adminButton(200.f, 350.f, 200.f, 100.f, "Admin");
    Button userButton(600.f, 350.f, 200.f, 100.f, "Buy");
    Button historyButton(1000.f, 350.f, 200.f, 100.f, "See History");
    Button goBackButton(50.f, 700.f, 100.f, 50.f, "Close The App");

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            } else if (event.type == sf::Event::MouseButtonPressed) {
                float px = event.mouseButton.x;
                float py = event.mouseButton.y;

                if (adminButton.IsIn(px, py)) {
                    AdminMode(window, dataBase);
                }
                else if (userButton.IsIn(px, py)) {
                    UserMode(window, dataBase);
                } else if (historyButton.IsIn(px, py)) {
                    ShowHistory(window, dataBase);
                } else if (goBackButton.IsIn(px, py)) {
                    return;
                }
            }
        }
        window.clear();
        AddTitle(window, "Choose mode:");
        adminButton.Draw(window);
        userButton.Draw(window);
        historyButton.Draw(window);
        goBackButton.Draw(window);
        window.display();
    }
}

int main() {
    NFont::GetFont();
    sf::RenderWindow window(sf::VideoMode(1400, 800), "Shop");
    TDataBase dataBase("db.sqlite");

    ChooseMode(window, dataBase);
 
    window.close();
    dataBase.Close();
    return 0;
}
