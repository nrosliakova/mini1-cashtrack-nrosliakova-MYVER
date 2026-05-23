
# cashtrack — black-box observations

> Brief: `practicals/W1/Practice_1.1_legacy.md` §"What you will produce" #1.
> Target length: ~250 words.
> Submit by Sunday May 17, 2026, 23:59 EEST.

---

## What `cashtrack` does (observable behavior)

Five bullets. What the tool DOES, not how it works. No source-reading.


1. **Imports and persists CSV transactions into a local database.** Running `import` reads a CSV file, reports counts of categorized/uncategorized/duplicate transactions, sums the total amount, and shows the date range. The data is retained after import (confirmed: `list` shows the same 26 rows without re-specifying the file).
2. **Automatically categorizes transactions at import time.** All 26 transactions from the sample were categorized (0 uncategorized). The `categories` command exposes the rules used — meaning categorization is rule-driven, not manual.
3. **`list` outputs a formatted ledger from the internal database, not directly from the CSV.** The output is columnar (date, description, amount, category) and sorted by date. It also renders some descriptions with garbled characters (encoding issue, discussed below), but the category column is fully populated.
4. **`sort` operates directly on a CSV file and prints to stdout — it does not touch the internal database.** The command takes a filename and `--column` flag, and returns CSV to stdout without any import-style summary. The output is the raw sorted CSV, not the formatted ledger that `list` produces.
5. **The tool has a known encoding bug affecting multi-byte characters.** Several transaction descriptions show garbled bytes (e.g., `|:��,��|` and `d��\`) in both `list` and `sort` output. This affects descriptions that likely contain non-ASCII characters (store names with accents or Unicode). The bug is consistent across commands, suggesting it's in the shared input/display layer.


## Three behaviors I suspect but cannot confirm

For each: a falsifiable statement + the one input I would feed to confirm/refute it.

### Suspected behavior #1

**Claim:** `import` is idempotent with respect to duplicates — re-importing the same file a second time will skip all 26 rows (reporting "26 duplicates skipped, 0 imported") rather than double-inserting them.

> Basis: The import summary explicitly tracks "duplicates skipped," which implies the tool has deduplication logic. But zero duplicates appeared in this run because it was a fresh import. We have not seen that branch execute.
> 

**Probing input:**
```bash
./build/cashtrack import samples/chase_export.csv
```

Run the exact same import command a second time immediately after invocation 1. If the output reads `- 26 duplicates skipped` and `- 0 categorized`, deduplication is confirmed. If it reads `Imported 52 transactions`, deduplication does not exist or is not key-based.


### Suspected behavior #2

**Claim:** `report --by month` aggregates net amounts per calendar month across all transactions in the database, not per-file.

> Basis: The help text says `report` supports `--by category|tag|month`. We know the database contains September 2024 data. But we have never run `report`, so we don't know whether it reads from the persisted database or requires a file argument, and we don't know its output format or whether it sums, counts, or does something else.
> 

**Probing input:**
```bash
./build/cashtrack report --by month
```

Run with no file argument after importing. If it prints a monthly breakdown (e.g., `2024-09: net X.XX`), it reads from the persisted database. If it errors asking for a file, it requires a file input. If it produces per-category or per-row output, the aggregation model is different than assumed.


### Suspected behavior #3

**Claim:** `sort` writes its output only to stdout and never modifies the input file in place.

> Basis: `sort` in invocation 3 printed CSV to the terminal. But `split` takes a `--out DIR` flag, implying some commands write to disk. It is possible `sort` also has an `--out` option or overwrites the source file when given a flag we haven't seen. The current evidence only shows one invocation with no output flag.
> 

**Probing input:**
```bash
./build/cashtrack sort samples/chase_export.csv --column amount > /dev/null
cat samples/chase_export.csv
```

Redirect stdout to `/dev/null` to suppress it, then print the original file. If the file contents are unchanged, `sort` is stdout-only. If the file has been rewritten in sorted order, the command modifies the source file in place (at least under some conditions).