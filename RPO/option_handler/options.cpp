#include "options.hh"

using namespace General;

const std::string Options::basename_(const std::string& p)
{
#ifdef HAVE_BOOST_FILESYSTEM
    return boost::filesystem::path(p).stem().string();
#else
    size_t start = p.find_last_of('/');
    if(start == std::string::npos)
        start = 0;
    else
        ++start;
    return p.substr(start);
#endif
}

bool Options::notify(int argc, char **const argv, const po::options_description &hidden,
                     const po::options_description &desc, po::positional_options_description &p,
                     po::variables_map &vm) {
    po::options_description options;
    options.add(desc);
    options.add(hidden);
    bool need_help = false;

    try {
        po::store(po::command_line_parser(argc, (const char *const *) argv).
                          options(options).
                          positional(p).
                          run(),
                  vm);
    }
    catch(po::error &ex) {
        std::cout << ex.what() << std::endl << std::endl;
        need_help = true;
    }

    if (!need_help && !vm.count("help")) {
        try {
            po::notify(vm);
        }
        catch (po::error &ex) {
            std::cout << ex.what() << std::endl;
            need_help = true;
        }
    }
    else
        need_help = true;

    if(need_help)
    {
        std::cout << make_usage_string(basename_(argv[0]), hidden, desc, p) << std::endl;
        return false;
    }
    return true;
}

bool Options::parse(int argc, char **const argv) {
    std::vector<std::string> config_fnames;
    po::options_description general("General Options");
    general.add_options()
            ("help", "Display this help message")
            ("top_lattice,t", "Browse mutliset space from the maximal one to the void set")
            ("closed,c", "Extract patterns only from closed multisets")
            ("min_length", po::value<unsigned>(&cp_options.minimal_length_),
                    "Minimal number of items contained by an extracted pattern")
            ("max_length", po::value<unsigned>(&cp_options.maximal_length_),
                    "Maximal number of items contained by an extracted pattern")
            ("numeric_intervals,n", "Extract numeric intervals")
            ("output_tid_lists,l", "Option to output the tid lists describing the subgroups matched by the patterns")
            ("relevant,r", "Extract only relevant episodes")
            ("unsupervised", "Extract episodes without discriminant constraints")
            ;

    po::options_description desc;
    desc.add(general);

    po::options_description hidden("Positional Options (required)");
    hidden.add_options()
            ("positive_file,pos", po::value<std::string>(&positives)->required(),
             "input file containing the positive dataset to mine (string)\n"
                     "positional : positive_file")
            ("negative_file,neg", po::value<std::string>(&negatives),
             "input file containing the negative dataset to mine (string)\n"
                     "positional : negative_file")
            ("fmin,f", po::value<float>(&fmin),
             "minimal frequency threshold (number)\n"
                     "Number of sequences if >= 1 (support)\n"
                     "Percent of positive sequences number else\n"
                     "positional fmin")
            ("gmin,g", po::value<float>(&gmin),
             "minimal growth rate threshold (number)\n"
             "positional gmin");

    po::positional_options_description p;
    p.add("positive_file", 1);
    p.add("negative_file", 1);

    po::variables_map vm;

    if (!notify(argc, argv, hidden, desc, p, vm)) return false;
    if(!parse_vm(vm)) return false;
    return parse_files();
}

bool Options::parse_files() {
    cp_options.base_ = std::make_shared<VerticalDatabase>();
    if (!cp_options.base_->load_positives(positives)) {
        std::cerr << "Unable to parse " << positives << std::endl;
        return false;
    }
    if (!cp_options.unsupervised_ && !cp_options.base_->load_negatives(negatives)) {
        std::cerr << "Unable to parse " << negatives << std::endl;
        return false;
    }

    setMinsup(cp_options.base_->get_positives_size());
    cp_options.minimal_growth_rate_ = gmin;

    return true;
}

bool Options::parse_vm(const po::variables_map& vm){
    cp_options.closed_ = vm.count("closed") > 0;
    cp_options.relevant_ = vm.count("relevant") > 0;
    cp_options.branch_on_min_ = vm.count("top_lattice") == 0;
    cp_options.numeric_intervals_ = vm.count("numeric_intervals") > 0;
    cp_options.output_tid_lists_ = vm.count("output_tid_lists") > 0;
    cp_options.unsupervised_ = vm.count("unsupervised") > 0;
    cp_options.solutions(0);
    return true;
}

void Options::setMinsup(unsigned long nb_seq) {
    if (fmin >= 1)
        cp_options.minimal_support_ = (unsigned) ceil(fmin);
    else cp_options.minimal_support_ = (unsigned) ceil(fmin * nb_seq);
}

const std::vector<std::string> Options::get_unlimited_positional_args(const po::positional_options_description& p)
{
    assert(p.max_total_count() == std::numeric_limits<unsigned>::max());
    std::vector<std::string> parts;
    const int MAX = 1000;
    const std::string &last = p.name_for_position(MAX);

    for(unsigned i = 0; i < MAX; ++i)
    {
        const std::string &cur = p.name_for_position(i);
        if(cur == last)
        {
            parts.push_back(cur);
            parts.push_back('[' + cur + ']');
            parts.emplace_back("...");
            return parts;
        }
        parts.push_back(cur);
    }
    return parts;
}

const std::string Options::make_help_description(const std::string& program_name) {
    std::ostringstream oss;
    oss << "Description:" << std::endl;
    oss << program_name;
    //TODO Complete the description
    oss << " Extract discriminant partially ordered patterns." << std::endl;

    return oss.str();
}

const std::string Options::make_usage_string(
        const std::string& program_name,
        const po::options_description& hidden,
        const po::options_description& desc,
        const po::positional_options_description& p)
{
    std::vector<std::string> parts;
    parts.emplace_back("Usage: ");
    parts.push_back(program_name);
    size_t N = p.max_total_count();
    if(N == std::numeric_limits<unsigned>::max())
    {
        std::vector<std::string> args = get_unlimited_positional_args(p);
        parts.insert(parts.end(), args.begin(), args.end());
    }
    else
    {
        for(size_t i = 0; i < N; ++i)
        {
            parts.push_back(p.name_for_position((int) i));
        }
    }
    if(!desc.options().empty())
    {
        parts.emplace_back("[options]");
    }
    std::ostringstream oss;
    oss << make_help_description(program_name) << std::endl;
    std::copy(
            parts.begin(),
            parts.end(),
            std::ostream_iterator<std::string>(oss, " "));
    oss << std::endl << hidden;
    oss << std::endl << desc;
    return oss.str();
}