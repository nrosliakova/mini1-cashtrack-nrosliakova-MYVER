# cashtrack

> **A personal-finance CLI for people who actually own their data.**
> Import bank CSV exports. Categorize transactions. Get reports. Never leave the terminal.

[![CI](https://github.com/example/cashtrack/actions/workflows/ci.yml/badge.svg)](https://github.com/example/cashtrack/actions)
[![Release](https://img.shields.io/badge/release-v0.3.0-blue.svg)](https://github.com/example/cashtrack/releases/tag/v0.3.0)
![License](https://img.shields.io/badge/license-MIT-green.svg)

---

## Why cashtrack?

Look, we tried the alternatives:

| Tool | Why it didn't fit us |
|---|---|
| YNAB | $99/year, web-only, your data lives on their servers |
| Mint | Killed by Intuit in March 2024 |
| Monarch | Subscription, web-only |
| hledger / ledger-cli | Steep learning curve, your bank export doesn't fit plain-text accounting cleanly |
| GnuCash | Heavy desktop app, overkill for personal use |
| A spreadsheet | Fine until you have 18 months of data |

So we built **cashtrack** — small, fast, no servers, no subscriptions, no accounting degree. One binary. One JSON file. Done.

> **Design principle:** if you can do it with `awk` and a stubborn afternoon, cashtrack should do it in one command.

---

## What it does

- **Imports** CSV bank statements from any bank that exports CSV (we've tested Chase, Monobank, Privatbank, Revolut)
- **Categorizes** transactions automatically using a regex-based ruleset you can edit
- **Splits** large CSVs into smaller chunks for archival or per-month review
- **Sorts and filters** by date range, category, amount, tags
- **Reports** — by category, by tag, by month — with simple ASCII charts
- **Exports** back to CSV for any tool that consumes it

All from one binary. ~1.2 MB. No runtime dependencies.

---

## Quick start

```bash
# Download the latest release for your platform
# Linux x64
curl -L https://github.com/example/cashtrack/releases/download/v0.3.0/cashtrack-0.3.0-linux-x64 -o cashtrack
chmod +x cashtrack

# macOS arm64
curl -L https://github.com/example/cashtrack/releases/download/v0.3.0/cashtrack-0.3.0-macos-arm64 -o cashtrack
chmod +x cashtrack

# Windows x64 (PowerShell)
# iwr https://github.com/example/cashtrack/releases/download/v0.3.0/cashtrack-0.3.0-win-x64.exe -OutFile cashtrack.exe

# Import a bank export
./cashtrack import samples/chase_export.csv

# See your spending by category for last month
./cashtrack report --from 2024-09-01 --to 2024-09-30 --by category

# Split a huge CSV into 1000-row chunks
./cashtrack split big_export.csv --rows 1000 --out chunks/
```

That's it. No setup wizard. No config file. (There's a `~/.cashtrack/db.json` that gets created automatically — `%USERPROFILE%\.cashtrack\db.json` on Windows. You can edit it directly if you want.)

### Windows users — three setup paths

A native `cashtrack-0.3.0-win-x64.exe` is published on the Releases page. It works, but the rest of our examples below assume a POSIX shell. Pick whichever fits your workflow:

1. **WSL2 + Ubuntu 24.04 (recommended for course work).** Install via PowerShell as Administrator:
   ```
   wsl --install -d Ubuntu-24.04
   ```
   Inside the Ubuntu shell, download the `linux-x64` binary above. Every example in this README and in `DEMO.md` works unchanged.

2. **GitHub Codespaces (zero local setup).** Click `Code → Codespaces → Create codespace on main` on the repo page. The `.devcontainer/` config builds the dev environment for you in ~90 seconds.

3. **Native Windows binary.** Use `cashtrack-0.3.0-win-x64.exe` directly from PowerShell or `cmd.exe`. Note: some sample CSVs use POSIX line endings; if you re-save them through Excel, you may hit the "CRLF in last column" gotcha. (Pure black-box exploration in W1 — discover it yourself.)

---

## Building from source

```bash
git clone https://github.com/example/cashtrack
cd cashtrack
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
./build/cashtrack --help
```

Requirements:
- C++17 compiler — g++-9+, clang-10+, or MSVC 19.30+ (Visual Studio 2022)
- CMake 3.20+
- GoogleTest is fetched automatically

On Windows, use the Visual Studio 2022 "Developer Command Prompt" (or `vcvarsall.bat`) so MSVC and ninja are on PATH; CMake will pick up the MSVC generator automatically.

**Docker fallback** (works on Linux, macOS, and Windows with Docker Desktop):

```bash
docker build -t cashtrack .
docker run --rm -v $PWD/samples:/data cashtrack import /data/chase_export.csv
```

---

## Commands

```
cashtrack <command> [options]

Commands:
  import <file>          Import transactions from a CSV file
  list                   List all transactions (use --from --to --category --tag to filter)
  report                 Generate a report (--by category|tag|month)
  split <file>           Split a CSV into chunks (--rows N --out DIR)
  sort <file>            Sort a CSV by a column (--column COL [--numeric])
  export <file>          Export current database to CSV
  categories             Show categorization rules
  --help                 Show this help
  --version              Show version
```

Run `cashtrack <command> --help` for command-specific options.

---

## How categorization works

cashtrack maps the transaction description to a category using a list of regex rules in `categories.cpp`. By default we ship rules for common merchants:

```
^(starbucks|costa|nero|pret)        → coffee
^(tesco|sainsbury|atb|silpo|fora)   → groceries
^(uber|bolt|lyft|taxi)              → transport
^(spotify|netflix|youtube)          → subscriptions
```

If no rule matches, the transaction is categorized as `uncategorized`. You can edit `categories.cpp` and rebuild, or use `cashtrack categorize <id> <category>` to override per-transaction.

---

## Roadmap

- [x] CSV import (v0.1)
- [x] Auto-categorization with regex rules (v0.1)
- [x] Per-category reports with ASCII charts (v0.2)
- [x] CSV export (v0.2)
- [x] Date-range filters (v0.2)
- [x] Tag support (v0.3)
- [x] Duplicate detection on import (v0.3)
- [ ] Multi-currency support (planned v0.4)
- [ ] Budget alerts (planned v0.4)
- [ ] Web dashboard (`cashtrack serve`) (planned v0.5)
- [ ] Mobile companion app (someday)

---

## Comparison with other tools

| Feature | cashtrack | hledger | YNAB | A spreadsheet |
|---|:-:|:-:|:-:|:-:|
| Free | ✅ | ✅ | ❌ | ✅ |
| Offline | ✅ | ✅ | ❌ | ✅ |
| No subscription | ✅ | ✅ | ❌ | ✅ |
| One binary, no deps | ✅ | ✅ | ❌ | ❌ |
| Reads bank CSVs directly | ✅ | ❌ (needs conversion) | ✅ | ❌ |
| ASCII reports | ✅ | ✅ | ❌ | ❌ |
| Auto-categorization | ✅ | ⚠ manual rules | ✅ | ❌ |
| Built in 14 months by one person | ✅ | ❌ | ❌ | n/a |

---

## Known limitations (v0.3)

- Single currency assumed. We treat all amounts as the same currency. Multi-currency is on the roadmap.
- CSV parser is intentionally minimalist — we split on commas. If your bank uses semicolons or tab-delimited files, convert them with `tr` or `sed` first.
- Date parsing is best-effort. We try ISO-8601 first, then `MM/DD/YYYY`. If your bank uses something else, normalize first.
- No reconciliation against actual bank balance — we just trust the import.
- No undo. If you mis-categorize 500 transactions, you re-edit the rules and re-import.

---

## License

MIT. Use it, fork it, build something better.

## Credits

- alice — original design and implementation (2024)
- bob — duplicate detection, date-parse improvements, v0.3 release (2025)
- Coffee from various Kyiv coffee shops (uncategorized in the test data, fittingly)

---

> *"It's a tool. It does one thing. It does it on your laptop. That's enough."* — README rationale, July 2024
