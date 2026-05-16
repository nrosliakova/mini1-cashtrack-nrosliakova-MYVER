#pragma once

#include "transaction.hpp"
#include <string>
#include <vector>

namespace cashtrack {

struct ImportStats {
    int imported      = 0;
    int categorized   = 0;
    int uncategorized = 0;
    int duplicates    = 0;
    double total      = 0.0;
    std::string from_date;
    std::string to_date;
};

// Reads a CSV file, parses rows, deduplicates against the existing
// database, guesses categories for new ones, and returns the import
// stats. Yes, that's a lot — it grew this way over time.
class Importer {
public:
    Importer() = default;

    // Main entry point.
    // `existing` is consulted for duplicate detection.
    // The new transactions are appended.
    ImportStats import_file(const std::string& path,
                            std::vector<Transaction>& existing);

    // --- Helpers (public for testing) ---
    static std::vector<std::string> split_csv_row(const std::string& row);
    static Transaction              parse_row(const std::vector<std::string>& cols);
    static bool                     is_duplicate(const Transaction& t,
                                                 const std::vector<Transaction>& existing);

private:
    // Strips UTF-8 BOM if present at the start of the first line.
    static void strip_bom(std::string& s);
};

}  // namespace cashtrack
