#include "cashtrack/storage.hpp"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>

namespace fs = std::filesystem;

namespace cashtrack {

// --- tiny json helpers (escape and parse just what we need) -------------

static std::string json_escape(const std::string& s) {
    std::string out;
    out.reserve(s.size() + 2);
    for (char c : s) {
        switch (c) {
            case '"':  out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n";  break;
            case '\t': out += "\\t";  break;
            default:   out.push_back(c);
        }
    }
    return out;
}

static std::string json_unescape(const std::string& s) {
    std::string out;
    out.reserve(s.size());
    for (size_t i = 0; i < s.size(); ++i) {
        if (s[i] == '\\' && i + 1 < s.size()) {
            char next = s[++i];
            switch (next) {
                case '"':  out.push_back('"');  break;
                case '\\': out.push_back('\\'); break;
                case 'n':  out.push_back('\n'); break;
                case 't':  out.push_back('\t'); break;
                default:   out.push_back(next);
            }
        } else {
            out.push_back(s[i]);
        }
    }
    return out;
}

static std::string read_json_string(const std::string& s, size_t& pos) {
    if (s[pos] != '"') throw std::runtime_error("expected string");
    ++pos;
    std::string raw;
    while (pos < s.size() && s[pos] != '"') {
        if (s[pos] == '\\' && pos + 1 < s.size()) {
            raw.push_back(s[pos++]);
        }
        raw.push_back(s[pos++]);
    }
    if (pos >= s.size()) throw std::runtime_error("unterminated string");
    ++pos;  // closing "
    return json_unescape(raw);
}

// --- Storage -----------------------------------------------------------

Storage::Storage() : path_(default_path()) {}
Storage::Storage(std::string path) : path_(std::move(path)) {}

std::string Storage::default_path() {
    const char* home = std::getenv("HOME");
    if (home && *home) {
        return std::string(home) + "/.cashtrack/db.json";
    }
    return "cashtrack_db.json";
}

void Storage::ensure_parent_dir(const std::string& path) {
    fs::path p(path);
    if (p.has_parent_path()) fs::create_directories(p.parent_path());
}

bool Storage::exists() const {
    return fs::exists(path_);
}

void Storage::save(const std::vector<Transaction>& txns) const {
    ensure_parent_dir(path_);

    std::ofstream out(path_);
    if (!out) throw std::runtime_error("cannot write: " + path_);

    out << "[\n";
    for (size_t i = 0; i < txns.size(); ++i) {
        const auto& t = txns[i];
        const std::string desc(t.description());  // string_view -> owned copy
        out << "  {\"date\": \""        << json_escape(t.date())
            << "\", \"description\": \"" << json_escape(desc)
            << "\", \"amount\": "        << t.amount()
            << ", \"category\": \""     << json_escape(t.category())
            << "\", \"tags\": [";
        const auto& tags = t.tags();
        for (size_t j = 0; j < tags.size(); ++j) {
            out << "\"" << json_escape(tags[j]) << "\"";
            if (j + 1 < tags.size()) out << ", ";
        }
        out << "]}";
        if (i + 1 < txns.size()) out << ",";
        out << "\n";
    }
    out << "]\n";
}

std::vector<Transaction> Storage::load() const {
    std::vector<Transaction> out;
    std::ifstream in(path_);
    if (!in) return out;  // missing file = empty db

    std::stringstream ss;
    ss << in.rdbuf();
    std::string s = ss.str();

    size_t pos = 0;
    while (pos < s.size() && s[pos] != '[') ++pos;
    if (pos >= s.size()) return out;
    ++pos;

    while (pos < s.size()) {
        while (pos < s.size() && (s[pos] == ' ' || s[pos] == ',' ||
                                  s[pos] == '\n' || s[pos] == '\t')) ++pos;
        if (pos >= s.size() || s[pos] == ']') break;
        if (s[pos] != '{') break;
        ++pos;

        std::string date, description, category;
        double amount = 0.0;
        std::vector<std::string> tags;

        while (pos < s.size() && s[pos] != '}') {
            while (pos < s.size() && s[pos] != '"' && s[pos] != '}') ++pos;
            if (pos >= s.size() || s[pos] == '}') break;
            std::string key = read_json_string(s, pos);
            while (pos < s.size() && s[pos] != ':') ++pos;
            ++pos;  // skip :
            while (pos < s.size() && s[pos] == ' ') ++pos;

            if (key == "date") {
                date = read_json_string(s, pos);
            } else if (key == "description") {
                description = read_json_string(s, pos);
            } else if (key == "category") {
                category = read_json_string(s, pos);
            } else if (key == "amount") {
                size_t end = pos;
                while (end < s.size() && s[end] != ',' && s[end] != '}') ++end;
                amount = std::stod(s.substr(pos, end - pos));
                pos = end;
            } else if (key == "tags") {
                while (pos < s.size() && s[pos] != '[') ++pos;
                if (pos < s.size()) ++pos;
                while (pos < s.size() && s[pos] != ']') {
                    while (pos < s.size() && s[pos] != '"' && s[pos] != ']') ++pos;
                    if (pos >= s.size() || s[pos] == ']') break;
                    tags.push_back(read_json_string(s, pos));
                    while (pos < s.size() && (s[pos] == ',' || s[pos] == ' ')) ++pos;
                }
                if (pos < s.size()) ++pos;  // skip ]
            }
            while (pos < s.size() && s[pos] == ',') ++pos;
        }
        while (pos < s.size() && s[pos] != '}') ++pos;
        if (pos < s.size()) ++pos;  // skip }

        Transaction t(date, description, amount);
        if (!category.empty()) t.set_category(category);
        for (auto& tg : tags) t.add_tag(tg);
        out.push_back(std::move(t));
    }
    return out;
}

}  // namespace cashtrack
