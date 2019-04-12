#include "occurrence_map.hh"

std::vector<std::vector<std::vector<int>>>& OccurrenceMap::operator[](unsigned index) {
    return occurrences_[index];
}

const std::vector<std::vector<std::vector<int>>>& OccurrenceMap::operator[](unsigned index) const {
    return occurrences_.find(index)->second;
}
