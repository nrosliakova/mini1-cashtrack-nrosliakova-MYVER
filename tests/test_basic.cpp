#include <gtest/gtest.h>

#include "cashtrack/importer.hpp"
#include "cashtrack/storage.hpp"
#include "cashtrack/transaction.hpp"

#include <cstdio>
#include <fstream>

using namespace cashtrack;

TEST(Csv, SplitsSimpleRow) {
    auto cols = Importer::split_csv_row("a,b,c");
    ASSERT_EQ(cols.size(), 3u);
    EXPECT_EQ(cols[0], "a");
    EXPECT_EQ(cols[1], "b");
    EXPECT_EQ(cols[2], "c");
}

TEST(Csv, SplitsEmptyFields) {
    auto cols = Importer::split_csv_row("a,,c");
    ASSERT_EQ(cols.size(), 3u);
    EXPECT_EQ(cols[0], "a");
    EXPECT_EQ(cols[1], "");
    EXPECT_EQ(cols[2], "c");
}

TEST(Csv, ReadsHappyPathFile) {
    const char* path = "test_csv_happy.csv";
    {
        std::ofstream f(path);
        f << "date,description,amount\n"
          << "2024-09-01,coffee,-4.85\n"
          << "2024-09-02,salary,3000\n";
    }
    std::vector<Transaction> existing;
    Importer imp;
    auto stats = imp.import_file(path, existing);
    std::remove(path);

    ASSERT_EQ(existing.size(), 2u);
    EXPECT_EQ(stats.imported, 2);
    EXPECT_EQ(existing[0].date(),        "2024-09-01");
    EXPECT_EQ(std::string(existing[0].description()), "coffee");
    EXPECT_DOUBLE_EQ(existing[0].amount(), -4.85);
    EXPECT_DOUBLE_EQ(existing[1].amount(), 3000.0);
}

TEST(Storage, RoundTripsSimpleRecord) {
    const char* path = "test_db.json";
    std::remove(path);
    Storage s(path);

    std::vector<Transaction> in {
        Transaction("2024-09-01", "coffee", -4.85),
        Transaction("2024-09-02", "groceries", -22.30),
    };
    s.save(in);

    auto out = s.load();
    ASSERT_EQ(out.size(), 2u);
    EXPECT_EQ(std::string(out[0].description()), "coffee");
    EXPECT_DOUBLE_EQ(out[1].amount(), -22.30);
    std::remove(path);
}
