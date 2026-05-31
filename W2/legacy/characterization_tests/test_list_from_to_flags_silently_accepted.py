"""Characterization test: `list` silently accepts undocumented
--from / --to filter flags (cli.cpp cmd_list parses them), even though
`--help` documents `list` only as "List all transactions".

# Observed (W1): list accepts date-range filters not shown in --help.
# This is an UNDOCUMENTED INVARIANT (behavior with no stated intent),
# not necessarily a bug. (--from inclusive, --to exclusive per cmd_list.)
"""

CSV = (
    "date,description,amount\n"
    "2024-01-01,aaa,-1.00\n"
    "2024-06-01,bbb,-2.00\n"
    "2024-12-01,ccc,-3.00\n"
)


def test_list_from_to_flags_silently_accepted(cashtrack, tmp_path):
    csv = tmp_path / "range.csv"
    csv.write_text(CSV)

    assert cashtrack("import", str(csv)).returncode == 0

    listed = cashtrack("list", "--from", "2024-05-01", "--to", "2024-07-01")
    assert listed.returncode == 0

    # Undocumented flags are honored: only the in-range June row shows.
    assert "bbb" in listed.stdout
    assert "aaa" not in listed.stdout
    assert "ccc" not in listed.stdout
    assert "1 transactions." in listed.stdout