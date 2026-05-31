# Legacy notes (W2)

## Test inventory

Implemented:
1. test_import_twice_skips_duplicates.py -- confirms documented dedup
   behavior on a 3-row fixture. NOT a README<->binary gap.

Planned, not yet implemented:
2. test_list_long_descriptions_garbled_bytes.py -- would document the
   encoding bug observed in W1 (non-ASCII bytes mangled in list output).
3. test_sort_amount_lexicographic_negatives_misordered.py -- would
   document --column amount sorts lexicographically, not numerically.
4. test_import_european_dates_silently_misparsed.py -- would document
   silent mis-handling of DD/MM/YYYY date inputs.
5. test_list_from_to_flags_silently_accepted.py -- would document silent
   acceptance of undocumented filter flags on list.

## Candidate bugs

1. Encoding bug in list/sort output: non-ASCII description bytes mangled
   (observed in W1 on samples/chase_export.csv -- e.g. "атб" -> garbled).
2. sort --column amount is lexicographic, not numeric: e.g. -9.4 sorts
   before -27.41 instead of after (observed in W1).
3. Missing-file handling crashes with uncaught C++ exception: import on
   a nonexistent path raises std::runtime_error -> SIGABRT (exit ~134)
   instead of returning a clean error exit code (observed during W2
   test scaffolding when fixture path was wrong).

## What AI got wrong

- Story-Generator (greenfield track) invented Apple/Outlook calendar
  import as a P1 requirement; motivation names only Google Calendar.
  Discarded.
- Story-Generator marked 6 stories as P0; brief caps P0 at 3-4. Trimmed.
