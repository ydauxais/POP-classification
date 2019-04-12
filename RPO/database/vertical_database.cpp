#include "vertical_database.hh"


VerticalDatabase::VerticalDatabase(const std::string &positives, const std::string &negatives):
    VerticalDatabase()
{
    load_positives(positives);
    load_negatives(negatives);
}


std::vector<std::vector<unsigned>> VerticalDatabase::get_absolute_occurrences (
        const std::vector<std::vector<unsigned>>& sequence,
        const std::vector<unsigned>& cardinalities
) {
    std::vector<std::vector<unsigned>> masked_sequence;
    if (!mask_sequence_multiset(sequence, cardinalities, masked_sequence))
        return std::vector<std::vector<unsigned>>();
    return masked_sequence;
}

std::vector<std::vector<unsigned>> VerticalDatabase::get_absolute_occurrences (
        const std::vector<std::vector<std::vector<unsigned>>>& class_database,
        unsigned sequence,
        const std::vector<unsigned>& cardinalities
) {
    std::vector<std::vector<unsigned>> masked_sequence;
    if (!mask_sequence_multiset(class_database, sequence, cardinalities, masked_sequence))
        return std::vector<std::vector<unsigned>>();
 return masked_sequence;
}

std::vector<std::vector<std::vector<int>>>
        VerticalDatabase::get_class_occurrences(const std::vector<std::vector<std::vector<unsigned>>> &class_database,
                                                        const std::vector<unsigned> &cardinalities, bool numeric_intervals,
                                                        std::vector<unsigned> *negative_tid_list, bool minimal_occurrences, int time_window) const {
    std::vector<std::vector<std::vector<int>>> occurrences;
    for (unsigned sequence = 0; sequence < class_database[0].size(); sequence++) {
        std::vector<std::vector<int>> sequence_occurrences =
                get_sequence_occurrences(class_database, sequence, cardinalities, numeric_intervals, minimal_occurrences, time_window);
        if (!sequence_occurrences.empty()) {
            occurrences.push_back(sequence_occurrences);
            if (negative_tid_list)
                negative_tid_list->push_back(sequence);
        }
    }
    return occurrences;
}

std::vector<std::vector<std::vector<int>>>
        VerticalDatabase::get_class_occurrences(const std::vector<std::vector<std::vector<unsigned>>> &class_database,
                                                        const std::vector<unsigned> &cardinalities, const std::vector<unsigned> &tid_list,
                                                        bool numeric_intervals, std::vector<unsigned int> *negative_tid_list, bool minimal_occurrences,
                                                        int time_window) const {
    if (tid_list.empty())
        return get_class_occurrences(class_database, cardinalities, numeric_intervals, negative_tid_list, minimal_occurrences, time_window);
    else
        return get_class_occurrences_from_tid_list(class_database, cardinalities, tid_list, numeric_intervals, minimal_occurrences, time_window);
}

std::vector<std::vector<std::vector<int>>>
        VerticalDatabase::get_class_occurrences_from_tid_list(const std::vector<std::vector<std::vector<unsigned>>> &class_database,
                                                                      const std::vector<unsigned> &cardinalities, const std::vector<unsigned> &tid_list,
                                                                      bool numeric_intervals, bool minimal_occurrences, int time_window) const {
    std::vector<std::vector<std::vector<int>>> occurrences;
    for (unsigned sequence : tid_list) {
        std::vector<std::vector<int>> sequence_occurrences =
                get_sequence_occurrences(class_database, sequence, cardinalities, numeric_intervals, minimal_occurrences, time_window);
        if (!sequence_occurrences.empty())
            occurrences.push_back(sequence_occurrences);
    }
    return occurrences;
}

std::vector<std::vector<unsigned>>
        VerticalDatabase::get_combinations(
        const std::vector<unsigned>& absolute_occurrences, unsigned card) {
    return get_combinations(absolute_occurrences, 0, card);
}

