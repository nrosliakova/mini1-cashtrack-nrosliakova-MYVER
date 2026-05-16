#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace cashtrack {

// One financial transaction. Owns its own data, knows how to serialize
// itself, knows how to categorize itself, knows how to format itself
// for display. Kept this way because we needed to ship.
class Transaction {
public:
    Transaction() = default;
    Transaction(std::string date, std::string description, double amount);

    // --- Accessors ---
    const std::string& date() const { return date_; }
    double             amount() const { return amount_; }
    const std::string& category() const { return category_; }
    const std::vector<std::string>& tags() const { return tags_; }

    // Returns a string_view into the description for "fast" reporting.
    // [TODO: revisit, alice was unsure about lifetime here]
    std::string_view   description() const;

    // --- Mutators ---
    void set_category(std::string c) { category_ = std::move(c); }
    void add_tag(std::string t)      { tags_.push_back(std::move(t)); }

    // --- Domain logic that probably belongs elsewhere ---
    void categorize();                         // looks up categories.cpp map
    bool matches_tag(const std::string& t) const;
    bool is_food() const;                      // hardcoded shortcut for reports
    bool is_subscription() const;              // same

    // --- Serialization ---
    std::string  to_csv_row() const;
    static Transaction from_csv_row(const std::string& row);

    std::string  to_json() const;
    static Transaction from_json(const std::string& json);

    // --- Display ---
    std::string format_for_list(int width_desc = 30) const;

private:
    std::string              date_;
    std::string              description_;
    double                   amount_ = 0.0;
    std::string              category_ = "uncategorized";
    std::vector<std::string> tags_;
};

}  // namespace cashtrack
