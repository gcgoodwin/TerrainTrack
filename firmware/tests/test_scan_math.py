import unittest
import sys
from pathlib import Path
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'mac'))
try:
    from terraintrack_scan import point_from_range
except ImportError:
    point_from_range = None

@unittest.skipUnless(point_from_range is not None,'Install Mac dependencies first')
class TestScanMath(unittest.TestCase):
    def test_forward(self):
        x,y,z=point_from_range(100,90,90)
        self.assertAlmostEqual(x,0,places=6)
        self.assertAlmostEqual(y,1,places=6)
        self.assertAlmostEqual(z,0,places=6)
    def test_vertical(self):
        x,y,z=point_from_range(200,90,180)
        self.assertAlmostEqual(x,0,places=6)
        self.assertAlmostEqual(y,0,places=6)
        self.assertAlmostEqual(z,2,places=6)

if __name__=='__main__':
    unittest.main()
