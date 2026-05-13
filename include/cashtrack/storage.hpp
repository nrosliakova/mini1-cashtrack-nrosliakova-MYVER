#pragma once

#include "transaction.hpp"
#include <string>
#include <vector>

namespace cashtrack {

// Persists the transaction list to a JSON file. Default location is
// ~/.cashtrack/db.json on Unix-likes. Falls back to ./cashtrack_db.json
// if HOME is not set.
class Storage {
public:
    Storage();
    explicit Storage(std::string path);

    const std::string& path() const { return path_; }

    std::vector<Transaction> load() const;
    void                     save(const std::vector<Transaction>& txns) const;

    // Returns true if the underlying file exists.
    bool exists() const;

private:
    std::string path_;

    static std::string default_path();
    static void        ensure_parent_dir(const std::string& path);
};

}  // namespace cashtrack
