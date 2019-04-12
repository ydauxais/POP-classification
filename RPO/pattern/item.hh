#ifndef GDE_ITEM_H
#define GDE_ITEM_H


class Item {

private:
    unsigned item_;
    unsigned card_;

public:
    // Only for SWIG
    // TODO: use proper ifdef
    Item() = default;
    Item(unsigned item, unsigned card): item_(item), card_(card) {};

    unsigned get_card() const;
    unsigned get_item() const;

    bool operator< (const Item& other) const;
};


#endif //GDE_ITEM_H
