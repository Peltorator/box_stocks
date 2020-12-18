#pragma once

#include <vector>
#include <iostream>
#include <sstream>
#include "../ShopModel/filled_box.cpp"
#include "../ShopModel/order.cpp"
#include "../../libs/cpp-httplib/httplib.h"

namespace NHttp {

    httplib::Client cli("http://localhost:8080");

    std::string DecodeImage(const std::string &s) {
        std::string bytes;
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

    std::string GetImageBytes(const std::string &filename) {
        std::ifstream file(filename, std::ios::binary);
        std::string bytes((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        std::string ans;
        ans.reserve(bytes.size() << 3);
        for (size_t i = 0; i < bytes.size(); i++) {
            for (int j = 7; j >= 0; j--) {
                ans.push_back(((bytes[i] >> j) & 1) + '0');
            }
        }
        return ans;
    }

    std::vector<std::pair<TBox, uint32_t>> GetBoxes() {
        auto res = cli.Get("/get_boxes");
        std::stringstream body(res->body);
        std::vector<std::pair<TBox, uint32_t>> boxes;
        int boxesSize;
        body >> boxesSize;
        boxes.reserve(boxesSize);
        for (int i = 0; i < boxesSize; i++) {
            int64_t id, weight, volume, cost, amount;
            std::string name, image;
            body >> id >> name >> weight >> volume >> cost >> image >> amount;
            boxes.emplace_back(TBox(id, name, weight, volume, cost, DecodeImage(image)), amount);
        }
        return boxes;
    }

    std::vector<TBox> GetAvailableBoxes() {
        std::vector<std::pair<TBox, uint32_t>> allBoxes;
        std::vector<TBox> availableBoxes;
        for (const auto& [box, amount] : allBoxes) {
            if (amount > 0) {
                availableBoxes.emplace_back(box);
            }
        }
        return availableBoxes;
    }

    std::vector<std::pair<TItem, uint32_t>> GetItems() {
        auto res = cli.Get("/get_items");
        std::stringstream body(res->body);
        std::vector<std::pair<TItem, uint32_t>> items;
        int itemsSize;
        body >> itemsSize;
        items.reserve(itemsSize);
        for (int i = 0; i < itemsSize; i++) {
            int64_t id, weight, volume, cost, amount;
            std::string name, image;
            body >> id >> name >> weight >> volume >> cost >> image >> amount;
            items.emplace_back(TItem(id, name, weight, volume, cost, DecodeImage(image)), amount);
        }
        return items;
    }

    void AddItem(const uint64_t itemID) {
        auto res = cli.Get(("/add_item/" + std::to_string(itemID)).c_str());
    }

    void DeleteItem(const uint64_t itemID) {
        auto res = cli.Get(("/delete_item/" + std::to_string(itemID)).c_str());
    }

    bool OrderIsEmpty() {
        auto res = cli.Get("/order_is_empty");
        std::stringstream body(res->body);
        int ans;
        body >> ans;
        return static_cast<bool>(ans);
    }

    std::vector<TFilledBox> ShopBuy() {
        auto res = cli.Get("/buy");
        std::stringstream body(res->body);
        std::vector<TFilledBox> order;
        int orderSize;
        body >> orderSize;
        order.reserve(orderSize);
        for (int i = 0; i < orderSize; i++) {
            int64_t boxID;
            body >> boxID;
            int itemsSize;
            body >> itemsSize;
            std::vector<uint64_t> itemIDs(itemsSize);
            for (int j = 0; j < itemsSize; j++) {
                uint64_t itemID;
                body >> itemID;
                itemIDs.emplace_back(itemID);
            }
            order.emplace_back(boxID, itemIDs);
        }
        return order;
    }

    void SaveOrder(const std::vector<TFilledBox>& order) {
        // TODO
    }

    void UpdateItems(const std::vector<std::pair<uint64_t, int32_t>>& items) {
        for (const auto& [itemID, amount] : items) {
            auto res = cli.Get(("/update_item/" + std::to_string(itemID) + "/" + std::to_string(amount)).c_str());
        }
    }

    void InsertItem(const TItem& item, const std::string& imagePath) {
        auto res = cli.Get(("/insert_item/" + item.ItemName + "/" + std::to_string(item.Weight) + "/" + std::to_string(item.Volume) + "/" + std::to_string(item.Cost) + "/" + GetImageBytes(imagePath)).c_str());
    }

    void UpdateBoxes(const std::vector<std::pair<uint64_t, int32_t>>& boxes) {
        for (const auto& [boxID, amount] : boxes) {
            auto res = cli.Get(("/update_box" + std::to_string(boxID) + "/" + std::to_string(amount)).c_str());
        }
    }

    void InsertBox(const TBox& box, const std::string& imagePath) {
         auto res = cli.Get(("/insert_box/" + box.BoxName + "/" + std::to_string(box.MaxWeight) + "/" + std::to_string(box.MaxVolume) + "/" + std::to_string(box.Cost) + "/" + GetImageBytes(imagePath)).c_str());
    }

    std::vector<TOrder> GetOrders() {
        auto res = cli.Get("/get_orders");
        std::stringstream body(res->body);
        std::vector<TOrder> orders;
        int ordersSize;
        body >> ordersSize;
        orders.reserve(ordersSize);
        for (int i = 0; i < ordersSize; i++) {
            int64_t orderID, userID;
            std::string userName, orderDate;
            body >> orderID >> userID >> userName >> orderDate;
            std::vector<TFilledBox> filledBoxes;
            int filledBoxesSize;
            body >> filledBoxesSize;
            for (int j = 0; j < filledBoxesSize; j++) {
                uint64_t boxID;
                body >> boxID;
                int itemsSize;
                body >> itemsSize;
                std::vector<uint64_t> itemIDs(itemsSize);
                for (int k = 0; k < itemsSize; k++) {
                    uint64_t itemID;
                    body >> itemID;
                    itemIDs.emplace_back(itemID);
                }
                filledBoxes.emplace_back(boxID, itemIDs);
            }
            orders.emplace_back(orderID, userID, userName, orderDate, filledBoxes);
        }
        return orders;
    }

    bool CheckIfItemExists(const std::string& itemName) {
        auto res = cli.Get(("/item_exists/" + itemName).c_str());
        std::stringstream body(res->body);
        int ans;
        body >> ans;
        return static_cast<bool>(ans);
    }

    bool CheckIfBoxExists(const std::string& boxName) {
        auto res = cli.Get(("/box_exists/" + boxName).c_str());
        std::stringstream body(res->body);
        int ans;
        body >> ans;
        return static_cast<bool>(ans);
    }
}
