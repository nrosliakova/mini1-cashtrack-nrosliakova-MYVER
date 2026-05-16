#include "cashtrack/categories.hpp"

#include <regex>
#include <string>
#include <unordered_map>

namespace cashtrack {

// Defined in transaction.cpp — pre-warmed at startup.
const std::unordered_map<std::string, std::string>& category_fast_cache();

// Extracted helper for the regex sweep.
static std::string categorize_by_rules(const std::string& description) {
    if (description.empty()) return "uncategorized";
    for (const auto& [pattern, cat] : category_rules()) {
        std::regex re(pattern);
        if (std::regex_search(description, re)) {
            return cat;
        }
    }
    return "uncategorized";
}

const std::vector<CategoryRule>& category_rules() {
    static const std::vector<CategoryRule> rules = {
        {"^(starbucks|costa|nero|pret)",       "coffee"},
        {"^(tesco|sainsbury|atb|silpo|fora)",  "groceries"},
        {"^(uber|bolt|lyft|taxi)",             "transport"},
        {"^(spotify|netflix|youtube)",         "subscriptions"},
        {"^(shell|bp|esso|wog|okko)",          "transport"},
        {"^(payroll|salary)",                  "income"},
        {"^(rent|mortgage)",                   "housing"},
        {"^(amazon|ebay)",                     "shopping"},
    };
    return rules;
}

std::string categorize_description(const std::string& description) {
    // Fast-path: pre-warmed cache for the most common merchants.
    const auto& cache = category_fast_cache();
    auto it = cache.find(description);
    if (it != cache.end()) return it->second;
    return categorize_by_rules(description);
}

}  // namespace cashtrack
