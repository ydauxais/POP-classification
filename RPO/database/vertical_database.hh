#ifndef GDE_VERTICAL_DATABASE_HH
#define GDE_VERTICAL_DATABASE_HH

#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <vector>

#include "database.hh"

class VerticalDatabase : public Database {

private:
    std::vector<std::string> code_events_;
    std::map<std::string, unsigned> events_;

    bool is_generated_positive_cardinalities_;
    std::vector<std::vector<unsigned>> positive_cardinalities_;

    std::vector<std::vector<std::vector<unsigned>>> positives_;
    std::vector<std::vector<std::vector<unsigned>>> negatives_;

public:
    VerticalDatabase(): is_generated_positive_cardinalities_(false) {};
    ~VerticalDatabase() override = default;
    VerticalDatabase(const std::string& positives, const std::string& negatives);

    std::string get_event_name(unsigned code) const override {
        return code_events_[code];
    }

    std::vector<std::vector<unsigned>> get_positives_event_cardinalities() override;

    std::string get_multiset_str(const std::vector<unsigned>& cardinalities) const override;
    std::vector<std::pair<std::string, unsigned int>> get_multiset_str_list(const std::vector<unsigned> &cardinalities) const override;
    std::map<unsigned, std::vector<std::vector<std::vector<int>>>>
            get_occurrences(const std::vector<unsigned> &cardinalities, const std::vector<unsigned> &tid_list,
                                        std::vector<unsigned> *negative_tid_list, bool numeric_intervals, bool minimal_occurrences,
                                        int time_window) const override;

    unsigned long get_negatives_size() const override;
    unsigned long get_positives_size() const override;

    static
    std::vector<std::vector<int>>
    get_sequence_occurrences(const std::vector<std::vector<unsigned>> &sequence, const std::vector<unsigned> &cardinalities,
                                 bool numeric_intervals, bool minimal_occurrences=false, int time_window=-1);

    static
    std::vector<std::vector<int>>
    get_sequence_occurrences(const std::vector<std::vector<std::vector<unsigned>>> &class_database, unsigned sequence,
                                 const std::vector<unsigned> &cardinalities, bool numeric_intervals, bool minimal_occurrences=false,
                                 int time_window=-1);

    const std::vector<std::string> get_vocabulary() const override {
        return code_events_;
    }

    bool is_item_in_transaction(unsigned int item, unsigned card, unsigned tid) const override;

    bool load_negatives(const std::string& filename) override;
    bool load_positives(const std::string& filename) override;

    void set_item_mapping(const std::vector<std::string>& item_mapping) override;
    void set_negatives(const std::vector<std::vector<std::vector<unsigned>>>& negatives) override;
    void set_positives(const std::vector<std::vector<std::vector<unsigned>>>& positives) override;

private:
    void generate_positive_cardinalities();


    static
    std::vector<std::vector<unsigned>> get_absolute_occurrences (
            const std::vector<std::vector<unsigned>>& sequence,
            const std::vector<unsigned>& cardinalities
    );

    static
    std::vector<std::vector<unsigned>> get_absolute_occurrences (
            const std::vector<std::vector<std::vector<unsigned>>>& class_database,
            unsigned sequence,
            const std::vector<unsigned>& cardinalities
    );

    std::vector<std::vector<std::vector<int>>>
    get_class_occurrences(const std::vector<std::vector<std::vector<unsigned>>> &class_database,
                              const std::vector<unsigned> &cardinalities, bool numeric_intervals,
                              std::vector<unsigned> *negative_tid_list, bool minimal_occurrences, int time_window) const;

    std::vector<std::vector<std::vector<int>>>
    get_class_occurrences(const std::vector<std::vector<std::vector<unsigned>>> &class_database,
                              const std::vector<unsigned> &cardinalities, const std::vector<unsigned> &tid_list,
                              bool numeric_interval, std::vector<unsigned int> *negative_tid_list = nullptr,
                              bool minimal_occurrences=false, int time_window=-1) const;


    std::vector<std::vector<std::vector<int>>>
    get_class_occurrences_from_tid_list(const std::vector<std::vector<std::vector<unsigned>>> &class_database,
                                            const std::vector<unsigned> &cardinalities, const std::vector<unsigned> &tid_list,
                                            bool numeric_intervals, bool minimal_occurrences, int time_window) const;

    static std::vector<std::vector<unsigned>> get_combinations(
            const std::vector<unsigned>& absolute_occurrences, unsigned card);

    static std::vector<std::vector<unsigned>> get_combinations(
            const std::vector<unsigned>& absolute_occurrences, unsigned index, unsigned card);

    static std::vector<std::vector<unsigned>> get_minimal_occurrences(
            const std::vector<std::vector<unsigned int>> &occurrences, int time_window = -1
    );

    static
    std::vector<std::vector<int>>
    get_relative_combinations(const std::vector<std::vector<unsigned>> &occurrences, bool numeric_intervals);

    static
    std::vector<std::vector<int>> get_relative_occurrences(const std::vector<std::vector<unsigned>> &absolute_occurrences,
                                                               const std::vector<unsigned> &cardinalities,
                                                               bool numeric_intervals, bool minimal_occurrences,
                                                               int time_window = -1);

    static
    bool is_bound_included(int min, int max, int o_min, int o_max);

    bool load_file(const std::string& filename, std::vector<std::vector<std::vector<unsigned>>>& base);
    bool parse_line_file(std::ifstream& fin, std::vector<std::vector<std::vector<unsigned>>>& base);

    static
    bool mask_sequence_multiset(const std::vector<std::vector<unsigned>>& sequence,
                                const std::vector<unsigned>& cardinalities,
                                std::vector<std::vector<unsigned>>& masked_sequence);

    static
    bool mask_sequence_multiset(const std::vector<std::vector<std::vector<unsigned>>>& class_database,
                                unsigned sequence, const std::vector<unsigned>& cardinalities,
                                std::vector<std::vector<unsigned>>& masked_sequence);

    static std::vector<std::vector<unsigned>> merge_combinations(
            const std::vector<std::vector<unsigned>>& head,
            const std::vector<std::vector<unsigned>>& tail);

    static int sign(int a);
};

#endif //GDE_VERTICAL_DATABASE_HH