std::vector<std::vector<unsigned>>
VerticalDatabase::get_combinations(
        const std::vector<unsigned>& absolute_occurrences, unsigned index, unsigned card) {
    if (card > 0) {
        unsigned item = absolute_occurrences[index];
        std::vector<std::vector<unsigned>> partial_combinations =
                get_combinations(absolute_occurrences, index+1, card-1);
        std::vector<std::vector<unsigned>> combinations;
        combinations.emplace_back(1,item);
        combinations = merge_combinations(combinations, partial_combinations);

        if (absolute_occurrences.size() - index > card) {
            std::vector<std::vector<unsigned>> combinations_without_item =
                    get_combinations(absolute_occurrences, index+1, card);
            for (const std::vector<unsigned> &c : combinations_without_item)
                combinations.push_back(c);
        }

        return combinations;
    }

    std::vector<std::vector<unsigned>> empty;
    empty.emplace_back();
    return empty;
}

std::string VerticalDatabase::get_multiset_str(const std::vector<unsigned>& cardinalities) const {
    std::ostringstream os;
    os << "{";
    bool first  = true;
    for (unsigned item = 0; item < cardinalities.size(); item++) {
        bool next_void = cardinalities[item] == 0;
        if (!first && !next_void)
            os << ", ";
        if (!next_void) {
            os << code_events_[item] << ":" << cardinalities[item];
            first = false;
        }
    }
    os << "}";
    return os.str();
}

std::vector<std::pair<std::string, unsigned int>> VerticalDatabase::get_multiset_str_list(
        const std::vector<unsigned> &cardinalities) const {
    std::vector<std::pair<std::string, unsigned>> str_list;
    str_list.reserve(cardinalities.size());

    for (unsigned item = 0; item < cardinalities.size(); item++)
        if (cardinalities[item] > 0)
            str_list.emplace_back(code_events_[item], cardinalities[item]);
    return str_list;
}

unsigned long VerticalDatabase::get_negatives_size() const {
    if (negatives_.empty()) return 0;
    return  negatives_[0].size();
}

std::map<unsigned, std::vector<std::vector<std::vector<int>>>>
        VerticalDatabase::get_occurrences(const std::vector<unsigned> &cardinalities, const std::vector<unsigned> &tid_list,
                                                  std::vector<unsigned> *negative_tid_list, bool numeric_intervals, bool minimal_occurrences,
                                                  int time_window) const {
    std::map<unsigned, std::vector<std::vector<std::vector<int>>>> occurrences;
    occurrences[1] = get_class_occurrences(positives_, cardinalities, tid_list, numeric_intervals, nullptr, minimal_occurrences, time_window);
    if (!negatives_.empty())
        occurrences[0] = get_class_occurrences(negatives_, cardinalities, std::vector<unsigned>(), numeric_intervals,
                                           negative_tid_list, minimal_occurrences, time_window);
    else
        occurrences[0] = std::vector<std::vector<std::vector<int>>>();
    return occurrences;
};

void VerticalDatabase::generate_positive_cardinalities() {
    positive_cardinalities_ = std::vector<std::vector<unsigned>>();

    for (auto &positive : positives_) {
        std::vector<unsigned> cardinalities;
        cardinalities.reserve(positive.size());
        for (const std::vector<unsigned> &sequence : positive)
            cardinalities.push_back((unsigned) sequence.size());
        positive_cardinalities_.push_back(cardinalities);
    }
}

std::vector<std::vector<unsigned>> VerticalDatabase::get_minimal_occurrences(
        const std::vector<std::vector<unsigned int>> &occurrences,
        int time_window) {
    std::vector<bool> removed(occurrences.size(), false);
    for (unsigned i = 0; i < occurrences.size(); i++) {
        if (!removed[i]) {
            int min_occ = *std::min(occurrences[i].begin(), occurrences[i].end());
            int max_occ = *std::max(occurrences[i].begin(), occurrences[i].end());
            // std::cout << "I: " << i << std::endl;
            // std::cout << "[" << min_occ << ", " << max_occ << "]" << std::endl;

            if (time_window >= 0 && max_occ - min_occ > time_window)
                removed[i] = true;
            else {
                for (unsigned j = i + 1; j < occurrences.size(); j++) {
                    if (!removed[j]) {
                        int other_min_occ = *std::min(occurrences[j].begin(), occurrences[j].end());
                        int other_max_occ = *std::max(occurrences[j].begin(), occurrences[j].end());

                        // std::cout << "Other: [" << other_min_occ << ", " << other_max_occ << "]" << std::endl;

                        if (time_window >= 0 && other_max_occ - other_min_occ > time_window)
                            removed[j] = true;
                        else {
                           if (is_bound_included(min_occ, max_occ, other_min_occ, other_max_occ)) {
                               removed[i] = true;
                                break;
                           }
                           else if (is_bound_included(other_min_occ, other_max_occ, min_occ, max_occ))
                               removed[j] = true;
                        }
                    }
                }
            }
        }
    }

    std::vector<std::vector<unsigned>> res;
    for (unsigned i = 0; i < removed.size(); i++) {
        if (!removed[i])
            res.push_back(occurrences[i]);
    }

    return res;
}

