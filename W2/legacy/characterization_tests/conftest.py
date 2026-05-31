import subprocess
from pathlib import Path
import pytest

BINARY = Path(__file__).resolve().parents[3] / "build" / "cashtrack"

@pytest.fixture
def cashtrack(tmp_path):
    """Run cashtrack with HOME isolated to tmp_path. Returns a callable."""
    def _run(*args):
        env = {
            "HOME": str(tmp_path),
            "PATH": "/usr/bin:/bin:/usr/local/bin",
        }
        return subprocess.run(
            [str(BINARY), *args],
            env=env,
            capture_output=True,
            text=True,
        )
    return _run