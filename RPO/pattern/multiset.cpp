#include "multiset.hh"

Multiset::Multiset(std::shared_ptr<Database> base, const std::vector<unsigned> &cardinalities) :
    base_(base), multiset_(cardinalities)
{
    item_vector_ = generate_item_vector();
};

std::vector<std::pair<unsigned, unsigned>> Multiset::generate_item_vector() const {
    std::vector<std::pair<unsigned, unsigned>> item_vector;
    for (unsigned item = 0; item < multiset_.size(); item++)
        for (unsigned i = 0; i < multiset_[item]; i++)
            item_vector.emplace_back(item, i);
    return item_vector;
}

const std::vector<std::pair<unsigned, unsigned>>& Multiset::get_item_vector() const {
    return item_vector_;
}

std::map<unsigned int, std::vector<std::vector<std::vector<int>>>> Multiset::get_occurrences() const {
    return base_->get_occurrences(multiset_, std::vector<unsigned>(), nullptr, false, false, 0);
};

std::map<unsigned int, std::vector<std::vector<std::vector<int>>>>
Multiset::get_occurrences(const std::vector<unsigned> &tid_list, bool numeric_intervals,
                          std::vector<unsigned int> *negative_tid_list, bool minimal_occurrences, int time_window) const {
    return base_->get_occurrences(multiset_, tid_list, negative_tid_list, numeric_intervals, minimal_occurrences, time_window);
}

