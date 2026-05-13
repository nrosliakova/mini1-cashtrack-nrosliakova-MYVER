#pragma once

#include "transaction.hpp"
#include <string>
#include <vector>
#include <map>
#include <optional>

namespace cashtrack {

enum class GroupBy {
    Category,
    Tag,
    Month,
};

struct ReportOptions {
    std::optional<std::string> from_date;   // YYYY-MM-DD, inclusive (mostly)
    std::optional<std::string> to_date;     // YYYY-MM-DD, inclusive (mostly)
    std::optional<std::string> category;
    std::optional<std::string> tag;
    GroupBy                    group_by = GroupBy::Category;
};

// Generates reports. Does filtering, aggregation, ASCII chart rendering,
// CSV export, and a partial "format for email" path that was added
// during one of those Sunday-evening commits.
class Reporter {
public:
    Reporter() = default;

    // The grand entrypoint.
    std::string make_report(const std::vector<Transaction>& txns,
                            const ReportOptions& opts) const;

    // Export current filtered transactions to a CSV string.
    std::string export_csv(const std::vector<Transaction>& txns,
                           const ReportOptions& opts) const;

    // Remove duplicate transactions in-place by (date, description, amount).
    // Used at the end of import; also exposed for the "cleanup" subcommand.
    void deduplicate(std::vector<Transaction>& txns) const;

    // --- Helpers (public for testing) ---
    static std::vector<Transaction> filter(const std::vector<Transaction>& txns,
                                           const ReportOptions& opts);
    static std::map<std::string, double> aggregate(
        const std::vector<Transaction>& txns,
        GroupBy group_by);
    static std::string render_ascii_chart(
        const std::map<std::string, double>& aggregated);

private:
    // TODO(alice): refactor when we add email reports. (still TODO)
    std::string format_for_email(const std::map<std::string, double>&) const;
};

}  // namespace cashtrack
