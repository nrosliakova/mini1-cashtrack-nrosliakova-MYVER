#include "cashtrack/categories.hpp"
#include "cashtrack/importer.hpp"
#include "cashtrack/reporter.hpp"
#include "cashtrack/storage.hpp"
#include "cashtrack/transaction.hpp"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace cashtrack {

static int cmd_import(int argc, char** argv) {
    if (argc < 3) {
        std::cerr << "usage: cashtrack import <file>\n";
        return 2;
    }
    Storage storage;
    auto existing = storage.load();

    Importer imp;
    auto stats = imp.import_file(argv[2], existing);

    Reporter r;
    r.deduplicate(existing);
    storage.save(existing);

    std::cout << "Imported " << stats.imported << " transactions.\n"
              << "  - " << stats.categorized   << " categorized\n"
              << "  - " << stats.uncategorized << " uncategorized\n"
              << "  - " << stats.duplicates    << " duplicates skipped\n"
              << "\n"
              << "Total amount: " << stats.total << "\n";
    if (!stats.from_date.empty()) {
        std::cout << "Date range: " << stats.from_date << " → "
                  << stats.to_date << "\n";
    }
    return 0;
}

static int cmd_list(int argc, char** argv) {
    std::string from, to, tag_filter, category_filter;
    for (int i = 2; i + 1 < argc; ++i) {
        std::string a = argv[i];
        if      (a == "--from")     from            = argv[++i];
        else if (a == "--to")       to              = argv[++i];
        else if (a == "--tag")      tag_filter      = argv[++i];
        else if (a == "--category") category_filter = argv[++i];
    }

    Storage storage;
    auto txns = storage.load();
    size_t shown = 0;
    for (auto& t : txns) {
        if (!from.empty() && t.date() <  from) continue;
        if (!to.empty()   && t.date() >= to)   continue;
        if (!category_filter.empty() && t.category() != category_filter) continue;
        if (!tag_filter.empty() && !t.matches_tag(tag_filter)) continue;
        std::cout << t.format_for_list() << "\n";
        ++shown;
    }
    std::cout << shown << " transactions.\n";
    return 0;
}

static int cmd_sort(int argc, char** argv) {
    if (argc < 3) {
        std::cerr << "usage: cashtrack sort <file> [--column COL]\n";
        return 2;
    }
    std::string col = "date";
    bool numeric = false;
    for (int i = 3; i + 1 < argc; ++i) {
        std::string a = argv[i];
        if      (a == "--column")  col     = argv[++i];
        else if (a == "--numeric") numeric = true;
    }

    std::ifstream in(argv[2]);
    if (!in) { std::cerr << "cannot open: " << argv[2] << "\n"; return 1; }
    std::string header;
    std::getline(in, header);
    if (!header.empty() && header.back() == '\r') header.pop_back();

    std::vector<Transaction> txns;
    std::string line;
    while (std::getline(in, line)) {
        if (line.empty()) continue;
        try {
            auto cols = Importer::split_csv_row(line);
            txns.push_back(Importer::parse_row(cols));
        } catch (...) { continue; }
    }

    int col_idx = 0;
    if      (col == "date")        col_idx = 0;
    else if (col == "description") col_idx = 1;
    else if (col == "amount")      col_idx = 2;
    else { std::cerr << "unknown column: " << col << "\n"; return 2; }

    auto key_of = [col_idx](const Transaction& t) -> std::string {
        if (col_idx == 0) return t.date();
        if (col_idx == 1) return std::string(t.description());
        // amount: stringify so sort is identical regardless of column.
        return std::to_string(t.amount());
    };
    if (col_idx == 2 && numeric) {
        std::sort(txns.begin(), txns.end(),
                  [](const Transaction& a, const Transaction& b) {
                      return a.amount() < b.amount();
                  });
    } else {
        std::sort(txns.begin(), txns.end(),
                  [&](const Transaction& a, const Transaction& b) {
                      return key_of(a) < key_of(b);
                  });
    }

    std::cout << "date,description,amount\n";
    for (auto& t : txns) {
        std::cout << t.date() << "," << t.description() << ","
                  << t.amount() << "\n";
    }
    return 0;
}

