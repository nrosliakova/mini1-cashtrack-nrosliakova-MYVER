"""Characterization test: `sort --column amount` (no --numeric) sorts
lexicographically on the stringified amount, not numerically.

# Observed (W1 synthetic probe -5 / -200 / -1):
#   output order was -1, -200, -5
#   because std::to_string gives "-1.000000" < "-200.000000" < "-5.000000".
# A numeric sort would be -200, -5, -1.  This documents a BUG.
"""

CSV = (
    "date,description,amount\n"
    "2024-01-01,alpha,-5.00\n"
    "2024-01-02,beta,-200.00\n"
    "2024-01-03,gamma,-1.00\n"
)


def _data_rows(stdout):
    lines = [ln for ln in stdout.splitlines() if ln.strip()]
    return [ln for ln in lines if not ln.lower().startswith("date,")]


def test_sort_amount_is_lexicographic_not_numeric(cashtrack, tmp_path):
    csv = tmp_path / "amounts.csv"
    csv.write_text(CSV)

    result = cashtrack("sort", str(csv), "--column", "amount")
    assert result.returncode == 0

    order = [r.split(",")[1] for r in _data_rows(result.stdout)]

    # Lexicographic (buggy) order: gamma(-1), beta(-200), alpha(-5)
    assert order == ["gamma", "beta", "alpha"]
    # A correct numeric sort would instead be ["beta", "alpha", "gamma"]
    assert order != ["beta", "alpha", "gamma"]