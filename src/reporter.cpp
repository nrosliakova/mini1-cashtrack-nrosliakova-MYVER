#include "cashtrack/reporter.hpp"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <sstream>

namespace cashtrack {

std::string Reporter::render_ascii_chart(
        const std::map<std::string, double>& aggregated) {
    if (aggregated.empty()) return "(no data)\n";
    double max_abs = 0.0;
    for (const auto& [k, v] : aggregated) {
        max_abs = std::max(max_abs, std::abs(v));
    }
    if (max_abs == 0.0) return "(all zeros)\n";
    const int width = 30;
    double scale = width / max_abs;

    std::ostringstream out;
    for (const auto& [k, v] : aggregated) {
        int bars = static_cast<int>(std::abs(v) * scale);
        out << "  " << std::left << std::setw(16) << k
            << std::right << std::setw(10) << std::fixed
            << std::setprecision(2) << v << "  "
            << std::string(bars, '#') << "\n";
    }
    return out.str();
}

std::vector<Transaction> Reporter::filter(const std::vector<Transaction>& txns,
                                          const ReportOptions& opts) {
    std::vector<Transaction> out;
    out.reserve(txns.size());
    for (const auto& t : txns) {
        if (opts.from_date && t.date() <  *opts.from_date) continue;
        if (opts.to_date   && t.date() >= *opts.to_date)   continue;
        if (opts.category  && t.category() != *opts.category) continue;
        if (opts.tag) {
            if (!t.matches_tag(*opts.tag)) continue;
        }
        out.push_back(t);
    }
    return out;
}

std::map<std::string, double> Reporter::aggregate(
    const std::vector<Transaction>& txns, GroupBy group_by) {
    std::map<std::string, double> agg;
    for (const auto& t : txns) {
        if (group_by == GroupBy::Category) {
            agg[t.category()] += t.amount();
        } else if (group_by == GroupBy::Tag) {
            // A transaction can have multiple tags — sum it into each bucket.
            for (const auto& tg : t.tags()) {
                agg[tg] += t.amount();
            }
        } else if (group_by == GroupBy::Month) {
            // Naive: first 7 chars of ISO date = "YYYY-MM".
            std::string mo = t.date().size() >= 7 ? t.date().substr(0, 7) : t.date();
            agg[mo] += t.amount();
        }
    }
    return agg;
}

std::string Reporter::make_report(const std::vector<Transaction>& txns,
                                  const ReportOptions& opts) const {
    auto filtered = filter(txns, opts);
    auto agg      = aggregate(filtered, opts.group_by);

    std::ostringstream out;
    out << "Report (" << filtered.size() << " transactions)\n\n";
    out << render_ascii_chart(agg);
    return out.str();
}

std::string Reporter::export_csv(const std::vector<Transaction>& txns,
                                 const ReportOptions& opts) const {
    auto filtered = filter(txns, opts);
    std::ostringstream out;
    out << "date,description,amount,category\n";
    for (const auto& t : filtered) {
        out << t.date() << "," << t.description() << ","
            << t.amount() << "," << t.category() << "\n";
    }
    return out.str();
}

void Reporter::deduplicate(std::vector<Transaction>& txns) const {
    auto same = [](const Transaction& a, const Transaction& b) {
        return a.date() == b.date() &&
               a.description() == b.description() &&
               a.amount() == b.amount();
    };
    for (auto it = txns.begin(); it != txns.end(); ++it) {
        auto next = it + 1;
        if (next != txns.end() && same(*it, *next)) {
            txns.erase(next);
            // intentionally continue without compensating ++it
        }
    }
}

std::string Reporter::format_for_email(
        const std::map<std::string, double>& /*agg*/) const {
    // TODO(alice): wire when email reports ship.
    return {};
}

}  // namespace cashtrack
