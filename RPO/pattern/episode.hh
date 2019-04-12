#ifndef GDE_EPISODE_HH
#define GDE_EPISODE_HH

#include <iostream>
#include <sstream>

#include "item.hh"
#include "multiset.hh"
#include "occurrence_map.hh"
#include "tid_map.hh"
#include "vertical_database.hh"

class Episode {

public:
    /*
    enum class Relationship {
        after, before, same_time, strictly_after, strictly_before
    };*/

protected:
    Multiset multiset_;
    std::map<std::pair<Item, Item>, std::pair<int, int>> constraints_;
    std::map<std::pair<Item, Item>, unsigned> map_couple_to_occurrence_index_;

    std::map<unsigned, unsigned> frequencies_;
    bool numeric_constraints_ {false};

    bool valid_{true};
    TidMap tid_list_;

public:
    // Only for SWIG
    // TODO: use proper ifdef
    Episode() = default;
    Episode(const Multiset &multiset, const std::vector<std::pair<int, int>> &constraints, bool numeric_constraints = false);

    const std::map<std::pair<Item, Item>, std::pair<int, int>> & get_constraint_map() const;

    const std::map<unsigned, unsigned>& get_frequencies() const;
    unsigned get_frequency(unsigned class_id) const;
    const Multiset& get_multiset() const;
    const TidMap & get_tid_list() const;

    bool is_chronicle() const {
        return numeric_constraints_;
    }

    bool is_valid() const {
        return valid_;
    }

    unsigned int occurs_in_sequence(const std::vector<std::vector<unsigned>> &sequence, bool number = false) const;

    void set_frequencies(const OccurrenceMap &occurrences);

    void set_frequencies(const std::map<unsigned, unsigned>& frequencies);
    void set_tid_list(const TidMap &tid_list);

    void set_not_valid() {
        valid_ = false;
    }

    explicit operator std::string() const;

protected:
    unsigned compute_class_frequency(const std::vector<std::vector<std::vector<int>>> &class_occurrences) const;
    std::pair<unsigned, unsigned> get_item_couple(unsigned i, unsigned n) const;
    virtual std::string get_string_constraints() const;
    bool occurs_in_bag(const std::vector<std::vector<int>> &bag) const;
    virtual bool occurs_in_instance(const std::vector<int> &instance) const;
};


#endif //GDE_EPISODE_HH
