# SPDX-License-Identifier: GPL-3.0-only
"""Build a unique matte-rock mesh with three irregular upper-trunk branches.
Uses the original addon's geometry as input; does not edit any Workshop asset.
"""
from pathlib import Path
import math,struct,sys,json,random
p=Path(__file__).resolve().parents[2]
from read_mesh import read_mesh
output=p/'build/asset-preview';output.mkdir(parents=True,exist_ok=True)
def chunk(tag,payload):return struct.pack('<HI',tag,len(payload)+6)+payload
def sub(a,b):return tuple(x-y for x,y in zip(a,b))
def add(a,b):return tuple(x+y for x,y in zip(a,b))
def mul(a,t):return tuple(x*t for x in a)
def cross(a,b):return (a[1]*b[2]-a[2]*b[1],a[2]*b[0]-a[0]*b[2],a[0]*b[1]-a[1]*b[0])
def dot(a,b):return sum(x*y for x,y in zip(a,b))
def unit(v):return mul(v,1/max(math.sqrt(dot(v,v)),1e-9))
original=read_mesh(p/'assets/EarthSpike/v001/AM_EarthSpike_v001.mesh')
points=[(x,z,-y) for x,y,z in original['verts']];uv=[(u,1-v) for u,v in original['uv']]
faces=list(original['faces']);branch_info=[]
rng=random.Random(4317)
for root,tip,radius in [((.7,14.6,.05),(5.4,18.2,1.3),.95),((.95,16.0,-.1),(-3.0,19.0,-3.0),.8),((1.1,17.4,-.2),(1.8,19.4,4.0),.62)]:
    direction=unit(sub(tip,root));side=unit(cross(direction,(0,1,0)));up=unit(cross(side,direction))
    start=len(points);sides=6;rings=[(0,1),(.34,.83),(.69,.47),(.9,.21)]
    for r,(fraction,width) in enumerate(rings):
        center=add(root,mul(sub(tip,root),fraction))
        center=add(center,mul(up,math.sin(fraction*math.pi)*radius*.16))
        for i in range(sides):
            angle=math.tau*i/sides+.12*r
            offset=add(mul(side,math.cos(angle)*radius*width*rng.uniform(.85,1.15)),mul(up,math.sin(angle)*radius*width*.8*rng.uniform(.87,1.13)))
            points.append(add(center,offset));uv.append((i/sides*.6+r*.065,fraction*.65))
    for r in range(len(rings)-1):
        for i in range(sides):
            a=start+r*sides+i;b=start+r*sides+(i+1)%sides;c=a+sides;d=b+sides
            faces.extend([(a,b,c),(b,d,c)])
    end=len(points);points.append(tip);uv.append((.3,.8))
    for i in range(sides):faces.append((start+3*sides+i,start+3*sides+(i+1)%sides,end))
    # Ensure branch faces are outward while retaining original base winding.
    for k in range(len(original['faces'])+sum(x['triangles'] for x in branch_info),len(faces)):
        tri=faces[k];a,b,c=[points[j] for j in tri];n=cross(sub(b,a),sub(c,a));center=mul(add(add(a,b),c),1/3)
        along=dot(sub(center,root),direction);radial=sub(sub(center,root),mul(direction,along))
        if dot(n,radial)<0:faces[k]=(tri[0],tri[2],tri[1])
    branch_info.append({'root':root,'tip':tip,'triangles':42})
vertices=[];indices=[]
for tri in faces:
    a,b,c=[points[i] for i in tri];normal=unit(cross(sub(b,a),sub(c,a)))
    t0,t1,t2=[uv[i] for i in tri];du1=t1[0]-t0[0];dv1=t1[1]-t0[1];du2=t2[0]-t0[0];dv2=t2[1]-t0[1]
    det=du1*dv2-du2*dv1
    tangent=unit(mul(sub(mul(sub(b,a),dv2),mul(sub(c,a),dv1)),1/det)) if abs(det)>1e-7 else unit(cross(normal,(0,0,1)))
    tangent=unit(sub(tangent,mul(normal,dot(tangent,normal))))
    for i in tri:
        indices.append(len(vertices));vertices.append(struct.pack('<12f',*points[i],*normal,*uv[i],*tangent,1))
decl=b''.join(chunk(0x5110,struct.pack('<5H',0,t,s,o,0)) for t,s,o in [(2,1,0),(2,4,12),(1,7,24),(3,9,32)])
geo=chunk(0x5000,struct.pack('<I',len(vertices))+chunk(0x5100,decl)+chunk(0x5200,struct.pack('<HH',0,48)+chunk(0x5210,b''.join(vertices))))
sm=b'EV_EarthSpike_Charcoal_v001\n'+b'\x00'+struct.pack('<I',len(indices))+b'\x00'+struct.pack('<'+'H'*len(indices),*indices)+geo+chunk(0x4010,struct.pack('<H',4))
lo=[min(v[i] for v in points) for i in range(3)];hi=[max(v[i] for v in points) for i in range(3)];radius=max(math.sqrt(dot(v,v)) for v in points)
result=b'\x00\x10[MeshSerializer_v1.100]\n'+chunk(0x3000,b'\x00'+chunk(0x4000,sm)+chunk(0x9000,struct.pack('<7f',*lo,*hi,radius)))
out=output/'AEV_EarthRock_v3.mesh';out.write_bytes(result)
info={'vertices':len(vertices),'triangles':len(faces),'branches':branch_info,'bounds':[lo,hi],'unique_mesh':out.name}
(output/'rock-spike-v3-geometry.json').write_text(json.dumps(info,indent=2)+'\n',encoding='utf-8')
assert len(faces)==len(original['faces'])+126
print('Branched rock mesh:',len(faces),'triangles; 3 branches; height remains20 before ground-anchored runtime stretch')
