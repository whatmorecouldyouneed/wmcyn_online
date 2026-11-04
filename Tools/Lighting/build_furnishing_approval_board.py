"""Offline approval boards from measured geometry and neutral FBX renders."""
import json
from pathlib import Path
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
from matplotlib.collections import PolyCollection
from matplotlib.patches import Rectangle

root = Path('Saved/Diagnostics/Lighting')
out = Path('Specs/004-crib-furnishing/Previews')
out.mkdir(parents=True,exist_ok=True)
items = json.loads((root/'furnishing_fbx_bounds.json').read_text())
fig,axes=plt.subplots(2,5,figsize=(20,9),facecolor='white')
for ax,item in zip(axes.flat,items):
    ax.imshow(plt.imread(root/'FurniturePreviews'/(Path(item['source']).stem+'.png')))
    ax.axis('off')
    dims=' x '.join(f'{v:.2f}' for v in item['dimensions_m'])
    ax.set_title(item['source'].removesuffix('.fbx').replace('_',' ')+'\n'+dims+' m | '+f"{item['triangles']:,} tris",fontsize=10)
fig.suptitle('WMCYN furniture candidates | actual source geometry | neutral clay, NOT final materials',fontsize=17)
fig.tight_layout()
fig.savefig(out/'furniture-candidates.png',dpi=130)
plt.close(fig)

rows=json.loads((root/'furnishing_live_geometry.json').read_text())
fig,axes=plt.subplots(1,2,figsize=(14,11))
for ax,z in zip(axes,[6,462]):
    for row in rows:
        if row['component'] not in ['Plane_049','Plane_003','Plane_004']:
            continue
        tris=[t for t in row['horizontal_triangles'] if abs(sum(p[2] for p in t)/3-z)<4]
        ax.add_collection(PolyCollection([[(p[0]/100,p[1]/100) for p in t] for t in tris],facecolor='#edf0f0',edgecolor='#c5cece',linewidth=.4))
    ax.set_xlim(-20,8); ax.set_ylim(-16,23); ax.set_aspect('equal')
    ax.set_xlabel('Unreal X (metres)'); ax.set_ylabel('Unreal Y (metres)')
    ax.set_title(f'Measured floor ~Z {z} cm'); ax.grid(alpha=.15)

def box(ax,x,y,w,h,label,color):
    ax.add_patch(Rectangle((x,y),w,h,facecolor=color,edgecolor='#303940',alpha=.6))
    ax.text(x+w/2,y+h/2,label,ha='center',va='center',fontsize=8)

# Zone proposals are deliberately not an executable actor-placement manifest.
box(axes[0],-12,-9,9,8,'','#8eb8bf')
axes[0].text(-11.8,-1.7,'A / LOUNGE: 9 x 8 m',fontsize=9)
box(axes[0],-10.8,-8.1,4.616,5.091,'Sectional','#467c89')
box(axes[0],-5.7,-7.8,1.45,1.19,'Chair 1','#d0ad73')
box(axes[0],-5.7,-5.4,1.45,1.19,'Chair 2','#d0ad73')
box(axes[0],-8.1,-5.8,.927,1.909,'','#d0ad73')
box(axes[0],-5,6,4,4,'B / COUNTER-SIDE\n4 x 4 m review zone\n4 stools; align after survey','#d0ad73')
box(axes[1],-10,12,6,6,'','#8eb8bf')
axes[1].text(-9.8,17.2,'C / DINING: 6 x 6 m',fontsize=8)
axes[1].text(-9.8,12.5,'Table + four chairs',fontsize=8)
box(axes[1],-7.68,14.32,1.357,1.357,'Table','#d0ad73')
box(axes[1],-18.56,.38,8.88,9.07,'PROTECTED\nInterview / bathroom\nNo furnishing in this batch','#eab0ae')
fig.suptitle('LAYOUT A: approval concept over measured floor surfaces',fontsize=16)
fig.text(.5,.035,'Furniture boxes are proposed bounds, not validated actor transforms. Floor coverage does not prove wall/door clearance.\nBar counter height, full swing envelopes and source orientation remain import-pilot gates.',ha='center',fontsize=10)
fig.tight_layout(rect=(0,.07,1,.96))
fig.savefig(out/'layout-a-measured-floors.png',dpi=150)
