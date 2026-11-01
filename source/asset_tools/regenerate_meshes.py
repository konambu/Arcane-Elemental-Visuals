# SPDX-License-Identifier: GPL-3.0-only
"""Regenerate the three visual meshes without overwriting installed resources."""
from pathlib import Path
import subprocess
import sys
from fireball_mesh import fireball_mesh

if __name__ == '__main__':
    root = Path(__file__).resolve().parents[2]
    output = root / 'build/asset-preview'
    output.mkdir(parents=True, exist_ok=True)
    (output / 'AEV_fire_missile.mesh').write_bytes(fireball_mesh())
    for script in ['rock_spike_mesh.py', 'fire_vortex_mesh.py']:
        subprocess.run([sys.executable, str(Path(__file__).with_name(script))], check=True)
    print('Mesh outputs:', output)
