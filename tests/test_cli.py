"""Check documented CLI output and exit statuses (standard library only)."""

import pathlib
import subprocess
import sys
import unittest

if len(sys.argv) != 2:
    raise SystemExit("Usage: test_cli.py <netsync-executable>")
EXECUTABLE = pathlib.Path(sys.argv[1]).resolve()


class CommandLineTests(unittest.TestCase):
    def run_cli(self, argument):
        return subprocess.run(
            [str(EXECUTABLE), argument], capture_output=True,
            text=True, timeout=10, check=False,
        )

    def test_demo_matches_golden_summary(self):
        result = self.run_cli("demo")
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(result.stdout, """ticks=14
issued_inputs=12
server_ack=12
client_ack=12
server_position=14
predicted_position=14
peak_state_gap=1
upstream_sent=14
upstream_dropped=3
upstream_reordered=5
downstream_sent=14
downstream_dropped=4
downstream_reordered=2
converged=true
""")

    def test_ideal_reference_has_no_loss_or_gap(self):
        result = self.run_cli("ideal")
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertIn("peak_state_gap=0\n", result.stdout)
        self.assertIn("upstream_dropped=0\n", result.stdout)
        self.assertIn("downstream_dropped=0\n", result.stdout)

    def test_trace_has_expected_columns_and_rows(self):
        result = self.run_cli("trace")
        self.assertEqual(result.returncode, 0, result.stderr)
        rows = result.stdout.splitlines()
        self.assertEqual(rows[0], "tick,issued,server_ack,client_ack,server_position,predicted_position,pending")
        self.assertEqual(len(rows), 15)
        self.assertEqual(rows[-1], "13,12,12,12,14,14,0")
        self.assertTrue(all(len(row.split(",")) == 7 for row in rows))

    def test_invalid_mode_reports_exit_two(self):
        result = self.run_cli("unexpected")
        self.assertEqual(result.returncode, 2)
        self.assertIn("Unknown command", result.stderr)

    def test_help_is_available(self):
        result = self.run_cli("help")
        self.assertEqual(result.returncode, 0)
        self.assertIn("trace:", result.stdout)


if __name__ == "__main__":
    unittest.main(argv=[sys.argv[0]], verbosity=2)
