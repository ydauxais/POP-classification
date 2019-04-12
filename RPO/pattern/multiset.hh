#ifndef GDE_MULTISET_HH
#define GDE_MULTISET_HH

#include <map>
#include <memory>
#include <string>
#include <vector>
#include "database.hh"

class Multiset {

private:
    std::shared_ptr<Database> base_;
    std::vector<unsigned> multiset_;
    //TODO: use Item as in Episode
    std::vector<std::pair<unsigned, unsigned>> item_vector_;

public:
    Multiset(): base_(nullptr){};
    Multiset(std::shared_ptr<Database> base, const std::vector<unsigned>& cardinalities);;

    const std::vector<std::pair<unsigned, unsigned>>& get_item_vector() const;
    std::map<unsigned int, std::vector<std::vector<std::vector<int>>>> get_occurrences() const;
    std::map<unsigned int, std::vector<std::vector<std::vector<int>>>>
            get_occurrences(const std::vector<unsigned> &tid_list, bool numeric_intervals,
                                        std::vector<unsigned int> *negative_tid_list = nullptr,
                                        bool minimal_occurrences = false, int time_window = -1) const;
    std::string get_event_name(unsigned code) const {
        return base_->get_event_name(code);
    }

    const std::vector<unsigned>& get_multiset() const {
        return multiset_;
    }

    std::vector<std::pair<std::string, unsigned>> get_multiset_str_list() const {
        return base_->get_multiset_str_list(multiset_);
    }

    unsigned long size() const {
        return item_vector_.size();
    }

    explicit operator std::string() const {
        return base_->get_multiset_str(multiset_);
    }

private:
    std::vector<std::pair<unsigned, unsigned>> generate_item_vector() const;
};


#endif //GDE_MULTISET_HH
