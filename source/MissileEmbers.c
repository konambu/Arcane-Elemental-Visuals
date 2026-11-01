/* Small, bounded world-space ember trails. Native APIs use opaque pointers;
 * only a verified release VC100 SSO string is passed across the Ogre ABI.
 * No character/combat or projectile physics state is modified.
 */
typedef unsigned long DWORD;
typedef unsigned long long U64;
typedef void* MODULE;
typedef int BOOL;
int _fltused=0;
__declspec(dllimport) MODULE __stdcall GetModuleHandleA(const char*);
__declspec(dllimport) void* __stdcall GetProcAddress(MODULE,const char*);
__declspec(dllimport) DWORD __stdcall GetModuleFileNameA(MODULE,char*,DWORD);
__declspec(dllimport) DWORD __stdcall GetCurrentThreadId(void);
__declspec(dllimport) U64 __stdcall GetTickCount64(void);
__declspec(dllimport) void __stdcall AcquireSRWLockExclusive(void*);
__declspec(dllimport) void __stdcall ReleaseSRWLockExclusive(void*);
__declspec(dllimport) void* __stdcall CreateFileA(const char*,DWORD,DWORD,void*,DWORD,DWORD,void*);
__declspec(dllimport) BOOL __stdcall WriteFile(void*,const void*,DWORD,DWORD*,void*);
__declspec(dllimport) BOOL __stdcall CloseHandle(void*);
__declspec(dllimport) unsigned int __stdcall GetPrivateProfileIntA(const char*,const char*,int,const char*);