static int cmd_split(int argc, char** argv) {
    if (argc < 3) {
        std::cerr << "usage: cashtrack split <file> --rows N --out DIR\n";
        return 2;
    }
    std::string in_path = argv[2];
    int rows_per_chunk = 1000;
    std::string out_dir = ".";
    for (int i = 3; i + 1 < argc; ++i) {
        std::string a = argv[i];
        if      (a == "--rows") rows_per_chunk = std::stoi(argv[++i]);
        else if (a == "--out")  out_dir        = argv[++i];
    }

    std::ifstream in(in_path);
    if (!in) { std::cerr << "cannot open: " << in_path << "\n"; return 1; }

    namespace fs = std::filesystem;
    fs::create_directories(out_dir);
    std::string base = fs::path(in_path).stem().string();

    std::string header;
    if (!std::getline(in, header)) {
        std::cerr << "empty file\n"; return 1;
    }

    int chunk_idx = 0;
    int rows_in_chunk = 0;
    std::ofstream out;
    std::string line;
    auto open_next = [&]() {
        ++chunk_idx;
        std::ostringstream name;
        name << base << "_" << std::setw(3) << std::setfill('0') << chunk_idx << ".csv";
        fs::path p = fs::path(out_dir) / name.str();
        out.close();
        out.open(p);
        out << header << "\n";
        rows_in_chunk = 0;
    };
    open_next();

    while (std::getline(in, line)) {
        if (rows_in_chunk >= rows_per_chunk) open_next();
        out << line << "\n";
        ++rows_in_chunk;
    }
    std::cout << "Wrote " << chunk_idx << " chunks into " << out_dir << "\n";
    return 0;
}

static int cmd_export(int argc, char** argv) {
    if (argc < 3) {
        std::cerr << "usage: cashtrack export <file>\n";
        return 2;
    }
    Storage storage;
    auto txns = storage.load();

    std::ofstream out(argv[2]);
    if (!out) { std::cerr << "cannot write: " << argv[2] << "\n"; return 1; }
    out << "date,description,amount,category\n";
    for (auto& t : txns) {
        out << t.date() << "," << t.description() << ","
            << t.amount() << "," << t.category() << "\n";
    }
    std::cout << "Exported " << txns.size() << " transactions to "
              << argv[2] << ".\n";
    return 0;
}

static int cmd_report(int argc, char** argv) {
    ReportOptions opts;
    for (int i = 2; i + 1 < argc; ++i) {
        std::string a = argv[i];
        if      (a == "--from")     opts.from_date = argv[++i];
        else if (a == "--to")       opts.to_date   = argv[++i];
        else if (a == "--category") opts.category  = argv[++i];
        else if (a == "--tag")      opts.tag       = argv[++i];
        else if (a == "--by") {
            std::string g = argv[++i];
            if (g == "tag")   opts.group_by = GroupBy::Tag;
            if (g == "month") opts.group_by = GroupBy::Month;
        }
    }
    Storage storage;
    auto txns = storage.load();
    Reporter r;
    std::cout << r.make_report(txns, opts);
    return 0;
}

static int cmd_categories() {
    std::cout << "Categorization rules (in match order):\n\n";
    for (const auto& [pat, cat] : category_rules()) {
        std::cout << "  " << std::left << std::setw(42) << pat
                  << " → " << cat << "\n";
    }
    std::cout << "\n" << category_rules().size()
              << " rules. To customize, edit categories.cpp and rebuild.\n";
    return 0;
}

static int cmd_help() {
    std::cout <<
        "cashtrack — a personal-finance CLI\n"
        "\n"
        "Usage: cashtrack <command> [options]\n"
        "\n"
        "Commands:\n"
        "  import <file>          Import transactions from a CSV file\n"
        "  list                   List all transactions\n"
        "  report                 Generate a report (--by category|tag|month)\n"
        "  split <file>           Split a CSV into chunks (--rows N --out DIR)\n"
        "  sort <file>            Sort a CSV by a column (--column COL)\n"
        "  export <file>          Export current database to CSV\n"
        "  categories             Show categorization rules\n"
        "  --help                 Show this help\n"
        "  --version              Show version\n";
    return 0;
}

static int cmd_version() {
    std::cout << "cashtrack 0.3.0 (2025-05-01)\n";
    return 0;
}

int run(int argc, char** argv) {
    if (argc < 2) return cmd_help();

    std::string cmd = argv[1];
    if (cmd == "import")              return cmd_import(argc, argv);
    if (cmd == "list")                return cmd_list(argc, argv);
    if (cmd == "report")              return cmd_report(argc, argv);
    if (cmd == "sort")                return cmd_sort(argc, argv);
    if (cmd == "split")               return cmd_split(argc, argv);
    if (cmd == "export")              return cmd_export(argc, argv);
    if (cmd == "categories")          return cmd_categories();
    if (cmd == "--help"    || cmd == "-h") return cmd_help();
    if (cmd == "--version" || cmd == "-v") return cmd_version();

    std::cerr << "unknown command: " << cmd << "\n";
    return 2;
}

}  // namespace cashtrack
