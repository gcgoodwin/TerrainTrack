"""Unit tests for network-independent host helpers."""
import unittest
from pathlib import Path
import sys

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'mac'))
try:
    from terraintrack_host import limit
except ImportError:
    limit = None


@unittest.skipUnless(limit is not None, 'Install Mac dependencies first')
class TestLimit(unittest.TestCase):
    def test_clamp(self):
        self.assertEqual(limit(120, 65, 115), 115)
        self.assertEqual(limit(-2, 65, 115), 65)
        self.assertEqual(limit(90.1, 65, 115), 90)


if __name__ == '__main__':
    unittest.main()
