# cashtrack — Demo invocations

A short guided tour of what cashtrack does. Run these in order against the `samples/` directory.

> **For SE460 students:** These are the invocations you should start with in W1 Practice 1. Try them all, then go off-script with your own inputs. The bugs are not in these examples — that's the point of black-box exploration.

---

## 1. First contact

```
$ ./cashtrack --version
cashtrack 0.3.0 (2025-05-01)

$ ./cashtrack --help
cashtrack — a personal-finance CLI

Usage: cashtrack <command> [options]

Commands:
  import <file>          Import transactions from a CSV file
  list                   List all transactions
  report                 Generate a report
  split <file>           Split a CSV into chunks
  sort <file>            Sort a CSV by a column
  export <file>          Export current database to CSV
  categories             Show categorization rules
  --help                 Show this help
  --version              Show version

Run 'cashtrack <command> --help' for command-specific options.
```

---

## 2. Import a bank statement

```
$ ./cashtrack import samples/chase_export.csv
Imported 24 transactions.
  - 21 categorized
  - 3 uncategorized
  - 0 duplicates skipped

Total amount: 1842.37
Date range: 2024-09-01 → 2024-09-28
```

---

## 3. List recent transactions

```
$ ./cashtrack list --from 2024-09-20
2024-09-21  starbucks #4187            -4.85     coffee
2024-09-21  shell gas station          -52.30    transport
2024-09-22  tesco extra peckham        -27.41    groceries
2024-09-23  amazon mktplace            -19.99    uncategorized
2024-09-24  spotify premium            -10.99    subscriptions
2024-09-25  payroll deposit         +3200.00     income
2024-09-26  tesco extra peckham        -34.18    groceries
2024-09-27  uber trip                  -12.40    transport
2024-09-28  costa coffee                -3.95    coffee

9 transactions.
```

---

## 4. A report by category

```
$ ./cashtrack report --by category --from 2024-09-01 --to 2024-09-30
2024-09 spending by category

  groceries       312.84  ████████████████████████████
  transport       184.20  ████████████████
  coffee           67.55  ██████
  subscriptions    32.97  ███
  uncategorized    19.99  ██
  income        3200.00  (excluded from spending chart)

Total spent: 617.55
```

---

## 5. Split a CSV file

```
$ ./cashtrack split samples/chase_export.csv --rows 10 --out chunks/
Wrote 3 chunks:
  chunks/chase_export_001.csv (10 rows)
  chunks/chase_export_002.csv (10 rows)
  chunks/chase_export_003.csv (4 rows)
```

---

## 6. Sort a CSV by a column

```
$ ./cashtrack sort samples/chase_export.csv --column date > sorted.csv
Sorted 24 rows by date.

$ ./cashtrack sort samples/chase_export.csv --column amount --numeric > by_amount.csv
Sorted 24 rows by amount (numeric).
```

---

## 7. Export the current database

```
$ ./cashtrack export backup.csv
Exported 24 transactions to backup.csv.
```

---

## 8. View categorization rules

```
$ ./cashtrack categories
Categorization rules (in match order):

  ^(starbucks|costa|nero|pret)              → coffee
  ^(tesco|sainsbury|atb|silpo|fora)         → groceries
  ^(uber|bolt|lyft|taxi)                    → transport
  ^(spotify|netflix|youtube)                → subscriptions
  ^(shell|bp|esso|wog|okko)                 → transport
  ^(payroll|salary)                         → income
  ^(rent|mortgage)                          → housing
  ^(amazon|ebay)                            → shopping

8 rules. To customize, edit categories.cpp and rebuild.
```

---

## Try the other sample files yourself

```
samples/
├── chase_export.csv         (the happy path — 24 USD transactions)
├── monobank_export.csv      (Ukrainian merchant names, UAH)
├── tricky_quotes.csv        (merchant names with commas)
├── european_dates.csv       (DD/MM/YYYY format)
├── duplicates.csv           (the same transaction imported from two sources)
└── windows_export.csv       (saved from Excel on Windows — line endings vary)
```

Some of these are harder for cashtrack to handle correctly than others. **Finding out which, and how, is the work.**
