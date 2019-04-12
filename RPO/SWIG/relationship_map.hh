
#ifndef GDE_RELATIONSHIP_MAP_HH
#define GDE_RELATIONSHIP_MAP_HH

#include <sstream>
#include "episode.hh"

class RelationshipMap {

private:
    std::map<std::pair<Item, Item>, std::pair<int, int>> constraints_;

public:
    RelationshipMap(const Episode& episode) {
        constraints_ = episode.get_constraint_map();
    }

    std::string get_relationship(const std::pair<Item, Item>& key) const {
        auto it = constraints_.find(key);
        if (it == constraints_.end())
            return "";
        return get_relationship_string(it->second);
    }

    std::string get_relationship_string(const std::pair<int, int>& r) const {
        std::stringstream os;
        os << "[" << r.first << "," << r.second << "]";
        return os.str();
    }
};


#endif //GDE_RELATIONSHIP_MAP_HH
