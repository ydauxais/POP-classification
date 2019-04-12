#ifndef GDE_PYTHON_FUNCTIONS_HH
#define GDE_PYTHON_FUNCTIONS_HH

#include "cp_engine.hh"
#include "options.hh"

unsigned base_size(CPEngineOptions& options);

void init_database(CPEngineOptions& options);
void set_item_mapping(CPEngineOptions& options,
        const std::vector<std::string>& item_mapping);
void set_negatives(CPEngineOptions& options,
                   const std::vector<std::vector<std::vector<unsigned>>>& negatives);
void set_positives(CPEngineOptions& options,
        const std::vector<std::vector<std::vector<unsigned>>>& positives);

bool parse_files(CPEngineOptions& options, const std::string& positives, const std::string& negatives);
int run(const CPEngineOptions& options);

#endif //GDE_PYTHON_FUNCTIONS_HH