std::vector<std::vector<unsigned>> VerticalDatabase::get_positives_event_cardinalities() {
    if (!is_generated_positive_cardinalities_)
        generate_positive_cardinalities();
    return positive_cardinalities_;
};

unsigned long VerticalDatabase::get_positives_size() const {
    if (positives_.empty()) return 0;
    return  positives_[0].size();
}

std::vector<std::vector<int>> VerticalDatabase::get_relative_combinations(const std::vector<std::vector<unsigned>> &occurrences, bool numeric_intervals) {
    std::vector<std::vector<int>> relative_combinations;
    for (std::vector<unsigned> occ : occurrences) {
        std::vector<int> relative_combination;
        for (unsigned i = 0; i < occ.size(); i++)
            for (unsigned j = i+1; j < occ.size(); j++) {
                int value = occ[j] - occ[i];
                if (!numeric_intervals)
                    value = sign(value);
                relative_combination.push_back(value);
            }
        relative_combinations.push_back(relative_combination);
    }
    return relative_combinations;
}

std::vector<std::vector<int>> VerticalDatabase::get_relative_occurrences(const std::vector<std::vector<unsigned>> &absolute_occurrences,
                                                                         const std::vector<unsigned> &cardinalities, bool numeric_intervals, bool minimal_occurrences,
                                                                         int time_window) {
    std::vector<std::vector<unsigned>> occurrences;
    unsigned index = 0;
    for (unsigned card : cardinalities) {
        if (card > 0) {
            if (occurrences.empty())
                occurrences = get_combinations(absolute_occurrences[index], card);
            else
                occurrences = merge_combinations(occurrences, get_combinations(absolute_occurrences[index], card));
            index++;
        }
    }

    if (minimal_occurrences)
        occurrences = get_minimal_occurrences(occurrences, time_window);
    return get_relative_combinations(occurrences, numeric_intervals);
}

std::vector<std::vector<int>>
VerticalDatabase::get_sequence_occurrences(const std::vector<std::vector<unsigned>> &sequence, const std::vector<unsigned> &cardinalities,
                                           bool numeric_intervals, bool minimal_occurrences, int time_window) {
    std::vector<std::vector<unsigned>> absolute_occurences =
            get_absolute_occurrences(sequence, cardinalities);
    if (!absolute_occurences.empty())
        return get_relative_occurrences(absolute_occurences, cardinalities, numeric_intervals, minimal_occurrences, time_window);
    return std::vector<std::vector<int>>();
}

std::vector<std::vector<int>> VerticalDatabase::get_sequence_occurrences(const std::vector<std::vector<std::vector<unsigned>>> &class_database, unsigned sequence,
                                                                         const std::vector<unsigned> &cardinalities, bool numeric_intervals, bool minimal_occurrences,
                                                                         int time_window) {
    std::vector<std::vector<unsigned>> absolute_occurences =
            get_absolute_occurrences(class_database, sequence, cardinalities);
    if (!absolute_occurences.empty())
        return get_relative_occurrences(absolute_occurences, cardinalities, numeric_intervals, minimal_occurrences, time_window);
    return std::vector<std::vector<int>>();
}

bool VerticalDatabase::is_item_in_transaction(unsigned int item, unsigned card, unsigned tid) const {
    return positives_.size() > item && positives_[item].size() > tid && positives_[item][tid].size() >= card;
}