typedef struct { union { char small[16]; const char* heap; } text; U64 size,capacity; } OldString;
typedef struct { float x,y,z; } Vec3;
typedef struct { void* object;void* control; } MaterialRef;
static MaterialRef flightMaterial;
static void* (*materialManager)(void);
static void* (*findMaterial)(void*,MaterialRef*,const OldString*,const OldString*);
static void* (*cloneMaterial)(void*,MaterialRef*,const OldString*,unsigned char,const OldString*);
static void* (*materialTechnique)(void*,unsigned short);
static void* (*techniquePass)(void*,unsigned short);
static void* (*passTexture)(void*,U64);
static void (*textureNameSet)(void*,const OldString*,int);
static int flightCloneReady;
static void (*originalShoot)(void*,void*,void*,int,const Vec3*);
static int flightLogged,loadedAmmoTrails;
static int flightMaterialEnabled;
typedef struct {
    void *entity,*projectileNode,*scene,*particleNode,*particles;
    U64 born,stopAt;
    int used,stopRequested,rendered;
    int flying,hitPending,impact;
    Vec3 hitPosition;
    void* effectParent;
} Trail;
#define MAX_TRAILS 16
#define MAX_TRAIL_AGE_MS 8000
#define CLEANUP_DELAY_MS 1250
static Trail trails[MAX_TRAILS];
static void* (*originalInstanceCreate)(void*,const OldString*,int);
static void* (*originalWound)(void*,unsigned char,int,void*,int*,void*,int*,void*);
static void (*nodeScale)(void*,const Vec3*);
static int flyingCaptureLogged,impactLogged;
static OldString impactTemplate;
static void* hiddenFlight[128];
static volatile U64 hiddenMask;
static unsigned char (*originalInstanceVisible)(void*,void*);
static void (*originalInstanceInUse)(void*,unsigned char);
static void* (*nodeParent)(void*);
static Vec3* (*localPosition)(void*,Vec3*);
static int hiddenRenderLogged;
static void* stateLock;
static DWORD internalThread;
static int enabled,templateWarning,firstTrailLogged;
static char logPath[1024];
static const char* volatile renderStage="exception: update entry\r\n";
static void* entityVtable;
static OldString templateName={{"AEV_Embers"},10,15};
static int (*addHook)(void*,void*,void**);
static long long (*realAddress)(void*);
static void (*originalAttach)(void*,void*);
static void (*originalDetach)(void*,void*);
static void (*originalDetachAll)(void*);
static void (*originalDestroyNode)(void*,void*);
static void (*originalDestroyEntity)(void*,void*);
static void (*originalClearScene)(void*);
static void (*originalGpu)(void*,float);
static void* (*entityMesh)(void*);
static void (*entitySetMaterial)(void*,const OldString*,const OldString*);
static void* (*getSubEntity)(void*,U64);
static const OldString* (*getMaterialName)(void*);
static int finalMaterialLogged;
static void (*originalSubMaterial)(void*,const void*);
static void (*subMaterialName)(void*,const OldString*,const OldString*);
static void* (*subParent)(void*);
static int materialOverrideFailed;
static unsigned int (*getVisibilityFlags)(void*);
static void (*setVisibilityFlags)(void*,unsigned int);
static void (*setVisible)(void*,unsigned char);
static void (*setRenderQueue)(void*,unsigned char);
static void (*setLocalAabb)(void*,const float*);
static void (*puBoundsAuto)(void*,unsigned char,float);
static const float diagnosticBounds[6]={0,0,0,64,64,64};
static int positionLogged;
static int visibilityLogged;
static void (*originalParticleQueue)(void*,void*,void*,void*);
static int queueLogged,noQueueLogged;
static OldString fireMaterial;
static OldString materialGroup={{"General"},7,15};
static const OldString* (*resourceName)(void*);
static void* (*nodeCreator)(void*);
static Vec3* (*derivedPosition)(void*,Vec3*);
static void* (*particleManager)(void);
static void* (*particleTemplate)(void*,const OldString*);
static void* (*createParticles)(void*,const OldString*);
static void (*destroyParticles)(void*,void*);
static void (*setEmitting)(void*,unsigned char);
static void* (*puCreate)(void*,const OldString*,const OldString*,void*);
static unsigned int particleSerial;
static void (*puUpdate)(void*,float);
static U64 (*puParticleCount)(void*);
static int liveParticlesLogged;
static U64 lastUpdateTick;
static void (*puDestroy)(void*,void*,void*);
static void (*puStart)(void*);
static void (*puStopFade)(void*);
static void (*puKeepLocal)(void*,unsigned char);
static void (*beginParticles)(void*);
static void* (*createNode)(void*,int);
static void* (*rootNode)(void*,int);
static void (*addChild)(void*,void*);
static void (*setPosition)(void*,const Vec3*);
static unsigned int length(const char* s) { unsigned int n=0;while(s[n])++n;return n; }
static void copyText(char* d,const char* s) { while((*d++=*s++)!=0){} }
static void logLine(const char* s) {
    DWORD written; void* f=CreateFileA(logPath,4,3,0,4,0x80,0);
    if(f!=(void*)(U64)-1) { WriteFile(f,s,length(s),&written,0);CloseHandle(f); }
}
static int isInternal(void) { return internalThread==GetCurrentThreadId(); }
static void lock(void) { AcquireSRWLockExclusive(&stateLock); }
static void unlock(void) { ReleaseSRWLockExclusive(&stateLock); }
static U64 hiddenBit(void* entity) {U64 p=(U64)entity;return (U64)1<<(((p>>4)^(p>>17))&63);}
static void rebuildHiddenMask(void) {int i;U64 mask=0;for(i=0;i<128;++i)if(hiddenFlight[i])mask|=hiddenBit(hiddenFlight[i]);hiddenMask=mask;}
static void* createPuParticles(void* scene,const OldString* name) {
    OldString unique={{"AEV_00000000"},12,15};
    unsigned int i,serial=++particleSerial;
    for(i=0;i<8;++i) unique.text.small[4+i]="0123456789abcdef"[(serial>>(28-i*4))&15];
    return puCreate(particleManager(),&unique,name,scene);
}
static void destroyPuParticles(void* scene,void* particles) {
    puDestroy(particleManager(),particles,scene);
}
static void fadePuParticles(void* particles,unsigned char active) {
    if(active) puStart(particles);else puStopFade(particles);
}
static void beginPuParticles(void* particles) {
    puKeepLocal(particles,0);
    puStart(particles);
    if(puBoundsAuto)puBoundsAuto(particles,0,0);
}
static void particleQueueHook(void* particles,void* queue,void* camera,void* lodCamera) {
    int i;
    for(i=0;i<MAX_TRAILS;++i) if(trails[i].used && trails[i].particles==particles) {
        trails[i].rendered=1;
        if(!queueLogged) {
            queueLogged=1;
            logLine("RENDER: own ember system reached ParticleUniverse render queue.\r\n");
        }
        break;
    }
    originalParticleQueue(particles,queue,camera,lodCamera);
}
static void eraseTrail(Trail* t) {
    if(t->particles && !t->rendered && !noQueueLogged) {
        noQueueLogged=1;
        logLine("RENDER: ember system removed without ever reaching ParticleUniverse render queue.\r\n");
    }
    if(t->particles) destroyParticles(t->scene,t->particles);
    if(t->particleNode) originalDestroyNode(t->scene,t->particleNode);
    t->used=0; t->entity=0;t->projectileNode=0;t->particles=0;t->particleNode=0;t->scene=0;
    t->born=0;t->stopAt=0;t->stopRequested=0;t->rendered=0;
    t->flying=0;t->hitPending=0;t->impact=0;
}
static int isMissile(void* object) {
    void** meshRef; const OldString* s; const char* data; const char* wanted="AEV_fire_missile.mesh";
    U64 i,n=length(wanted);
    if(!object || *(void**)object!=entityVtable) return 0;
    meshRef=(void**)entityMesh(object);
    if(!meshRef || !*meshRef) return 0;
    s=resourceName(*meshRef);
    if(!s || s->size<n || s->size>1024 || s->capacity<s->size) return 0;
    data=s->capacity<16?s->text.small:s->text.heap;
    if(!data) return 0;
    for(i=0;i<n;++i) if(data[s->size-n+i]!=wanted[i]) return 0;
    return 1;
}
static int isCustomAmmo(const OldString* s) {
    const char* wanted="AEV_fire_missile.mesh";const char* data;U64 i,n=length(wanted);
    if(!s || s->size<n || s->size>1024 || s->capacity<s->size)return 0;
    data=s->capacity<16?s->text.small:s->text.heap;
    if(!data)return 0;
    for(i=0;i<n;++i)if(data[s->size-n+i]!=wanted[i])return 0;
    return 1;
}
static int isFireFlightMaterial(const OldString* s) {
    const char* prefix="AEV_NativeFlight_Fire";const char* data;unsigned int i,n=length(prefix);
    if(!s || s->size<n || s->size>1024 || s->capacity<s->size)return 0;
    data=s->capacity<16?s->text.small:s->text.heap;if(!data)return 0;
    for(i=0;i<n;++i)if(data[i]!=prefix[i])return 0;return 1;
}
static void* instanceCreateHook(void* manager,const OldString* material,int memoryType) {
    void* entity=originalInstanceCreate(manager,material,memoryType);int i;
    if(!enabled || !entity || !isFireFlightMaterial(material))return entity;
    lock();
    for(i=0;i<128;++i)if(hiddenFlight[i]==entity)hiddenFlight[i]=0;
    rebuildHiddenMask();
    for(i=0;i<MAX_TRAILS;++i)if(trails[i].used && trails[i].entity==entity)break;
    if(i==MAX_TRAILS)for(i=0;i<MAX_TRAILS;++i)if(!trails[i].used)break;
    if(i<MAX_TRAILS) {
        trails[i].used=1;trails[i].entity=entity;trails[i].flying=1;
        trails[i].born=GetTickCount64();trails[i].stopAt=0;trails[i].stopRequested=0;
        if(!flyingCaptureLogged) {flyingCaptureLogged=1;logLine("FLIGHT FX: captured real InstancedEntity projectile, not chamber ammo.\r\n");}
    }
    unlock();return entity;
}
static unsigned char instanceVisibleHook(void* entity,void* camera) {
    int i;
    if(!enabled || !(hiddenMask&hiddenBit(entity)))return originalInstanceVisible(entity,camera);
    lock();
    for(i=0;i<128;++i)if(hiddenFlight[i]==entity) {
        unlock();
        if(!hiddenRenderLogged) {hiddenRenderLogged=1;logLine("IMPACT RENDER: hit fireball excluded from instance rendering.\r\n");}
        return 0;
    }
    unlock();
    return originalInstanceVisible(entity,camera);
}
static void instanceInUseHook(void* entity,unsigned char inUse) {
    int i;
    if(inUse && (hiddenMask&hiddenBit(entity))) {lock();for(i=0;i<128;++i)if(hiddenFlight[i]==entity)hiddenFlight[i]=0;rebuildHiddenMask();unlock();}
    originalInstanceInUse(entity,inUse);
}
static void* woundHook(void* medical,unsigned char low,int area,void* damage,int* material,void* attacker,int* direction,void* harpoon) {
    void* result;void* node=0;void* entity=0;int matched=0,i;Vec3 position;
    __try {
        if(enabled && harpoon) {
            /* Lucius Harpoon layout: two 32-byte hands, four floats, entity/node. */
            entity=*(void**)((char*)harpoon+0x50);node=*(void**)((char*)harpoon+0x58);
            lock();
            for(i=0;i<MAX_TRAILS;++i)if(trails[i].used && trails[i].flying && trails[i].entity==entity) {matched=1;break;}
            unlock();
            if(matched && node) {
                if(localPosition)localPosition(node,&position);else derivedPosition(node,&position);
                void* scene=nodeCreator(node);
                void* parent=nodeParent?nodeParent(node):0;
                lock();
                if(trails[i].used && trails[i].entity==entity) {
                    trails[i].hitPosition=position;trails[i].hitPending=1;
                    trails[i].scene=scene;
                    trails[i].effectParent=parent;
                }
                unlock();
            }
        }
    } __except(1) {matched=0;logLine("IMPACT FX: hit metadata unavailable; native damage preserved.\r\n");}
    result=originalWound(medical,low,area,damage,material,attacker,direction,harpoon);
    if(matched && entity) {
        lock();
        for(i=0;i<128;++i)if(!hiddenFlight[i] || hiddenFlight[i]==entity) {hiddenFlight[i]=entity;break;}
        rebuildHiddenMask();
        unlock();
        if(!impactLogged) {impactLogged=1;logLine("IMPACT FX: character hit; instance visibility suppression and torch flame queued.\r\n");}
    }
    return result;
}
static void shootMaterialHook(void* gun,void* actor,void* target,int stat,const Vec3* aim) {
    MaterialRef saved,*slot=0;
    /* GunClass layout is guarded by the supported 1.0.65 image check.
     * liveAmmoMesh at B8 identifies only this add-on; harpoonMaterial at 70
     * is passed to HarpoonManager separately from the chamber entities. */
    if(enabled && gun && isCustomAmmo((const OldString*)((char*)gun+0xb8))) {
        slot=(MaterialRef*)((char*)gun+0x70);
        if(!flightMaterialEnabled) {
            if(!flightLogged) {
                flightLogged=1;
                logLine("FLIGHT CONTROL: keeping original flight material; sphere mesh unchanged.\r\n");
                __try {
                    if(slot->object && resourceName) {
                        const OldString* s=resourceName(slot->object);
                        if(s && s->size<255 && s->capacity>=s->size) {
                            const char* data=s->capacity<16?s->text.small:s->text.heap;
                            char line[280];unsigned int j;
                            copyText(line,"Native flight material: ");
                            for(j=0;j<s->size;++j)line[24+j]=data[j];
                            line[24+j]='\r';line[25+j]='\n';line[26+j]=0;logLine(line);
                        }
                    }
                } __except(1) {logLine("Native material name unavailable; original flight call preserved.\r\n");}
            }
            originalShoot(gun,actor,target,stat,aim);return;
        }
        if(!slot->object)slot=0; /* Preserve native no-projectile suppression. */
        if(slot && !flightMaterial.object && cloneMaterial) {
            OldString cloneName,texture;
            void* technique;void* pass;void* unit;
            cloneName.text.heap="AEV_NativeFlight_Fire";cloneName.size=length(cloneName.text.heap);cloneName.capacity=cloneName.size;
            copyText(texture.text.small,"AEV_Fire.dds");texture.size=12;texture.capacity=15;
            __try {
            cloneMaterial(slot->object,&flightMaterial,&cloneName,1,&materialGroup);
            if(flightMaterial.object) {
                technique=materialTechnique(flightMaterial.object,0);
                pass=technique?techniquePass(technique,0):0;
                unit=pass?passTexture(pass,0):0;
                if(unit) {textureNameSet(unit,&texture,2);flightCloneReady=1;
                    logLine("FLIGHT CLONE: original projectile shader retained; diffuse image replaced with red-orange fire.\r\n");}
            }
            } __except(1) {logLine("FLIGHT CLONE failed; original flight material retained.\r\n");}
        }
        if(slot && cloneMaterial && !flightCloneReady)slot=0;
        if(slot && !flightMaterial.object)slot=0;
    }
    if(!slot) {originalShoot(gun,actor,target,stat,aim);return;}
    saved=*slot;
    /* Borrow the cached owning reference only during this synchronous call;
     * do not decrement, destroy or mutate the original shared material. */
    *slot=flightMaterial;
    __try {originalShoot(gun,actor,target,stat,aim);}
    __finally {*slot=saved;}
    if(!flightLogged) {
        flightLogged=1;logLine("FLIGHT: supplied fire material during native shoot; original gun material restored. Chamber trails disabled.\r\n");
    }
}
static void subMaterialHook(void* sub,const void* material) {
    originalSubMaterial(sub,material);
    if(!enabled || materialOverrideFailed) return;
    __try {
        void* parent=subParent(sub);
        const OldString* name;const char* data;U64 i;
        if(!isMissile(parent))return;
        name=getMaterialName(sub);
        if(!name || name->size>255 || name->capacity<name->size)return;
        data=name->capacity<16?name->text.small:name->text.heap;
        if(name->size==fireMaterial.size) {
            for(i=0;i<name->size && data[i]==fireMaterial.text.heap[i];++i){}
            if(i==name->size)return; /* Also terminates nested setMaterialName calls. */
        }
        if(!finalMaterialLogged) {
            char line[300];unsigned int j;
            copyText(line,"FINAL missile material replaced: ");
            for(j=0;j<name->size;++j)line[33+j]=data[j];
            line[33+j]='\r';line[34+j]='\n';line[35+j]=0;
            logLine(line);finalMaterialLogged=1;
        }
        subMaterialName(sub,&fireMaterial,&materialGroup);
        if(setRenderQueue)setRenderQueue(parent,50);
    } __except(1) {
        materialOverrideFailed=1;
        logLine("DISABLED: subentity material override failed; particle diagnostics remain enabled.\r\n");
    }
}
static void requestStop(void* node,void* entity) {
    int i;
    for(i=0;i<MAX_TRAILS;++i) if(trails[i].used &&
        ((node && trails[i].projectileNode==node) || (entity && trails[i].entity==entity))) {
        trails[i].projectileNode=0;trails[i].entity=0;trails[i].stopRequested=1;
    }
}
static void attachHook(void* node,void* object) {
    int i; U64 now;
    originalAttach(node,object);
    if(enabled && !isInternal()) {
        int tracked=0;lock();
        for(i=0;i<MAX_TRAILS;++i)if(trails[i].used && trails[i].flying && trails[i].entity==object) {
            trails[i].projectileNode=node;tracked=1;break;
        }
        unlock();
        if(tracked) {if(nodeScale) {const Vec3 one={1,1,1};nodeScale(node,&one);}return;}
    }
    if(!enabled || isInternal() || !isMissile(object)) return;
    if(entitySetMaterial) {
        __try {
            entitySetMaterial(object,&fireMaterial,&materialGroup);
            if(setRenderQueue)setRenderQueue(object,50);
        } __except(1) {
            enabled=0;logLine("DISABLED: assigning missile fire material failed.\r\n");return;
        }
    }
    if(!loadedAmmoTrails)return;
    now=GetTickCount64();lock();
    requestStop(0,object);
    for(i=0;i<MAX_TRAILS;++i) if(!trails[i].used) {
        trails[i].used=1;trails[i].entity=object;trails[i].projectileNode=node;
        trails[i].born=now;trails[i].stopRequested=0;trails[i].stopAt=0;
        break;
    }
    unlock();
}
static void detachHook(void* node,void* object) {
    if(enabled && !isInternal()) { lock();requestStop(node,object);unlock(); }
    originalDetach(node,object);
}
static void detachAllHook(void* node) {
    if(enabled && !isInternal()) { lock();requestStop(node,0);unlock(); }
    originalDetachAll(node);
}
static void destroyNodeHook(void* scene,void* node) {
    if(enabled && !isInternal()) { lock();requestStop(node,0);unlock(); }
    originalDestroyNode(scene,node);
}
static void destroyEntityHook(void* scene,void* entity) {
    if(enabled && !isInternal()) { lock();requestStop(0,entity);unlock(); }
    originalDestroyEntity(scene,entity);
}
static void clearSceneHook(void* scene) {
    int i;
    if(enabled && !isInternal()) {
        lock();internalThread=GetCurrentThreadId();
        for(i=0;i<MAX_TRAILS;++i) if(trails[i].used && trails[i].scene==scene) eraseTrail(&trails[i]);
        /* Pending nodes have no creator cached yet; don't retain them across reset. */
        for(i=0;i<MAX_TRAILS;++i) if(trails[i].used && !trails[i].scene) trails[i].used=0;
        for(i=0;i<128;++i)hiddenFlight[i]=0;
        hiddenMask=0;
        internalThread=0;unlock();
    }
    originalClearScene(scene);
}
static void updateTrails(void) {
    int i; U64 now=GetTickCount64();
    float elapsed=lastUpdateTick?(float)(now-lastUpdateTick)*0.001f:0.016f;
    lastUpdateTick=now;if(elapsed>0.1f)elapsed=0.1f;
    lock(); internalThread=GetCurrentThreadId();
    for(i=0;i<MAX_TRAILS;++i) if(trails[i].used) {
        Trail* t=&trails[i]; Vec3 position;
        if(t->hitPending) {
            Vec3 hit=t->hitPosition;void* scene=t->scene;void* parent=t->effectParent;
            eraseTrail(t);t->used=1;t->impact=1;t->hitPosition=hit;t->scene=scene;t->effectParent=parent;t->born=now;
        }
        if(now-t->born>MAX_TRAIL_AGE_MS)t->stopRequested=1;
        if(t->impact && now-t->born>=4600)t->stopRequested=1;
        if(t->stopRequested && !t->stopAt) {
            if(t->particles) setEmitting(t->particles,0);
            t->stopAt=now;
        }
        if(t->stopAt) {
            if(t->particles && puUpdate) {
                renderStage="exception: updating fading particle simulation\r\n";
                puUpdate(t->particles,elapsed);
            }
            if(!t->particles || now-t->stopAt>=CLEANUP_DELAY_MS) eraseTrail(t);
            continue;
        }
        if(!t->impact && !t->projectileNode) {
            if(!t->flying || now-t->born>200)eraseTrail(t);
            continue;
        }
        renderStage="exception: reading projectile position\r\n";
        if(t->impact)position=t->hitPosition;
        else {
            if(localPosition)localPosition(t->projectileNode,&position);else derivedPosition(t->projectileNode,&position);
            t->effectParent=nodeParent?nodeParent(t->projectileNode):0;
        }
        if(!positionLogged) {
            union {float f;unsigned int u;} component;
            char line[]="POSITION: projectile xyz float32hex=00000000/00000000/00000000\r\n";
            unsigned int axis,j;
            for(axis=0;axis<3;++axis) {
                component.f=axis==0?position.x:(axis==1?position.y:position.z);
                for(j=0;j<8;++j)line[36+axis*9+j]="0123456789abcdef"[(component.u>>(28-j*4))&15];
            }
            logLine(line);positionLogged=1;
        }
        if(!t->particles) {
            void* manager;
            renderStage="exception: particle manager/template lookup\r\n";
            manager=particleManager();
            const OldString* chosen=t->impact?&impactTemplate:&templateName;
            if(!manager || !particleTemplate(manager,chosen)) {
                if(!templateWarning) { logLine("INACTIVE: AEV_Embers particle template missing; no particles created.\r\n");templateWarning=1; }
                eraseTrail(t);continue;
            }
            renderStage="exception: reading projectile scene manager\r\n";
            if(!t->impact)t->scene=nodeCreator(t->projectileNode);
            if(!t->scene) { eraseTrail(t);continue; }
            renderStage="exception: creating ParticleUniverse system\r\n";
            t->particles=createParticles(t->scene,chosen);
            renderStage="exception: creating particle scene node\r\n";
            t->particleNode=createNode(t->scene,0);
            if(!t->particles || !t->particleNode) { eraseTrail(t);continue; }
            if(setRenderQueue)setRenderQueue(t->particles,84);
            if(t->entity && getVisibilityFlags && setVisibilityFlags) {
                unsigned int source=getVisibilityFlags(t->entity),before=getVisibilityFlags(t->particles),j;
                char message[]="VISIBILITY: projectile=00000000 particle-default=00000000; copied projectile flags.\r\n";
                setVisibilityFlags(t->particles,source);
                if(setVisible)setVisible(t->particles,1);
                if(!visibilityLogged) {
                    for(j=0;j<8;++j) {
                        message[23+j]="0123456789abcdef"[(source>>(28-j*4))&15];
                        message[49+j]="0123456789abcdef"[(before>>(28-j*4))&15];
                    }
                    logLine(message);visibilityLogged=1;
                }
            }
            renderStage="exception: parenting particle scene node\r\n";
            addChild(t->effectParent?t->effectParent:rootNode(t->scene,0),t->particleNode);
            renderStage="exception: positioning particle scene node\r\n";
            setPosition(t->particleNode,&position);
            renderStage="exception: attaching ParticleUniverse system\r\n";
            originalAttach(t->particleNode,t->particles);
            renderStage="exception: starting ParticleUniverse emission\r\n";
            beginParticles(t->particles);
            if(!firstTrailLogged) {
                firstTrailLogged=1;
                logLine("ACTIVE: first missile ember trail created. Projectile identified; particle template and render objects available.\r\n");
            }
        } else setPosition(t->particleNode,&position);
        if(t->particles && puUpdate) {
            renderStage="exception: updating particle simulation\r\n";
            puUpdate(t->particles,elapsed);
            if(setLocalAabb)setLocalAabb(t->particles,diagnosticBounds);
            if(puParticleCount && !liveParticlesLogged && puParticleCount(t->particles)>0) {
                liveParticlesLogged=1;logLine("ACTIVE: ember simulation emitted live particles.\r\n");
            }
        }
    }
    internalThread=0;unlock();
}
static void gpuHook(void* world,float delta) {
    originalGpu(world,delta);
    if(!enabled) return;
    __try { updateTrails(); }
    __except(1) {
        /* Stop retries rather than repeatedly touching failed rendering state. */
        enabled=0;internalThread=0;
        ReleaseSRWLockExclusive(&stateLock);
        logLine("DISABLED: rendering exception during ember update. Restart and disable add-on.\r\n");
        logLine(renderStage);
    }
}
static int validModule(MODULE module,unsigned int stamp,unsigned int size) {
    unsigned char* b=(unsigned char*)module; unsigned int off;
    if(!b || *(unsigned short*)b!=0x5a4d) return 0;
    off=*(unsigned int*)(b+0x3c);
    return *(unsigned int*)(b+off)==0x4550 && *(unsigned int*)(b+off+8)==stamp &&
           *(unsigned int*)(b+off+24+56)==size;
}
static void buildMismatch(MODULE game,MODULE ogre) {
    char message[]="INACTIVE: build mismatch. Runtime game timestamp/size=00000000/00000000 Ogre=00000000/00000000\r\n";
    MODULE modules[2];unsigned int k,j,off,values[4];char* slots[4];
    modules[0]=game;modules[1]=ogre;
    for(k=0;k<2;++k) {
        unsigned char* b=(unsigned char*)modules[k];
        off=*(unsigned int*)(b+0x3c);
        values[k*2]=*(unsigned int*)(b+off+8);
        values[k*2+1]=*(unsigned int*)(b+off+24+56);
    }
    slots[0]=message+54;slots[1]=message+63;slots[2]=message+77;slots[3]=message+86;
    for(k=0;k<4;++k) for(j=0;j<8;++j)
        slots[k][j]="0123456789abcdef"[(values[k]>>(28-j*4))&15];
    logLine(message);
}
void EmberStart(MODULE self) {
    MODULE ogre,lib,game,pu; DWORD i,last=0;int ok=1;void* gpuStub;void* gpu;
    char configPath[1024];
    fireMaterial.text.heap="AEV_Missile_Fire";
    fireMaterial.size=length(fireMaterial.text.heap);fireMaterial.capacity=fireMaterial.size;
    impactTemplate.text.heap="AEV_Holy_FireBurst";impactTemplate.size=length(impactTemplate.text.heap);impactTemplate.capacity=impactTemplate.size;
    if(!GetModuleFileNameA(self,logPath,sizeof(logPath))) return;
    for(i=0;logPath[i];++i) if(logPath[i]=='\\' || logPath[i]=='/') last=i;
    copyText(logPath+last+1,"MissileEmbers.log");
    copyText(configPath,logPath);copyText(configPath+last+1,"MissileEmbers.ini");
    flightMaterialEnabled=GetPrivateProfileIntA("Embers","FlightMaterial",0,configPath)!=0;
    if(!GetPrivateProfileIntA("Embers","Enabled",1,configPath)) {
        logLine("INACTIVE: disabled in MissileEmbers.ini.\r\n");return;
    }
    ogre=GetModuleHandleA("OgreMain_x64.dll");lib=GetModuleHandleA("KenshiLib.dll");game=GetModuleHandleA("kenshi_x64.exe");
    pu=GetModuleHandleA("Plugin_ParticleUniverse_x64.dll");
    /* Filled from the tested local binaries; refuse unreviewed engine builds. */
    if(!ogre || !lib || !game || !pu) { logLine("INACTIVE: game/Ogre/KenshiLib/ParticleUniverse not loaded.\r\n");return; }
    /* Steam's running image has a different PE header from its disk wrapper.
     * Game function addresses are resolved by KenshiLib, never fixed RVAs. */
    if((!validModule(game,0x6602d59d,0x232d000) &&
        !validModule(game,0x65d604d7,0x232c000)) || !validModule(ogre,0x5ca5f929,0x9c9000)) {
        buildMismatch(game,ogre);return;
    }
    if(!validModule(pu,0x5ca5faa4,0x718000)) { logLine("INACTIVE: unsupported ParticleUniverse build.\r\n");return; }
#define API(field,module,symbol) *(void**)(&field)=GetProcAddress(module,symbol);if(!field)ok=0
    API(addHook,lib,"?AddHook@KenshiLib@@YA?AW4HookStatus@1@PEAX0PEAPEAX@Z");
    API(realAddress,lib,"?GetRealAddress@KenshiLib@@YA_JPEAX@Z");
    entityVtable=GetProcAddress(ogre,"??_7Entity@Ogre@@6BMovableObject@1@@");if(!entityVtable)ok=0;
    API(entityMesh,ogre,"?getMesh@Entity@Ogre@@QEBAAEBV?$SharedPtr@VMesh@Ogre@@@2@XZ");
    API(entitySetMaterial,ogre,"?setMaterialName@Entity@Ogre@@QEAAXAEBV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@0@Z");
    API(getSubEntity,ogre,"?getSubEntity@Entity@Ogre@@QEAAPEAVSubEntity@2@_K@Z");
    API(getMaterialName,ogre,"?getMaterialName@SubEntity@Ogre@@QEBAAEBV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@XZ");
    API(subParent,ogre,"?getParent@SubEntity@Ogre@@QEBAPEAVEntity@2@XZ");
    API(subMaterialName,ogre,"?setMaterialName@SubEntity@Ogre@@QEAAXAEBV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@0@Z");
    API(materialManager,ogre,"?getSingletonPtr@MaterialManager@Ogre@@SAPEAV12@XZ");
    API(findMaterial,ogre,"?getByName@MaterialManager@Ogre@@QEAA?AV?$SharedPtr@VMaterial@Ogre@@@2@AEBV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@0@Z");
    API(cloneMaterial,ogre,"?clone@Material@Ogre@@QEBA?AV?$SharedPtr@VMaterial@Ogre@@@2@AEBV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@_N0@Z");
    API(materialTechnique,ogre,"?getTechnique@Material@Ogre@@QEAAPEAVTechnique@2@G@Z");
    API(techniquePass,ogre,"?getPass@Technique@Ogre@@QEAAPEAVPass@2@G@Z");
    API(passTexture,ogre,"?getTextureUnitState@Pass@Ogre@@QEAAPEAVTextureUnitState@2@_K@Z");
    API(textureNameSet,ogre,"?setTextureName@TextureUnitState@Ogre@@QEAAXAEBV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@W4TextureType@2@@Z");
    API(getVisibilityFlags,ogre,"?getVisibilityFlags@MovableObject@Ogre@@QEBAIXZ");
    API(setVisibilityFlags,ogre,"?setVisibilityFlags@MovableObject@Ogre@@QEAAXI@Z");
    API(setVisible,ogre,"?setVisible@MovableObject@Ogre@@QEAAX_N@Z");
    API(setRenderQueue,ogre,"?setRenderQueueGroup@MovableObject@Ogre@@UEAAXE@Z");
    API(setLocalAabb,ogre,"?setLocalAabb@MovableObject@Ogre@@QEAAXUAabb@2@@Z");
    API(resourceName,ogre,"?getName@Resource@Ogre@@UEBAAEBV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@XZ");
    API(nodeCreator,ogre,"?getCreator@SceneNode@Ogre@@QEBAPEAVSceneManager@2@XZ");
    API(derivedPosition,ogre,"?_getDerivedPosition@Node@Ogre@@QEBA?AVVector3@2@XZ");
    API(particleManager,pu,"?getSingletonPtr@ParticleSystemManager@ParticleUniverse@@SAPEAV12@XZ");
    API(particleTemplate,pu,"?getParticleSystemTemplate@ParticleSystemManager@ParticleUniverse@@QEAAPEAVParticleSystem@2@AEBV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@@Z");
    API(puCreate,pu,"?createParticleSystem@ParticleSystemManager@ParticleUniverse@@QEAAPEAVParticleSystem@2@AEBV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@0PEAVSceneManager@Ogre@@@Z");
    API(puDestroy,pu,"?destroyParticleSystem@ParticleSystemManager@ParticleUniverse@@QEAAXPEAVParticleSystem@2@PEAVSceneManager@Ogre@@@Z");
    API(puStart,pu,"?start@ParticleSystem@ParticleUniverse@@QEAAXXZ");
    API(puUpdate,pu,"?_update@ParticleSystem@ParticleUniverse@@QEAAXM@Z");
    API(puBoundsAuto,pu,"?setBoundsAutoUpdated@ParticleSystem@ParticleUniverse@@QEAAX_NM@Z");
    API(puParticleCount,pu,"?getNumberOfEmittedParticles@ParticleSystem@ParticleUniverse@@QEAA_KXZ");
    API(puStopFade,pu,"?stopFade@ParticleSystem@ParticleUniverse@@QEAAXXZ");
    API(puKeepLocal,pu,"?setKeepLocal@ParticleSystem@ParticleUniverse@@QEAAX_N@Z");
    createParticles=createPuParticles;destroyParticles=destroyPuParticles;
    setEmitting=fadePuParticles;beginParticles=beginPuParticles;
    API(createNode,ogre,"?createSceneNode@SceneManager@Ogre@@UEAAPEAVSceneNode@2@W4SceneMemoryMgrTypes@2@@Z");
    API(rootNode,ogre,"?getRootSceneNode@SceneManager@Ogre@@QEAAPEAVSceneNode@2@W4SceneMemoryMgrTypes@2@@Z");
    API(addChild,ogre,"?addChild@Node@Ogre@@QEAAXPEAV12@@Z");
    API(setPosition,ogre,"?setPosition@Node@Ogre@@QEAAXAEBVVector3@2@@Z");
    API(nodeScale,ogre,"?setScale@Node@Ogre@@QEAAXAEBVVector3@2@@Z");
    API(nodeParent,ogre,"?getParent@Node@Ogre@@QEBAPEAV12@XZ");
    API(localPosition,ogre,"?getPosition@Node@Ogre@@QEBA?AVVector3@2@XZ");
    gpuStub=GetProcAddress(lib,"?_NV_mainLoop_GPUSensitiveStuff@GameWorld@@QEAAXM@Z");if(!gpuStub)ok=0;
    if(!ok) { logLine("INACTIVE: required API missing.\r\n");return; }
    gpu=(void*)(U64)realAddress(gpuStub);
    if(!gpu) { logLine("INACTIVE: main GPU loop address missing.\r\n");return; }
#define HOOK(symbol,detour,original) {void* target=GetProcAddress(ogre,symbol);if(!target || addHook(target,detour,(void**)&original)!=0 || !original)ok=0;}
    HOOK("?attachObject@SceneNode@Ogre@@QEAAXPEAVMovableObject@2@@Z",attachHook,originalAttach);
    HOOK("?detachObject@SceneNode@Ogre@@QEAAXPEAVMovableObject@2@@Z",detachHook,originalDetach);
    HOOK("?detachAllObjects@SceneNode@Ogre@@UEAAXXZ",detachAllHook,originalDetachAll);
    HOOK("?destroySceneNode@SceneManager@Ogre@@UEAAXPEAVSceneNode@2@@Z",destroyNodeHook,originalDestroyNode);
    HOOK("?destroyEntity@SceneManager@Ogre@@UEAAXPEAVEntity@2@@Z",destroyEntityHook,originalDestroyEntity);
    HOOK("?clearScene@SceneManager@Ogre@@UEAAXXZ",clearSceneHook,originalClearScene);
    HOOK("?setMaterial@SubEntity@Ogre@@QEAAXAEBV?$SharedPtr@VMaterial@Ogre@@@2@@Z",subMaterialHook,originalSubMaterial);
    HOOK("?createInstancedEntity@InstanceManager@Ogre@@QEAAPEAVInstancedEntity@2@AEBV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@W4SceneMemoryMgrTypes@2@@Z",instanceCreateHook,originalInstanceCreate);
    HOOK("?findVisible@InstancedEntity@Ogre@@IEBA_NPEAVCamera@2@@Z",instanceVisibleHook,originalInstanceVisible);
    HOOK("?setInUse@InstancedEntity@Ogre@@IEAAX_N@Z",instanceInUseHook,originalInstanceInUse);
    if(!ok) { logLine("INACTIVE: lifecycle hook installation failed.\r\n");return; }
    {
        void* target=GetProcAddress(pu,"?_updateRenderQueue@ParticleSystem@ParticleUniverse@@UEAAXPEAVRenderQueue@Ogre@@PEAVCamera@4@PEBV54@@Z");
        if(!target || addHook(target,particleQueueHook,(void**)&originalParticleQueue)!=0 || !originalParticleQueue) {
            logLine("INACTIVE: particle render diagnostic hook failed.\r\n");return;
        }
    }
    if(addHook(gpu,gpuHook,(void**)&originalGpu)!=0 || !originalGpu) { logLine("INACTIVE: GPU hook failed.\r\n");return; }
    {
        void* stub=GetProcAddress(lib,"?shoot@GunClass@@QEAAXPEAVCharacter@@PEAVRootObject@@W4StatsEnumerated@@AEBVVector3@Ogre@@@Z");
        void* target=stub?(void*)(U64)realAddress(stub):0;
        if(!target || addHook(target,shootMaterialHook,(void**)&originalShoot)!=0 || !originalShoot) {
            logLine("INACTIVE: native shoot material hook failed.\r\n");return;
        }
    }
    {
        void* stub=GetProcAddress(lib,"?addWound@MedicalSystem@@QEAAPEAVGameData@@_NW4CutDirection@@AEAVDamages@@AEAHPEAVRootObject@@AEAW4Enum@AttackDirection@@PEAVHarpoon@@@Z");
        void* target=stub?(void*)(U64)realAddress(stub):0;
        if(!target || addHook(target,woundHook,(void**)&originalWound)!=0 || !originalWound) {
            logLine("INACTIVE: character hit hook unavailable.\r\n");return;
        }
    }
    enabled=1;
    logLine("SESSION v0.23: projectile-parent local effects; per-instance hit visibility suppression.\r\n");
}

