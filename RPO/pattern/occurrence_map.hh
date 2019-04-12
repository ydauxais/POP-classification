#ifndef GDE_OCCURRENCE_MAP_HH
#define GDE_OCCURRENCE_MAP_HH

#include <map>
#include <vector>
#include <utility>

class OccurrenceMap {
private:
    std::map<unsigned, std::vector<std::vector<std::vector<int>>>> occurrences_;

public:
    typedef std::map<unsigned, std::vector<std::vector<std::vector<int>>>> T;
    typedef T::iterator iterator;
    typedef T::const_iterator const_iterator;

    OccurrenceMap() = default;
    OccurrenceMap(std::map<unsigned, std::vector<std::vector<std::vector<int>>>> occurrences):
        occurrences_(std::move(occurrences)) {}

    std::vector<std::vector<std::vector<int>>>& operator[](unsigned index);
    const std::vector<std::vector<std::vector<int>>>& operator[](unsigned index) const;

    iterator begin() { return occurrences_.begin(); }
    const_iterator begin() const { return occurrences_.cbegin(); }
    iterator end() { return occurrences_.end(); }
    const_iterator end() const { return occurrences_.cend(); }
};


#endif //GDE_OCCURRENCE_MAP_HH
