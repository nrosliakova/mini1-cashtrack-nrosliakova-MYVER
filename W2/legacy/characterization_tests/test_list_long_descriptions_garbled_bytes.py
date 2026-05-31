"""Characterization test: descriptions >= 16 chars are corrupted in the
persisted DB (and therefore in `list`), while short ones survive.

# Root cause (observed + code): Transaction::description() returns a
# std::string_view into a destroyed local std::string (transaction.cpp).
# Storage::save() copies the description THROUGH that dangling view, so
# heap-allocated (>15 char, beyond libstdc++ SSO) descriptions are saved
# as garbage; <=15 char descriptions (small-string optimization) survive.
# This documents a BUG.
"""

CSV = (
    "date,description,amount\n"
    "2024-01-01,short,-1.00\n"                    # 5 chars  -> survives
    "2024-01-02,supermarket purchase x,-2.00\n"  # >15 chars -> corrupted
)

SHORT_DESC = "short"
LONG_DESC = "supermarket purchase x"


def test_long_description_corrupted_in_list(cashtrack, tmp_path):
    csv = tmp_path / "descs.csv"
    csv.write_text(CSV)

    assert cashtrack("import", str(csv)).returncode == 0

    listed = cashtrack("list")
    assert listed.returncode == 0

    # Short description survives the round-trip verbatim.
    assert SHORT_DESC in listed.stdout
    # Long description is corrupted: it does NOT appear verbatim.
    assert LONG_DESC not in listed.stdout