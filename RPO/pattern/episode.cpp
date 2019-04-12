#include "episode.hh"

Episode::Episode(const Multiset &multiset, const std::vector<std::pair<int, int>> &constraints, bool numeric_constraints)
        : multiset_(multiset),
          numeric_constraints_(numeric_constraints) {
    std::vector<std::pair<unsigned, unsigned>> item_vector = multiset_.get_item_vector();
    auto n = static_cast<unsigned>(item_vector.size());

    for (unsigned i = 0; i < constraints.size(); i++) {
        std::pair<unsigned, unsigned> couple = get_item_couple(i, n);
        Item source(item_vector[couple.first].first, item_vector[couple.first].second);
        Item target(item_vector[couple.second].first, item_vector[couple.second].second);
        map_couple_to_occurrence_index_[{source, target}] = i;

        constraints_[{source, target}] = constraints[i];
    }
}

unsigned Episode::compute_class_frequency(
        const std::vector<std::vector<std::vector<int>>> &class_occurrences) const {
    unsigned freq = 0;
    for (const auto& bag : class_occurrences)
        if (occurs_in_bag(bag))
            freq++;
    return freq;
}

const std::map<std::pair<Item, Item>, std::pair<int, int>> & Episode::get_constraint_map() const {
    return constraints_;
}

const std::map<unsigned, unsigned>& Episode::get_frequencies() const {
    return frequencies_;
}

unsigned Episode::get_frequency(unsigned class_id) const {
    return frequencies_.find(class_id)->second;
}

const Multiset& Episode::get_multiset() const {
    return multiset_;
}

const TidMap & Episode::get_tid_list() const {
    return tid_list_;
}

std::pair<unsigned, unsigned> Episode::get_item_couple(unsigned i, unsigned n) const {
    unsigned source = 0;
    unsigned temp_i = i;

    while(temp_i >= n - source -1) {
        temp_i -= (n - source -1);
        source++;
    }

    return {source, source+temp_i+1};
}

std::string Episode::get_string_constraints() const {
    std::ostringstream str_constraints;
    for (const auto& constraint : constraints_) {
        std::pair<Item, Item> couple = constraint.first;
        str_constraints << multiset_.get_event_name(couple.first.get_item()) << ":" << couple.first.get_card();

        if (!numeric_constraints_) {
            if (constraint.second.first < 0) {
                if (constraint.second.second < 0)
                    str_constraints << " <-| ";
                else if (constraint.second.second == 0)
                    str_constraints << " <- ";
            }
            else if (constraint.second.first == 0) {
                if (constraint.second.second == 0)
                    str_constraints << " -- ";
                else
                    str_constraints << " -> ";
            }
            else
                    str_constraints << " |-> ";
        }
        else
            str_constraints << " [" << constraint.second.first << ", " << constraint.second.second << "] ";

        str_constraints << multiset_.get_event_name(couple.second.get_item()) << ":" << couple.second.get_card() << std::endl;
    }
    return str_constraints.str();
}

bool Episode::occurs_in_bag(const std::vector<std::vector<int>> &bag) const {
    for (const auto& instance : bag)
        if (occurs_in_instance(instance))
            return true;
    return false;
}

bool Episode::occurs_in_instance(const std::vector<int> &instance) const {
    for (const auto& constraint : constraints_) {
        unsigned index = map_couple_to_occurrence_index_.find(constraint.first)->second;
        int value = instance[index];

        if (!numeric_constraints_) {
            if (value < 0) value = -1;
            else if (value > 0) value = 1;
        }

        if (value < constraint.second.first || value > constraint.second.second)
            return false;
    }
    return true;
}

unsigned int Episode::occurs_in_sequence(const std::vector<std::vector<unsigned>> &sequence, bool number) const {
    const std::vector<unsigned>& cardinalities = multiset_.get_multiset();
    std::vector<std::vector<int>> occurrences = VerticalDatabase::get_sequence_occurrences(sequence, cardinalities,
                                                                                           numeric_constraints_, false,
                                                                                           0);

    unsigned total = 0;
    for (const auto& occurrence: occurrences) {
        if (occurs_in_instance(occurrence)) {
            total++;
            if (!number)
                return total;
        }
    }
    return total;
}

void Episode::set_frequencies(const OccurrenceMap &occurrences) {
    for (const auto &class_occurrences : occurrences)
            frequencies_[class_occurrences.first] = compute_class_frequency(class_occurrences.second);
}

void Episode::set_frequencies(const std::map<unsigned, unsigned>& frequencies) {
    frequencies_[0] = frequencies.find(0)->second;
    frequencies_[1] = frequencies.find(1)->second;
}

void Episode::set_tid_list(const TidMap &tid_list) {
    tid_list_[0] = tid_list[0];
    tid_list_[1] = tid_list[1];
}

Episode::operator std::string() const {
    std::ostringstream str;
    str << std::string(multiset_) << std::endl << get_string_constraints();
    if (frequencies_.find(0) != frequencies_.end() && frequencies_.find(1) != frequencies_.end())
        str << "Support: " << get_frequency(1) << "/" << get_frequency(0);
    return str.str();
}