bool VerticalDatabase::mask_sequence_multiset(const std::vector<std::vector<unsigned>>& sequence,
                                              const std::vector<unsigned>& cardinalities,
                                              std::vector<std::vector<unsigned>>& masked_sequence) {
    for (unsigned item = 0; item < sequence.size() && item < cardinalities.size(); item++) {
        if (cardinalities[item] > 0) {
            if (sequence[item].size() >= cardinalities[item])
                masked_sequence.push_back(sequence[item]);
            else {
                return false;
            }
        }
    }
    return true;
}

bool VerticalDatabase::mask_sequence_multiset(const std::vector<std::vector<std::vector<unsigned>>>& class_database,
                                              unsigned sequence,
                                              const std::vector<unsigned>& cardinalities,
                                              std::vector<std::vector<unsigned>>& masked_sequence) {
    for (unsigned item = 0; item < class_database.size() && item < cardinalities.size(); item++) {
        if (cardinalities[item] > 0) {
            if (class_database[item][sequence].size() >= cardinalities[item])
                masked_sequence.push_back(class_database[item][sequence]);
            else {
                return false;
            }
        }
    }
    return true;
}

bool VerticalDatabase::is_bound_included(int min, int max, int o_min, int o_max) {
    return min <= o_min && max >= o_max && (min!= o_min || max != o_max);
}

bool VerticalDatabase::load_file(const std::string &filename, std::vector<std::vector<std::vector<unsigned>>> &base) {
    std::ifstream fin;
    fin.open(filename);
    if (!fin) {
        std::cerr << "Unable to open " << filename << "." << std::endl;
        return false;
    }

    return parse_line_file(fin, base);
}

bool VerticalDatabase::load_negatives(const std::string &filename) {
    return load_file(filename, negatives_);
}

bool VerticalDatabase::load_positives(const std::string &filename) {
    return load_file(filename, positives_);
}

std::vector<std::vector<unsigned>> VerticalDatabase::merge_combinations(
        const std::vector<std::vector<unsigned>>& head,
        const std::vector<std::vector<unsigned>>& tail) {
    std::vector<std::vector<unsigned>> combinations;
    for (const std::vector<unsigned> &hc : head) {
        for (std::vector<unsigned> tc : tail) {
            std::vector<unsigned> comb(hc);
            comb.insert(comb.end(), tc.begin(), tc.end());
            combinations.push_back(comb);
        }
    }
    return combinations;
}

bool VerticalDatabase::parse_line_file(std::ifstream &fin, std::vector<std::vector<std::vector<unsigned>>> &base) {
    std::map<unsigned, unsigned > sequences;

    std::string line;
    getline(fin,line);

    unsigned sid = 0;
    std::string label;
    unsigned date, event, index;

    std::map<unsigned, unsigned>::iterator it_sequence;
    std::map<std::string, unsigned>::iterator it_event;

    while (!fin.eof()) {
        std::istringstream str_line(line);

        while (str_line.good()) {
            str_line >> label;
            str_line >> date;

            if (str_line) {
                if ((it_event = events_.find(label)) != events_.end())
                    event = it_event->second;
                else {
                    event = (unsigned) events_.size();
                    events_[label] = event;
                    code_events_.push_back(label);
                }

                while (base.size() <= event)
                    base.emplace_back(sequences.size(), std::vector<unsigned>());

                if ((it_sequence = sequences.find(sid)) != sequences.end())
                    index = it_sequence->second;
                else {
                    index = (unsigned) sequences.size();
                    sequences[sid] = index;
                }

                while (base[event].size() <= index)
                    base[event].push_back(std::vector<unsigned>());

                base[event][index].push_back(date);
            }
        }

        getline(fin, line);
        sid++;
    }

    for (auto &i : base) {
        while (i.size() < sequences.size())
            i.emplace_back();
    }

    return true;
}

int VerticalDatabase::sign(int a) {
    if (a > 0) return 1;
    if (a == 0) return 0;
    return -1;
}

void VerticalDatabase::set_item_mapping(const std::vector<std::string> &item_mapping) {
    code_events_ = item_mapping;
    for (unsigned i = 0; i < code_events_.size(); i++)
        events_[code_events_[i]] = i;
}

void VerticalDatabase::set_positives(const std::vector<std::vector<std::vector<unsigned>>> &positives) {
    this->positives_ = positives;
}

void VerticalDatabase::set_negatives(const std::vector<std::vector<std::vector<unsigned>>> &negatives) {
    this->negatives_ = negatives;
}