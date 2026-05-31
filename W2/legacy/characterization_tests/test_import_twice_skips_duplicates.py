"""Characterization test: import is idempotent via key-based dedup.

# Observed (W2 report.md, three_tx.csv, 3 rows):
#   1st import -> "Imported 3 transactions" / "0 duplicates skipped"
#   2nd import -> "Imported 0 transactions" / "3 duplicates skipped"
#                 / "Total amount: 0"
# This PINS documented behavior. It is NOT a README<->binary gap.
"""

THREE_TX = (
    "date,description,amount\n"
    "2024-09-01,starbucks #4187,-4.85\n"
    "2024-09-02,uber trip,-9.40\n"
    "2024-09-03,tesco extra,-27.41\n"
)


def test_import_twice_skips_duplicates(cashtrack, tmp_path):
    csv = tmp_path / "three_tx.csv"
    csv.write_text(THREE_TX)

    first = cashtrack("import", str(csv))
    assert first.returncode == 0
    assert "Imported 3 transactions" in first.stdout
    assert "0 duplicates skipped" in first.stdout

    second = cashtrack("import", str(csv))
    assert second.returncode == 0
    assert "Imported 0 transactions" in second.stdout
    assert "3 duplicates skipped" in second.stdout