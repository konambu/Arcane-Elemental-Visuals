# SPDX-License-Identifier: GPL-3.0-only
"""Stretch only the addon's wind mesh height. Footprint and all combat data stay.
Read the untouched original-format addon copy from the stable release backup.
"""
from pathlib import Path
import struct,hashlib,json
p=Path(__file__).resolve().parents[2]
output=p/'build/asset-preview';output.mkdir(parents=True,exist_ok=True)
original=p/'source/asset_sources/whirlwind-before-fire.mesh'
b=bytearray(original.read_bytes());start=b.index(b'\n')+1
def chunks(a,z):
    while a<z:
        tag,n=struct.unpack_from('<HI',b,a);assert n>=6 and a+n<=z
        yield tag,a+6,a+n;a+=n
for top,a,z in chunks(start,len(b)):
    assert top==0x3000
    for tag,c,d in chunks(a+1,z):
        if tag==0x4000:
            end=b.index(b'\n',c);cursor=end+1
            shared,count,large=struct.unpack_from('<?I?',b,cursor);assert not shared
            cursor+=6+count*(4 if large else 2)
            for child,e,f in chunks(cursor,d):
                if child!=0x5000:continue
                n=struct.unpack_from('<I',b,e)[0]
                for gt,g,h in chunks(e+4,f):
                    if gt!=0x5200:continue
                    source,stride=struct.unpack_from('<HH',b,g);assert source==0 and stride==56
                    for bt,i,j in chunks(g+4,h):
                        assert bt==0x5210 and j-i==n*stride
                        for k in range(n):
                            pos=i+k*stride+4;y=struct.unpack_from('<f',b,pos)[0]
                            struct.pack_into('<f',b,pos,y*1.5)
        if tag==0x9000:
            x0,y0,z0,x1,y1,z1,r=struct.unpack_from('<7f',b,c)
            struct.pack_into('<7f',b,c,x0,y0*1.5,z0,x1,y1*1.5,z1,r*1.5)
target=output/'AM_Whirlwind_v001.mesh';target.write_bytes(b)
print('Wind visual mesh: height42 ->63, unchanged x/z/UV/index/material; SHA256',hashlib.sha256(b).hexdigest())
