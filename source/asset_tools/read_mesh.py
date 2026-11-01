# SPDX-License-Identifier: GPL-3.0-only
"""Read static Ogre 1.100 triangle submeshes for asset validation/Blender previews."""
from pathlib import Path
import struct,json,sys
def read_mesh(path):
    b=Path(path).read_bytes();start=b.index(b'\n')+1
    def chunks(a,z):
        while a<z:
            tag,n=struct.unpack_from('<HI',b,a)
            assert n>=6 and a+n<=z,(hex(tag),a,n,z)
            yield tag,a+6,a+n
            a+=n
        assert a==z
    allverts=[];alluv=[];allfaces=[];materials=[];bounds=None
    for top,a,z in chunks(start,len(b)):
        assert top==0x3000
        for tag,c,d in chunks(a+1,z):
            if tag==0x9000:bounds=struct.unpack_from('<7f',b,c)
            if tag!=0x4000:continue
            end=b.index(b'\n',c);materials.append(b[c:end].decode());p=end+1
            shared,count,index32=struct.unpack_from('<?I?',b,p);p+=6
            assert not shared
            indices=struct.unpack_from('<'+('I' if index32 else 'H')*count,b,p);p+=count*(4 if index32 else 2)
            verts=[];uv=[]
            for child,e,f in chunks(p,d):
                if child!=0x5000:continue
                n=struct.unpack_from('<I',b,e)[0];decl={};buffers={}
                for gt,g,h in chunks(e+4,f):
                    if gt==0x5100:
                        for et,i,j in chunks(g,h):
                            assert et==0x5110
                            source,typ,semantic,offset,index=struct.unpack_from('<5H',b,i)
                            decl[semantic,index]=(source,offset,typ)
                    if gt==0x5200:
                        source,stride=struct.unpack_from('<HH',b,g)
                        for bt,i,j in chunks(g+4,h):
                            assert bt==0x5210
                            assert j-i==n*stride
                            buffers[source]=(i,stride)
                for k in range(n):
                    source,off,typ=decl[1,0];data,stride=buffers[source]
                    x,y,z=struct.unpack_from('<3f',b,data+k*stride+off);verts.append((x,-z,y))
                    source,off,typ=decl[7,0];data,stride=buffers[source]
                    u,v=struct.unpack_from('<2f',b,data+k*stride+off);uv.append((u,1-v))
            assert len(verts)>max(indices)
            offset=len(allverts);allverts+=verts;alluv+=uv
            allfaces += [tuple(offset+k for k in indices[i:i+3]) for i in range(0,count,3)]
    return {'verts':allverts,'faces':allfaces,'uv':alluv,'materials':materials,'bounds_ogre':bounds}
if __name__=='__main__':
    result=read_mesh(sys.argv[1]);Path(sys.argv[2]).write_text(json.dumps(result),encoding='utf-8')
    print({k:(len(v) if k in ['verts','faces','uv'] else v) for k,v in result.items()})
