"""Plot measured live Unreal floor triangles, not inferred room rectangles."""
import json
from pathlib import Path
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
from matplotlib.collections import PolyCollection

root = Path('Saved/Diagnostics/Lighting')
rows = json.loads((root/'furnishing_live_geometry.json').read_text())
fig, axes = plt.subplots(1,3,figsize=(18,8))
for ax,z in zip(axes,[-470,6,462]):
    for row in rows:
        triangles = [t for t in row['horizontal_triangles'] if abs(sum(p[2] for p in t)/3-z)<4]
        if not triangles:
            continue
        ax.add_collection(PolyCollection([[(p[0]/100,p[1]/100) for p in t] for t in triangles],
            facecolor='#e0e5e7',edgecolor='#889397',linewidth=.3))
        if row['component'] in ['Plane_004','Plane_005','Plane_049','Plane_003']:
            a,b=row['bounds']
            ax.text((a[0]+b[0])/200,(a[1]+b[1])/200,row['component'],fontsize=8)
    ax.autoscale_view()
    ax.set_aspect('equal')
    ax.set_title(f'Floor elevation ~{z} cm')
    ax.set_xlabel('Unreal X (m)')
    ax.set_ylabel('Unreal Y (m)')
    ax.grid(alpha=.2)
fig.suptitle('Measured Crib horizontal surfaces | read-only Unreal export | not approved furniture placement')
fig.tight_layout()
fig.savefig(root/'furnishing_floor_measurements.png',dpi=130)
