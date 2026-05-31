"""Characterization test: DD/MM/YYYY dates are silently reordered as
MM/DD/YYYY by normalize_date (importer.cpp), with no validation.

# Observed (W1, european_dates.csv):
#   "01/10/2024" (1 Oct) is stored as "2024-01-10" (10 Jan) -- silent misparse.
# Sharper probe: a day > 12 proves blind reordering:
#   "25/12/2024" -> "2024-25-12" (impossible month, accepted without error).
# This documents a BUG (silent wrong output, no error).
"""

CSV = (
    "date,description,amount\n"
    "25/12/2024,cafe,-10.00\n"
)


def test_european_date_silently_misparsed(cashtrack, tmp_path):
    csv = tmp_path / "eu.csv"
    csv.write_text(CSV)

    imported = cashtrack("import", str(csv))
    assert imported.returncode == 0
    assert "Imported 1 transactions" in imported.stdout

    listed = cashtrack("list")
    assert listed.returncode == 0
    # Blindly reordered to an impossible month, no error raised.
    assert "2024-25-12" in listed.stdout
    # It did NOT keep the real ISO date for 25 Dec 2024.
    assert "2024-12-25" not in listed.stdout