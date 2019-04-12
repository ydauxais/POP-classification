#include "python_functions.hh"

unsigned base_size(CPEngineOptions& options) {
    return options.base_->get_positives_size();
}

bool parse_files(CPEngineOptions& options, const std::string& positives, const std::string& negatives) {
    init_database(options);
    if (!options.base_->load_positives(positives)) {
        std::cerr << "Unable to parse " << positives << std::endl;
        return false;
    }
    if (!options.base_->load_negatives(negatives)) {
        std::cerr << "Unable to parse " << negatives << std::endl;
        return false;
    }
    return true;
}

int run(const CPEngineOptions& options) {
    Script::run<CPEngine,DFS,CPEngineOptions>(options);
    if (options.episodes_ != nullptr)
        return options.episodes_->size();
    return 0;
}

void init_database(CPEngineOptions& options) {
    options.base_ = std::make_shared<VerticalDatabase>();
}

void set_item_mapping(CPEngineOptions& options,
                      const std::vector<std::string>& item_mapping){
    options.base_->set_item_mapping(item_mapping);
}

void set_negatives(CPEngineOptions& options,
                   const std::vector<std::vector<std::vector<unsigned>>>& negatives){
    options.base_->set_negatives(negatives);
}
void set_positives(CPEngineOptions& options,
                   const std::vector<std::vector<std::vector<unsigned>>>& positives){
    options.base_->set_positives(positives);
}