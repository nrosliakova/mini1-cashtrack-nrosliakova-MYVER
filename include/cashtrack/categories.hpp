#pragma once

#include <string>
#include <vector>
#include <utility>

namespace cashtrack {

// (pattern, category). Order matters — first match wins.
using CategoryRule = std::pair<std::string, std::string>;

// Returns the static list of regex rules used to map a transaction
// description to a category. Edit categories.cpp to change them.
//
// NOTE: returns a const reference. Lifetime is program duration.
const std::vector<CategoryRule>& category_rules();

// Returns the category for a given description, or "uncategorized".
std::string categorize_description(const std::string& description);

}  // namespace cashtrack
