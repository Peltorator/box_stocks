#pragma once

#include "../ShopModel/filled_box.cpp"
#include "../DataBase/database_queries.cpp"
#include <vector>
#include <unordered_map>

class TShop {
private:
    std::vector<std::pair<TItem, uint32_t>> RawItems;
    std::vector<TBox> Boxes;
    std::unordered_map<uint64_t, TItem> Items;
    std::unordered_map<uint64_t, uint32_t> AvailableAmounts;
    std::unordered_map<uint64_t, uint32_t> OrderAmounts;

    const uint64_t INF_COST = std::numeric_limits<uint64_t>::max();
public:
    TShop();

    std::vector<TBox> GetBoxes() const;

    std::vector<std::pair<TItem, uint32_t>> GetRawItems() const;

    void AddItem(const uint64_t itemID, const uint32_t amount = 1);

    void DeleteItem(const uint64_t itemID, const uint32_t amount = 1);

    std::vector<TFilledBox> Buy();

    void Save();

    void Load();

    bool OrderIsEmpty() const;
};
