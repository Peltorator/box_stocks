#pragma once

#include "box.h"
#include "item.h"
#include <list>

class TFilledBox {
private:
    typedef std::list<TItem> TItemContainer;

    TBox Box;
    TItemContainer Items;
public:
    TFilledBox(const TBox& box, const TItemContainer& items) : Box(box), Items(items) {}

    TBox GetBox() const {
        return Box;
    }

    TItemContainer GetItems() const {
        return Items;
    }
};
