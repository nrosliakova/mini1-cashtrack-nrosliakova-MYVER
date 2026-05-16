#include "cashtrack/importer.hpp"
#include "cashtrack/categories.hpp"

#include <fstream>
#include <sstream>
#include <stdexcept>

namespace cashtrack {

void Importer::strip_bom(std::string& s) {
    // UTF-8 BOM is 0xEF 0xBB 0xBF.
    if (s.size() >= 3 &&
        static_cast<unsigned char>(s[0]) == 0xEF &&
        static_cast<unsigned char>(s[1]) == 0xBB &&
        static_cast<unsigned char>(s[2]) == 0xBF) {
        s.erase(0, 3);
    }
}

std::vector<std::string> Importer::split_csv_row(const std::string& row) {
    // Splits on comma. Does NOT handle quoted fields with embedded commas;
    // see Known Limitations.
    std::vector<std::string> cols;
    std::string cur;
    for (char c : row) {
        if (c == ',') { cols.push_back(cur); cur.clear(); }
        else            cur.push_back(c);
    }
    cols.push_back(cur);
    return cols;
}

// Try ISO-8601 first; otherwise fall back to MM/DD/YYYY.
static std::string normalize_date(const std::string& in) {
    if (in.size() >= 10 && in[4] == '-' && in[7] == '-') return in;
    if (in.size() == 10 && in[2] == '/' && in[5] == '/') {
        std::string mm = in.substr(0, 2);
        std::string dd = in.substr(3, 2);
        std::string yy = in.substr(6, 4);
        return yy + "-" + mm + "-" + dd;
    }
    return in;
}

Transaction Importer::parse_row(const std::vector<std::string>& cols) {
    if (cols.size() < 3) throw std::runtime_error("row has < 3 columns");
    return Transaction(normalize_date(cols[0]), cols[1], std::stod(cols[2]));
}

bool Importer::is_duplicate(const Transaction& t,
                            const std::vector<Transaction>& existing) {
    for (const auto& e : existing) {
        if (e.date() == t.date() &&
            e.description() == t.description() &&
            e.amount() == t.amount()) {
            return true;
        }
    }
    return false;
}

ImportStats Importer::import_file(const std::string& path,
                                  std::vector<Transaction>& existing) {
    std::ifstream f(path);
    if (!f) throw std::runtime_error("cannot open: " + path);

    ImportStats stats;
    std::string line;

    // Header.
    if (!std::getline(f, line)) return stats;
    if (!line.empty() && line.back() == '\r') line.pop_back();
    // BOM handling: only the header is checked. (Subsequent CRLFs are not.)
    strip_bom(line);

    while (std::getline(f, line)) {
        if (line.empty()) continue;
        auto cols = split_csv_row(line);
        if (cols.size() < 3) continue;

        Transaction t;
        try {
            t = parse_row(cols);
        } catch (const std::exception&) {
            continue;
        }
        t.categorize();

        if (is_duplicate(t, existing)) {
            ++stats.duplicates;
            continue;
        }
        existing.push_back(t);
        ++stats.imported;
        if (t.category() != "uncategorized") ++stats.categorized;
        else                                  ++stats.uncategorized;
        stats.total += t.amount();
        if (stats.from_date.empty() || t.date() < stats.from_date)
            stats.from_date = t.date();
        if (stats.to_date.empty() || t.date() > stats.to_date)
            stats.to_date = t.date();
    }
    return stats;
}

}  // namespace cashtrack
