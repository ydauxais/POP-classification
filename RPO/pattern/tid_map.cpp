#include "tid_map.hh"

std::vector<unsigned>& TidMap::operator[](unsigned index) {
    return tids_[index];
}

const std::vector<unsigned>& TidMap::operator[](unsigned index) const {
    return tids_.find(index)->second;
}
