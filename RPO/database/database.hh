#ifndef GENERALIZED_DISCRIMINANT_EPISODES_DATABASE_HH
#define GENERALIZED_DISCRIMINANT_EPISODES_DATABASE_HH

#include <map>
#include <string>
#include <vector>

class Database {

public:
    Database() = default;;
    virtual ~Database() = default;;

    virtual unsigned long get_negatives_size() const = 0;
    virtual std::string get_event_name(unsigned code) const = 0;
    virtual std::map<unsigned, std::vector<std::vector<std::vector<int>>>>
            get_occurrences(const std::vector<unsigned> &cardinalities, const std::vector<unsigned> &tid_list,
                                        std::vector<unsigned> *negative_tid_list, bool numeric_intervals,
                                        bool minimal_occurrences,
                                        int time_window) const = 0;
    virtual std::vector<std::vector<unsigned>> get_positives_event_cardinalities() = 0;
    virtual unsigned long get_positives_size() const = 0;
    virtual const std::vector<std::string> get_vocabulary() const = 0;

    virtual bool is_item_in_transaction(unsigned int item, unsigned card, unsigned tid) const = 0;

    virtual bool load_negatives(const std::string& filename) = 0;
    virtual bool load_positives(const std::string& filename) = 0;

    virtual std::string get_multiset_str(const std::vector<unsigned>& cardinalities) const = 0;
    virtual std::vector<std::pair<std::string, unsigned int>> get_multiset_str_list(
            const std::vector<unsigned> &cardinalities) const = 0;

    virtual void set_item_mapping(const std::vector<std::string>& item_mapping) = 0;
    virtual void set_negatives(const std::vector<std::vector<std::vector<unsigned>>>& negatives) = 0;
    virtual void set_positives(const std::vector<std::vector<std::vector<unsigned>>>& positives) = 0;
};

#endif //GENERALIZED_DISCRIMINANT_EPISODES_DATABASE_HH
