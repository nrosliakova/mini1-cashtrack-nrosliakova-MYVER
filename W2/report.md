# W2 Report

## Status at submission

W2 shipped under tight time budget. Greenfield PRD v0 published at
CS460-SEP-2026/greenfield#47. Legacy w2 branch scaffolded with conftest.py,
one fixture, and one confirming characterization test
(test_import_twice_skips_duplicates.py).

## What worked

- Greenfield PRD §3 NOT-in-scope anchored to 5 candidate cashtrack test
  filenames (1 implemented at submission time).
- HOME-isolation pattern via subprocess env={"HOME": tmp_path} confirmed.
- First import returns "Imported 3 transactions / 0 duplicates skipped";
  re-import returns "Imported 0 / 3 duplicates skipped / Total amount: 0".
  Documented dedup behavior holds on this 3-row fixture.

## What didn't / open

- Did not reach >=6 characterization tests with >=3 documenting
  README<->binary gaps.
- Did not run Hypothesis-Generator session (pre-condition: >=5 manual
  tests first; unmet).
- Peer review not yet performed.

## Honest note

Submitting incomplete W2 to lock in the existing greenfield deliverable
and the scaffolded legacy structure rather than miss the deadline entirely.