#ifdef AEV_TEST
/* Isolated state/lifecycle tests; this export is NOT in the shipped DLL. */
static int testDestroyedParticles,testDestroyedNodes,testPositioned;
static void* testMesh=(void*)0x1234;
static OldString testMeshName;
static void* testEntityMesh(void* e) { (void)e;return &testMesh; }
static const OldString* testResourceName(void* r) { (void)r;return &testMeshName; }
static void* testScene(void* n) { (void)n;return (void*)0x400; }
static Vec3* testPosition(void* n,Vec3* v) { (void)n;v->x=10;v->y=20;v->z=30;return v; }
static void* testManager(void) { return (void*)0x500; }
static void* testTemplate(void* m,const OldString* s) { (void)m;(void)s;return (void*)0x501; }
static void* testCreateParticles(void* m,const OldString* s) { (void)m;(void)s;return (void*)0x600; }
static void testDestroyParticles(void* m,void* p) { (void)m;(void)p;++testDestroyedParticles; }
static void testDestroyNode(void* m,void* n) { (void)m;(void)n;++testDestroyedNodes; }
static void testEmitting(void* p,unsigned char active) { (void)p;(void)active; }
static void* testCreateNode(void* s,int mode) { (void)s;(void)mode;return (void*)0x700; }
static void* testRoot(void* s,int mode) { (void)s;(void)mode;return (void*)0x800; }
static void testTwo(void* a,void* b) { (void)a;(void)b; }
static void testOne(void* a) { (void)a; }
static void testSetPosition(void* n,const Vec3* v) { (void)n;(void)v;++testPositioned; }
static char testPreviousName[16];
static int testPuCalls,testPuFailure;
static int testVisibilityCopies;
static int testQueueAssignments;
static void testSetQueue(void* object,unsigned char group) {
    if(object!=(void*)0x600 || group!=84)testPuFailure=1;
    ++testQueueAssignments;
}
static int testQueueCalls;
static int testShootCalls,testShootUsedFire;
static int testWounds,testHidden,testInstanceCalls;
static unsigned char testInstanceVisible(void* entity,void* camera) {(void)entity;(void)camera;return 1;}
static void testInstanceInUse(void* entity,unsigned char inUse) {(void)entity;(void)inUse;}
static void* testNodeParent(void* node) {(void)node;return (void*)0x800;}
static void* testInstanceCreate(void* manager,const OldString* material,int type) {
    (void)manager;(void)material;(void)type;++testInstanceCalls;return (void*)0xabcd;
}
static void* testWound(void* m,unsigned char l,int a,void* d,int* mat,void* at,int* dir,void* h) {
    (void)m;(void)l;(void)a;(void)d;(void)mat;(void)at;(void)dir;(void)h;++testWounds;return (void*)0x9876;
}
static void testNodeScale(void* node,const Vec3* value) {
    if(node!=(void*)0x1000)testPuFailure=1;
    if(value->x==0 && value->y==0 && value->z==0)++testHidden;
}
static int testCloneCalls,testTextureChanges;
static void* testCloneMaterial(void* source,MaterialRef* result,const OldString* name,unsigned char groupChange,const OldString* group) {
    (void)name;(void)groupChange;(void)group;
    if(source!=(void*)0x111)testPuFailure=1;
    result->object=(void*)0x333;result->control=(void*)0x444;++testCloneCalls;return result;
}
static void* testMaterialTechnique(void* m,unsigned short i) {if(m!=(void*)0x333 || i)testPuFailure=1;return (void*)0x555;}
static void* testTechniquePass(void* t,unsigned short i) {if(t!=(void*)0x555 || i)testPuFailure=1;return (void*)0x666;}
static void* testPassTexture(void* p,U64 i) {if(p!=(void*)0x666 || i)testPuFailure=1;return (void*)0x777;}
static void testTextureSet(void* u,const OldString* name,int type) {
    if(u!=(void*)0x777 || name->size!=12 || type!=2)testPuFailure=1;
    ++testTextureChanges;
}
static void testShoot(void* gun,void* actor,void* target,int stat,const Vec3* aim) {
    MaterialRef* slot=(MaterialRef*)((char*)gun+0x70);
    (void)actor;(void)target;(void)stat;(void)aim;++testShootCalls;
    testShootUsedFire=slot->object==flightMaterial.object;
}
static void* testMaterialParent;
static const OldString* testCurrentMaterial;
static int testMaterialCalls,testMaterialOverrides;
static void testOriginalSubMaterial(void* sub,const void* material) {
    (void)sub;testCurrentMaterial=(const OldString*)material;++testMaterialCalls;
}
static void* testSubParent(void* sub) { (void)sub;return testMaterialParent; }
static const OldString* testSubName(void* sub) { (void)sub;return testCurrentMaterial; }
static void testSubNameSet(void* sub,const OldString* name,const OldString* group) {
    (void)group;++testMaterialOverrides;subMaterialHook(sub,name);
}
static void testParticleQueue(void* p,void* q,void* c,void* l) {
    (void)p;(void)q;(void)c;(void)l;++testQueueCalls;
}
static unsigned int testGetVisibility(void* object) { return object==(void*)0x600?1:0x40; }
static void testSetVisibility(void* object,unsigned int flags) {
    if(object!=(void*)0x600 || flags!=0x40)testPuFailure=1;
    ++testVisibilityCopies;
}
static int testUpdates;
static void testPuUpdate(void* particles,float elapsed) {
    if(particles!=(void*)0x600 || elapsed<0 || elapsed>0.1f)testPuFailure=1;
    ++testUpdates;
}
static void* testPuCreate(void* manager,const OldString* unique,const OldString* name,void* scene) {
    unsigned int i;int different=0;
    if(manager!=(void*)0x500 || scene!=(void*)0x400 || name!=&templateName ||
       unique->size!=12 || unique->capacity!=15) testPuFailure=1;
    for(i=0;i<12;++i) if(unique->text.small[i]!=testPreviousName[i])different=1;
    if(testPuCalls && !different)testPuFailure=1;
    copyText(testPreviousName,unique->text.small);++testPuCalls;
    return (void*)0x600;
}
__declspec(dllexport) int runEmberTests(void) {
    void* fakeObjects[32];int i,count;
    entityVtable=(void*)0x1122;
    testMeshName.text.heap="AEV_fire_missile.mesh";
    testMeshName.size=length(testMeshName.text.heap);testMeshName.capacity=31;
    entityMesh=testEntityMesh;resourceName=testResourceName;
    nodeCreator=testScene;derivedPosition=testPosition;
    particleManager=testManager;particleTemplate=testTemplate;
    createParticles=testCreateParticles;destroyParticles=testDestroyParticles;setEmitting=testEmitting;
    createNode=testCreateNode;rootNode=testRoot;addChild=testTwo;setPosition=testSetPosition;
    originalAttach=testTwo;originalDetach=testTwo;originalDetachAll=testOne;
    beginParticles=testOne;
    originalDestroyEntity=testTwo;originalDestroyNode=testDestroyNode;originalClearScene=testOne;
    enabled=1;
    loadedAmmoTrails=1;
    puUpdate=testPuUpdate;
    getVisibilityFlags=testGetVisibility;setVisibilityFlags=testSetVisibility;
    setRenderQueue=testSetQueue;
    for(i=0;i<32;++i) {fakeObjects[i]=entityVtable;attachHook((void*)(U64)(0x1000+i),&fakeObjects[i]);}
    count=0;for(i=0;i<MAX_TRAILS;++i) if(trails[i].used)++count;
    if(count!=MAX_TRAILS)return 1;
    updateTrails();
    if(!trails[0].particles || testPositioned!=MAX_TRAILS)return 2;
    originalParticleQueue=testParticleQueue;
    particleQueueHook((void*)0x999,0,0,0);
    if(trails[0].rendered || testQueueCalls!=1)return 13;
    particleQueueHook(trails[0].particles,0,0,0);
    if(!trails[0].rendered || testQueueCalls!=2)return 14;
    detachHook((void*)0x1000,&fakeObjects[0]);updateTrails();
    if(!trails[0].stopAt || trails[0].projectileNode)return 3;
    trails[0].stopAt=GetTickCount64()-CLEANUP_DELAY_MS-1;updateTrails();
    if(trails[0].used || testDestroyedParticles!=1 || testDestroyedNodes!=1)return 4;
    destroyNodeHook((void*)0x400,(void*)0x1001);updateTrails();
    if(!trails[1].stopAt)return 5;
    clearSceneHook((void*)0x400);
    count=0;for(i=0;i<MAX_TRAILS;++i) if(trails[i].used)++count;
    if(count || testDestroyedParticles!=MAX_TRAILS || testDestroyedNodes!=MAX_TRAILS+1)return 6;
    fakeObjects[0]=entityVtable;
    testMeshName.text.heap="AEV_not_a_missile.mesh";testMeshName.size=length(testMeshName.text.heap);
    if(isMissile(&fakeObjects[0]))return 7;
    puCreate=testPuCreate;
    if(createPuParticles((void*)0x400,&templateName)!=(void*)0x600)return 8;
    if(createPuParticles((void*)0x400,&templateName)!=(void*)0x600)return 9;
    if(testPuFailure || testPuCalls!=2)return 10;
    if(testUpdates<MAX_TRAILS)return 11;
    if(testVisibilityCopies!=MAX_TRAILS)return 12;
    if(testQueueAssignments!=MAX_TRAILS)return 15;
    originalSubMaterial=testOriginalSubMaterial;subParent=testSubParent;
    getMaterialName=testSubName;subMaterialName=testSubNameSet;setRenderQueue=0;
    testMaterialParent=&fakeObjects[0];
    fireMaterial.text.heap="AEV_Missile_Fire";fireMaterial.size=16;fireMaterial.capacity=16;
    subMaterialHook((void*)0x900,&templateName);
    if(testMaterialCalls!=1 || testMaterialOverrides)return 16;
    testMeshName.text.heap="AEV_fire_missile.mesh";testMeshName.size=length(testMeshName.text.heap);
    subMaterialHook((void*)0x900,&templateName);
    if(testMaterialCalls!=3 || testMaterialOverrides!=1 || testCurrentMaterial!=&fireMaterial)return 17;
    {
        static U64 fakeGun[40];MaterialRef* slot=(MaterialRef*)((char*)fakeGun+0x70);
        OldString* ammo=(OldString*)((char*)fakeGun+0xb8);
        slot->object=(void*)0x111;slot->control=(void*)0x222;
        ammo->text.heap="AEV_fire_missile.mesh";ammo->size=length(ammo->text.heap);ammo->capacity=31;
        flightMaterial.object=(void*)0x333;flightMaterial.control=(void*)0x444;originalShoot=testShoot;
        flightMaterialEnabled=1;
        shootMaterialHook(fakeGun,0,0,0,0);
        if(testShootCalls!=1 || !testShootUsedFire || slot->object!=(void*)0x111 || slot->control!=(void*)0x222)return 18;
        ammo->text.heap="unrelated_missile.mesh";ammo->size=length(ammo->text.heap);
        shootMaterialHook(fakeGun,0,0,0,0);
        if(testShootCalls!=2 || testShootUsedFire)return 19;
        ammo->text.heap="AEV_fire_missile.mesh";ammo->size=length(ammo->text.heap);slot->object=0;
        shootMaterialHook(fakeGun,0,0,0,0);
        if(testShootCalls!=3 || testShootUsedFire || slot->object)return 20;
        slot->object=(void*)0x111;flightMaterialEnabled=0;
        shootMaterialHook(fakeGun,0,0,0,0);
        if(testShootCalls!=4 || testShootUsedFire || slot->object!=(void*)0x111)return 21;
        cloneMaterial=testCloneMaterial;materialTechnique=testMaterialTechnique;techniquePass=testTechniquePass;
        passTexture=testPassTexture;textureNameSet=testTextureSet;
        flightMaterial.object=0;flightMaterial.control=0;flightMaterialEnabled=1;
        shootMaterialHook(fakeGun,0,0,0,0);
        shootMaterialHook(fakeGun,0,0,0,0);
        if(testShootCalls!=6 || !testShootUsedFire || testCloneCalls!=1 || testTextureChanges!=1 ||
            slot->object!=(void*)0x111 || slot->control!=(void*)0x222 || testPuFailure)return 22;
    }
    {
        static U64 harpoon[20];OldString name;
        originalInstanceCreate=testInstanceCreate;originalWound=testWound;nodeScale=testNodeScale;
        originalInstanceVisible=testInstanceVisible;originalInstanceInUse=testInstanceInUse;
        localPosition=testPosition;nodeParent=testNodeParent;
        name.text.heap="UnrelatedFlightMaterial";name.size=length(name.text.heap);name.capacity=name.size;
        instanceCreateHook(0,&name,0);
        for(i=0;i<MAX_TRAILS;++i)if(trails[i].used)return 23;
        name.text.heap="AEV_NativeFlight_Fire";name.size=length(name.text.heap);name.capacity=name.size;
        instanceCreateHook(0,&name,0);attachHook((void*)0x1000,(void*)0xabcd);
        if(!trails[0].flying || trails[0].projectileNode!=(void*)0x1000)return 24;
        *(void**)((char*)harpoon+0x50)=(void*)0xdead;
        *(void**)((char*)harpoon+0x58)=(void*)0x1000;
        if(woundHook(0,0,0,0,0,0,0,harpoon)!=(void*)0x9876 || testHidden || trails[0].hitPending)return 25;
        *(void**)((char*)harpoon+0x50)=(void*)0xabcd;
        if(woundHook(0,0,0,0,0,0,0,harpoon)!=(void*)0x9876 || !trails[0].hitPending)return 26;
        if(instanceVisibleHook((void*)0xabcd,0) || !instanceVisibleHook((void*)0xdead,0))return 31;
        if(testWounds!=2 || testInstanceCalls!=2 || testPuFailure)return 27;
        updateTrails();
        if(!trails[0].impact || trails[0].entity || !trails[0].particles || trails[0].hitPosition.x!=10 || trails[0].effectParent!=(void*)0x800)return 28;
        trails[0].born=GetTickCount64()-4601;updateTrails();
        if(!trails[0].stopAt)return 29;
        trails[0].stopAt=GetTickCount64()-CLEANUP_DELAY_MS-1;updateTrails();
        if(trails[0].used)return 30;
        instanceInUseHook((void*)0xabcd,1);
        if(!instanceVisibleHook((void*)0xabcd,0))return 32;
        clearSceneHook((void*)0x400);
    }
    enabled=0;return 0;
}
#endif
