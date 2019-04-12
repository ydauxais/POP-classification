#include "item.hh"

unsigned Item::get_card() const {
    return card_;
}

unsigned Item::get_item() const {
    return item_;
}

bool Item::operator<(const Item &other) const {
    if (item_ < other.item_)
        return true;
    return item_ == other.item_ && card_ < other.card_;
}