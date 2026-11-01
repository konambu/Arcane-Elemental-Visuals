# SPDX-License-Identifier: GPL-3.0-only
"""Generate an original UV sphere in Ogre MeshSerializer v1.100 format."""
import struct, math

def chunk(tag, payload):
    return struct.pack('<HI',tag,len(payload)+6)+payload

def fireball_mesh(radius=1.2, segments=24, rings=16):
    vertices=[];positions=[];indices=[]
    for row in range(rings+1):
        phi=math.pi*row/rings
        for col in range(segments+1):
            theta=2*math.pi*col/segments
            normal=(math.sin(phi)*math.cos(theta),math.cos(phi),math.sin(phi)*math.sin(theta))
            pos=tuple(radius*n for n in normal)
            positions.append(pos)
            vertices.append(struct.pack('<12f',*pos,*normal,col/segments,row/rings,-math.sin(theta),0,math.cos(theta),1))
    for row in range(rings):
        for col in range(segments):
            a=row*(segments+1)+col;b=a+segments+1
            for tri in [(a,b,a+1),(a+1,b,b+1)]:
                pts=[positions[i] for i in tri]
                u=[pts[1][i]-pts[0][i] for i in range(3)];v=[pts[2][i]-pts[0][i] for i in range(3)]
                n=(u[1]*v[2]-u[2]*v[1],u[2]*v[0]-u[0]*v[2],u[0]*v[1]-u[1]*v[0])
                dot=sum(n[i]*sum(p[i] for p in pts) for i in range(3))
                if abs(dot)<1e-9:continue
                indices.extend(tri if dot>0 else (tri[0],tri[2],tri[1]))
    assert len(indices)==6*segments*(rings-1)
    declaration=b''.join(chunk(0x5110,struct.pack('<5H',0,t,s,o,0)) for t,s,o in [(2,1,0),(2,4,12),(1,7,24),(3,9,32)])
    geometry=chunk(0x5000,struct.pack('<I',len(vertices))+chunk(0x5100,declaration)+chunk(0x5200,struct.pack('<HH',0,48)+chunk(0x5210,b''.join(vertices))))
    submesh=b'AEV_Missile_Fire\n'+b'\x00'+struct.pack('<I',len(indices))+b'\x00'+struct.pack('<'+'H'*len(indices),*indices)+geometry+chunk(0x4010,struct.pack('<H',4))
    extent=radius*1.1
    bounds=chunk(0x9000,struct.pack('<7f',-extent,-extent,-extent,extent,extent,extent,extent))
    return b'\x00\x10[MeshSerializer_v1.100]\n'+chunk(0x3000,b'\x00'+chunk(0x4000,submesh)+bounds)
