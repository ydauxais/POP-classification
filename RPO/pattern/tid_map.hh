#ifndef GDE_TID_MAP_HH
#define GDE_TID_MAP_HH

#include <map>
#include <vector>

class TidMap {

private:
    std::map<unsigned, std::vector<unsigned>> tids_;

public:
    TidMap() = default;
    explicit TidMap(std::map<unsigned, std::vector<unsigned>> tids): tids_(std::move(tids)){}

    std::vector<unsigned>& operator[](unsigned index);
    const std::vector<unsigned>& operator[](unsigned index) const;
};


#endif //GDE_TID_MAP_HH
