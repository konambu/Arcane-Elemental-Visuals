/* Independent Earth/Fire Vortex companion. Pinned fireball DLL stays untouched. */
typedef unsigned long DWORD;
typedef unsigned long long U64;
typedef void* MODULE;
__declspec(dllimport) MODULE __stdcall GetModuleHandleA(const char*);
__declspec(dllimport) void* __stdcall GetProcAddress(MODULE,const char*);
__declspec(dllimport) DWORD __stdcall GetModuleFileNameA(MODULE,char*,DWORD);
__declspec(dllimport) DWORD __stdcall GetCurrentThreadId(void);
__declspec(dllimport) U64 __stdcall GetTickCount64(void);
__declspec(dllimport) void __stdcall AcquireSRWLockExclusive(void*);
__declspec(dllimport) void __stdcall ReleaseSRWLockExclusive(void*);
__declspec(dllimport) void* __stdcall CreateFileA(const char*,DWORD,DWORD,void*,DWORD,DWORD,void*);
__declspec(dllimport) int __stdcall WriteFile(void*,const void*,DWORD,DWORD*,void*);
__declspec(dllimport) int __stdcall CloseHandle(void*);
int _fltused=0;
/* VC100 string: 32-byte SSO/size/capacity payload plus allocator slot (40 total).
 * Only the payload is read from borrowed resource-name strings. */
typedef struct {union {char small[16];const char* heap;} text;U64 size,capacity,allocator;} OldString;
typedef struct {float x,y,z;} Vec3;
typedef struct {float w,x,y,z;} Quat;
typedef struct {void* scene;void* parent;void* node;void* particles;Vec3 position,axis;U64 born;int used,grounded;} Burst;
#define BURSTS 24
static Burst bursts[BURSTS];
static void* stateLock;
static int active,firstCapture,firstRender,firstEmit,missing,groundSkipLogged;
static DWORD internalThread;
static char logPath[1024];
static void* entityVtable;
static void (*entityMaterial)(void*,const OldString*,const OldString*);
static void* (*getSubEntity)(void*,U64);
static const OldString* (*subMaterialName)(void*);
static void (*setVisible)(void*,unsigned char);
static void* (*createRockEntity)(void*,const OldString*,const OldString*,int);
static void (*destroyRockEntity)(void*,void*);
static void (*setRockScale)(void*,const Vec3*);
static void (*setRockOrientation)(void*,float,float,float,float);
static void (*rockShadows)(void*,unsigned char);
static OldString earthMaterial,earthGroup;
static int earthMaterialLogged,windMaterialLogged;
static OldString templateName={{"AEV_EarthBurst"},14,15};
static unsigned int serial;
static void (*originalAttach)(void*,void*);
static void (*originalGpu)(void*,float);
static void (*originalClear)(void*);
static void (*originalDetachAll)(void*);
static void (*originalDetach)(void*,void*);
static void (*originalDestroyNode)(void*,void*);
static void (*stopFade)(void*);
static int (*addHook)(void*,void*,void**);
static long long (*realAddress)(void*);
static void* (*entityMesh)(void*);
static const OldString* (*resourceName)(void*);
static void* (*nodeCreator)(void*);
static void* (*nodeParent)(void*);
static Vec3* (*localPosition)(void*,Vec3*);
static Vec3* (*localScale)(void*,Vec3*);
static Quat* (*localOrientation)(void*,Quat*);
static unsigned char (*groundQuery)(Vec3*);
static void* (*particleManager)(void);
static void* (*particleTemplate)(void*,const OldString*);
static void* (*createParticles)(void*,const OldString*,const OldString*,void*);
static void (*destroyParticles)(void*,void*,void*);
static void (*startParticles)(void*);
static void (*updateParticles)(void*,float);
static void (*keepLocal)(void*,unsigned char);
static void (*autoBounds)(void*,unsigned char,float);
static U64 (*particleCount)(void*);
static void* (*createNode)(void*,int);
static void* (*rootNode)(void*,int);
static void (*addChild)(void*,void*);
static void (*setPosition)(void*,const Vec3*);
static void (*destroyNode)(void*,void*);
static void (*setQueue)(void*,unsigned char);
static void (*setAabb)(void*,const float*);
static void (*originalRender)(void*,void*,void*,void*);
static U64 lastTick;
static unsigned int length(const char* s) {unsigned int n=0;while(s[n])++n;return n;}
static void logLine(const char* s) {
    DWORD n;void* file=CreateFileA(logPath,4,3,0,4,0x80,0);
    if(file!=(void*)(U64)-1) {WriteFile(file,s,length(s),&n,0);CloseHandle(file);}
}
static void stringRef(OldString* out,const char* value) {
    U64 i;out->size=length(value);out->capacity=out->size<16?15:out->size;out->allocator=0;
    if(out->capacity<16)for(i=0;i<=out->size;++i)out->text.small[i]=value[i];
    else out->text.heap=value;
}
static void logMaterialName(const char* label,void* entity) {
    const OldString* name=subMaterialName(getSubEntity(entity,0));const char* data;
    char line[192];U64 i,n;
    logLine(label);if(!name || name->size>1024 || name->capacity<name->size)return;
    data=name->capacity<16?name->text.small:name->text.heap;if(!data)return;
    n=name->size<185?name->size:185;
    for(i=0;i<n;++i)line[i]=data[i];line[n++]='\r';line[n++]='\n';line[n]=0;logLine(line);
}
static void lock(void) {AcquireSRWLockExclusive(&stateLock);}
static void unlock(void) {ReleaseSRWLockExclusive(&stateLock);}
static int matches(const OldString* s) {
    const char* name="AM_EarthSpike_v001.mesh";const char* data;U64 i,n=length(name);
    if(!s || s->size<n || s->size>1024 || s->capacity<s->size)return 0;
    data=s->capacity<16?s->text.small:s->text.heap;if(!data)return 0;
    for(i=0;i<n;++i)if(data[s->size-n+i]!=name[i])return 0;return 1;
}
static Vec3 cross(Vec3 a,Vec3 b) {
    Vec3 result={a.y*b.z-a.z*b.y,a.z*b.x-a.x*b.z,a.x*b.y-a.y*b.x};return result;
}
static Vec3 spikeAxis(void* node) {
    Vec3 scale,v,qv,t,u;Quat q;
    localScale(node,&scale);localOrientation(node,&q);
    /* Installed Arcane DLL aligns this scaled mesh tip with its emergence ray. */
    v.x=1.158552885f*scale.x;v.y=20*scale.y;v.z=-.294518113f*scale.z;
    qv.x=q.x;qv.y=q.y;qv.z=q.z;t=cross(qv,v);u=cross(qv,t);
    v.x+=2*(q.w*t.x+u.x);v.y+=2*(q.w*t.y+u.y);v.z+=2*(q.w*t.z+u.z);return v;
}
static int groundBurst(Burst* b) {
    Vec3 origin=b->position,sample=origin;float low=0,high=3,mid;int i;
    if(!groundQuery || !groundQuery(&sample))return 0;
    if(origin.y>=sample.y-.05f) {sample.y+=.25f;b->position=sample;return 1;}
    if(!(b->axis.y>.01f))return 0;
    sample.x=origin.x+high*b->axis.x;sample.y=origin.y+high*b->axis.y;sample.z=origin.z+high*b->axis.z;
    {float height=sample.y;if(!groundQuery(&sample) || height<sample.y)return 0;}
    /* Bound the native terrain queries; no target/actor pointers are retained. */
    for(i=0;i<7;++i) {
        float height;mid=(low+high)*.5f;
        sample.x=origin.x+mid*b->axis.x;sample.y=origin.y+mid*b->axis.y;sample.z=origin.z+mid*b->axis.z;
        height=sample.y;if(!groundQuery(&sample))return 0;
        if(height<sample.y)low=mid;else high=mid;
    }
    sample.y+=.25f;b->position=sample;return 1;
}
#include "EarthRock.inl"
#include "MissileFlame.inl"
#include "FireVortex.inl"
static void attachHook(void* node,void* object) {
    void* scene;void* parent;Vec3 position,axis;int i;
    originalAttach(node,object);
    if(!active || internalThread==GetCurrentThreadId() || !object)return;
    __try {
        void** mesh;
        if(*(void**)object!=entityVtable)return;
        mesh=(void**)entityMesh(object);if(!mesh || !*mesh)return;
        if(stormMatches(resourceName(*mesh))) {captureStorm(node,object);return;}
        if(!matches(resourceName(*mesh)))return;
        if(!earthMaterialLogged)logMaterialName("EARTH MATERIAL before: ",object);
        /* Resource groups can reuse the stock-named mesh. Bind our material explicitly. */
        entityMaterial(object,&earthMaterial,&earthGroup);
        if(!earthMaterialLogged) {
            logMaterialName("EARTH MATERIAL after: ",object);earthMaterialLogged=1;
        }
        scene=nodeCreator(node);parent=nodeParent(node);localPosition(node,&position);
        axis=spikeAxis(node);
        captureRock(node,object,scene,parent);
        lock();
        for(i=0;i<BURSTS;++i)if(!bursts[i].used) {
            bursts[i].used=1;bursts[i].scene=scene;bursts[i].parent=parent;
            bursts[i].position=position;bursts[i].axis=axis;bursts[i].grounded=0;
            bursts[i].born=GetTickCount64();break;
        }
        unlock();
        if(!firstCapture) {firstCapture=1;logLine("CAPTURE: actual Earth Spike mesh; ground-local dust queued.\r\n");}
    } __except(1) {logLine("SKIP: Earth attachment metadata unavailable.\r\n");}
}
static void erase(Burst* b) {
    if(b->particles)destroyParticles(particleManager(),b->particles,b->scene);
    if(b->node)destroyNode(b->scene,b->node);
    b->particles=0;b->node=0;b->scene=0;b->parent=0;b->used=0;
}
static void renderHook(void* particle,void* queue,void* camera,void* lod) {
    int i;
    if(!firstRender)for(i=0;i<BURSTS;++i)if(bursts[i].used && bursts[i].particles==particle) {
        firstRender=1;logLine("RENDER: Earth dust reached native particle render queue.\r\n");break;
    }
    stormRender(particle);
    missileFlameRender(particle);
    originalRender(particle,queue,camera,lod);
}
static void update(void) {
    U64 now=GetTickCount64();float dt=lastTick?(float)(now-lastTick)*.001f:.016f;int i;
    const float bounds[6]={0,0,0,40,40,40};
    lastTick=now;if(dt>.1f)dt=.1f;
    lock();internalThread=GetCurrentThreadId();
    __try {
    for(i=0;i<BURSTS;++i)if(bursts[i].used) {
        Burst* b=&bursts[i];void* manager=particleManager();
        if(now-b->born>=1200) {erase(b);continue;}
        if(!b->particles) {
            OldString unique={{"AEV_D00000000"},13,15};unsigned int j,n=++serial;
            if(!b->grounded) {
                if(!groundBurst(b)) {
                    if(!groundSkipLogged) {groundSkipLogged=1;logLine("SKIP: native terrain unavailable or no Earth spike/ground intersection.\r\n");}
                    erase(b);continue;
                }
                b->grounded=1;
            }
            if(!manager || !particleTemplate(manager,&templateName)) {
                if(!missing) {missing=1;logLine("SKIP: AEV_EarthBurst template missing.\r\n");}
                erase(b);continue;
            }
            for(j=0;j<8;++j)unique.text.small[5+j]="0123456789abcdef"[(n>>(28-j*4))&15];
            b->particles=createParticles(manager,&unique,&templateName,b->scene);
            b->node=createNode(b->scene,0);
            if(!b->particles || !b->node) {erase(b);continue;}
            addChild(b->parent?b->parent:rootNode(b->scene,0),b->node);
            setPosition(b->node,&b->position);setQueue(b->particles,84);
            originalAttach(b->node,b->particles);keepLocal(b->particles,0);
            startParticles(b->particles);autoBounds(b->particles,0,0);
        }
        updateParticles(b->particles,dt);setAabb(b->particles,bounds);
        if(!firstEmit && particleCount(b->particles)>0) {firstEmit=1;logLine("EMIT: Earth dust/debris particles are alive.\r\n");}
    }
    updateRocksSafe(now);updateStorms(now,dt);updateMissileFlamesSafe(now,dt);
    } __finally {internalThread=0;unlock();}
}
static void gpuHook(void* world,float dt) {
    originalGpu(world,dt);if(!active)return;
    __try {update();}
    __except(1) {active=0;logLine("DISABLED: companion update failed. Fireball plugin unchanged.\r\n");}
}
static void clearHook(void* scene) {
    int i;
    if(internalThread==GetCurrentThreadId()) {originalClear(scene);return;}
    lock();internalThread=GetCurrentThreadId();
    __try {
        for(i=0;i<BURSTS;++i)if(bursts[i].used && bursts[i].scene==scene)erase(&bursts[i]);
        clearStorms(scene);
        clearRocks(scene);
        clearMissileFlames(scene);
    } __finally {internalThread=0;unlock();}
    originalClear(scene);
}
static int moduleMatches(MODULE module,unsigned int stamp,unsigned int size) {
    unsigned char* b=(unsigned char*)module;unsigned int off;
    if(!b || *(unsigned short*)b!=0x5a4d)return 0;
    off=*(unsigned int*)(b+0x3c);
    return *(unsigned int*)(b+off)==0x4550 && *(unsigned int*)(b+off+8)==stamp && *(unsigned int*)(b+off+80)==size;
}
__declspec(dllexport) void startPlugin(void) {
    MODULE ogre=GetModuleHandleA("OgreMain_x64.dll"),pu=GetModuleHandleA("Plugin_ParticleUniverse_x64.dll"),lib=GetModuleHandleA("KenshiLib.dll");
    MODULE arcane=GetModuleHandleA("ArcaneMagicNative.dll");
    MODULE self=GetModuleHandleA("AEV_EarthStorm-v4.dll");DWORD i,last=0;int ok=1;void* stub;void* target;
    if(active || originalAttach)return;
    if(!self || !GetModuleFileNameA(self,logPath,sizeof(logPath)))return;
    stringRef(&earthMaterial,"EV_EarthSpike_Charcoal_v001");stringRef(&earthGroup,"ArcaneEarthSpikeV001");
    stringRef(&rockMesh,"AEV_EarthRock_v3.mesh");
    stringRef(&missileTemplate,"AEV_MissileFlame");
    for(i=0;logPath[i];++i)if(logPath[i]=='\\' || logPath[i]=='/')last=i;
    {const char* n="EarthStorm.log";i=0;do {logPath[last+1+i]=n[i];}while(n[i++]);}
    if(!ogre || !pu || !lib) {logLine("INACTIVE: required runtime modules absent.\r\n");return;}
    if(!moduleMatches(arcane,0x6ae2bea8,0x55000)) {
        logLine("INACTIVE: unreviewed Arcane Magic build; source lifecycle assumptions not applied.\r\n");return;
    }
    if(*(U64*)((unsigned char*)arcane+0x9250)!=0x8b4820ec83485340ULL) {
        logLine("INACTIVE: original terrain query entry differs from audited bytes.\r\n");return;
    }
    groundQuery=(unsigned char (*)(Vec3*))((unsigned char*)arcane+0x9250);
    if(!moduleMatches(ogre,0x5ca5f929,0x9c9000) || !moduleMatches(pu,0x5ca5faa4,0x718000)) {
        logLine("INACTIVE: unreviewed Ogre/ParticleUniverse ABI.\r\n");return;
    }
#define API(field,module,symbol) *(void**)(&field)=GetProcAddress(module,symbol);if(!field)ok=0
    API(addHook,lib,"?AddHook@KenshiLib@@YA?AW4HookStatus@1@PEAX0PEAPEAX@Z");
    API(realAddress,lib,"?GetRealAddress@KenshiLib@@YA_JPEAX@Z");
    entityVtable=GetProcAddress(ogre,"??_7Entity@Ogre@@6BMovableObject@1@@");if(!entityVtable)ok=0;
    API(entityMesh,ogre,"?getMesh@Entity@Ogre@@QEBAAEBV?$SharedPtr@VMesh@Ogre@@@2@XZ");
    API(entityMaterial,ogre,"?setMaterialName@Entity@Ogre@@QEAAXAEBV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@0@Z");
    API(getSubEntity,ogre,"?getSubEntity@Entity@Ogre@@QEAAPEAVSubEntity@2@_K@Z");
    API(subMaterialName,ogre,"?getMaterialName@SubEntity@Ogre@@QEBAAEBV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@XZ");
    API(setVisible,ogre,"?setVisible@MovableObject@Ogre@@QEAAX_N@Z");
    API(createRockEntity,ogre,"?createEntity@SceneManager@Ogre@@UEAAPEAVEntity@2@AEBV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@0W4SceneMemoryMgrTypes@2@@Z");
    API(destroyRockEntity,ogre,"?destroyEntity@SceneManager@Ogre@@UEAAXPEAVEntity@2@@Z");
    API(setRockScale,ogre,"?setScale@Node@Ogre@@QEAAXAEBVVector3@2@@Z");
    API(setRockOrientation,ogre,"?setOrientation@Node@Ogre@@QEAAXMMMM@Z");
    API(rockShadows,ogre,"?setCastShadows@MovableObject@Ogre@@QEAAX_N@Z");
    API(resourceName,ogre,"?getName@Resource@Ogre@@UEBAAEBV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@XZ");
    API(nodeCreator,ogre,"?getCreator@SceneNode@Ogre@@QEBAPEAVSceneManager@2@XZ");
    API(nodeParent,ogre,"?getParent@Node@Ogre@@QEBAPEAV12@XZ");
    API(localPosition,ogre,"?getPosition@Node@Ogre@@QEBA?AVVector3@2@XZ");
    API(localScale,ogre,"?getScale@Node@Ogre@@QEBA?AVVector3@2@XZ");
    API(localOrientation,ogre,"?getOrientation@Node@Ogre@@QEBA?AVQuaternion@2@XZ");
    API(normaliseVector,ogre,"?normalise@Vector3@Ogre@@QEAAMXZ");
    API(normaliseQuaternion,ogre,"?normalise@Quaternion@Ogre@@QEAAMXZ");
    API(puParentNode,ogre,"?getParentSceneNode@MovableObject@Ogre@@QEBAPEAVSceneNode@2@XZ");
    API(puTemplateName,pu,"?getTemplateName@ParticleSystem@ParticleUniverse@@QEBAAEBV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@XZ");
    API(particleManager,pu,"?getSingletonPtr@ParticleSystemManager@ParticleUniverse@@SAPEAV12@XZ");
    API(particleTemplate,pu,"?getParticleSystemTemplate@ParticleSystemManager@ParticleUniverse@@QEAAPEAVParticleSystem@2@AEBV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@@Z");
    API(createParticles,pu,"?createParticleSystem@ParticleSystemManager@ParticleUniverse@@QEAAPEAVParticleSystem@2@AEBV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@0PEAVSceneManager@Ogre@@@Z");
    API(destroyParticles,pu,"?destroyParticleSystem@ParticleSystemManager@ParticleUniverse@@QEAAXPEAVParticleSystem@2@PEAVSceneManager@Ogre@@@Z");
    API(stopFade,pu,"?stopFade@ParticleSystem@ParticleUniverse@@QEAAXXZ");
    API(startParticles,pu,"?start@ParticleSystem@ParticleUniverse@@QEAAXXZ");
    API(updateParticles,pu,"?_update@ParticleSystem@ParticleUniverse@@QEAAXM@Z");
    API(keepLocal,pu,"?setKeepLocal@ParticleSystem@ParticleUniverse@@QEAAX_N@Z");
    API(autoBounds,pu,"?setBoundsAutoUpdated@ParticleSystem@ParticleUniverse@@QEAAX_NM@Z");
    API(particleCount,pu,"?getNumberOfEmittedParticles@ParticleSystem@ParticleUniverse@@QEAA_KXZ");
    API(createNode,ogre,"?createSceneNode@SceneManager@Ogre@@UEAAPEAVSceneNode@2@W4SceneMemoryMgrTypes@2@@Z");
    API(rootNode,ogre,"?getRootSceneNode@SceneManager@Ogre@@QEAAPEAVSceneNode@2@W4SceneMemoryMgrTypes@2@@Z");
    API(addChild,ogre,"?addChild@Node@Ogre@@QEAAXPEAV12@@Z");
    API(setPosition,ogre,"?setPosition@Node@Ogre@@QEAAXAEBVVector3@2@@Z");
    API(destroyNode,ogre,"?destroySceneNode@SceneManager@Ogre@@UEAAXPEAVSceneNode@2@@Z");
    API(setQueue,ogre,"?setRenderQueueGroup@MovableObject@Ogre@@UEAAXE@Z");
    API(setAabb,ogre,"?setLocalAabb@MovableObject@Ogre@@QEAAXUAabb@2@@Z");
    stub=GetProcAddress(lib,"?_NV_mainLoop_GPUSensitiveStuff@GameWorld@@QEAAXM@Z");
    if(!ok || !stub) {logLine("INACTIVE: required API absent.\r\n");return;}
#define HOOK(module,symbol,detour,original) target=GetProcAddress(module,symbol);if(!target || addHook(target,detour,(void**)&original)!=0 || !original)ok=0
    HOOK(ogre,"?attachObject@SceneNode@Ogre@@QEAAXPEAVMovableObject@2@@Z",attachHook,originalAttach);
    HOOK(ogre,"?detachAllObjects@SceneNode@Ogre@@UEAAXXZ",detachAllHook,originalDetachAll);
    HOOK(ogre,"?detachObject@SceneNode@Ogre@@QEAAXPEAVMovableObject@2@@Z",detachHook,originalDetach);
    HOOK(ogre,"?destroySceneNode@SceneManager@Ogre@@UEAAXPEAVSceneNode@2@@Z",destroyNodeHook,originalDestroyNode);
    HOOK(ogre,"?clearScene@SceneManager@Ogre@@UEAAXXZ",clearHook,originalClear);
    HOOK(pu,"?_updateRenderQueue@ParticleSystem@ParticleUniverse@@UEAAXPEAVRenderQueue@Ogre@@PEAVCamera@4@PEBV54@@Z",renderHook,originalRender);
    HOOK(pu,"?start@ParticleSystem@ParticleUniverse@@QEAAXXZ",puStartHook,originalPuStart);
    HOOK(pu,"?stopFade@ParticleSystem@ParticleUniverse@@QEAAXXZ",puFadeHook,originalPuFade);
    HOOK(pu,"?destroyParticleSystem@ParticleSystemManager@ParticleUniverse@@QEAAXPEAVParticleSystem@2@PEAVSceneManager@Ogre@@@Z",puDestroyHook,originalPuDestroy);
    target=(void*)(U64)realAddress(stub);
    if(!ok || !target || addHook(target,gpuHook,(void**)&originalGpu)!=0 || !originalGpu) {logLine("INACTIVE: hook chain unavailable.\r\n");return;}
    active=1;logLine("ACTIVE v4: flight-aligned larger missile flame; accepted projectile/impact, rock and wind preserved.\r\n");
}
int __stdcall DllMain(MODULE module,DWORD reason,void* reserved) {(void)module;(void)reason;(void)reserved;return 1;}

#ifdef AEV_EARTH_TEST
static void* fakeMesh=(void*)0x234;
static OldString fakeName;
static int attachments,creations,destructions,nodesRemoved,updates,parents,positioned,clears;
static int rockAssignments,windHidden;
static OldString testMaterialName;
static void testEntityMaterial(void* object,const OldString* material,const OldString* group) {
    (void)object;
    if(material->size==27 && group->size==20 && material->capacity>=16 && group->capacity>=16)++rockAssignments;
    testMaterialName=*material;
}
static void* testSubEntity(void* object,U64 i) {(void)object;(void)i;return (void*)0xabcd;}
static const OldString* testMaterial(void* object) {(void)object;return &testMaterialName;}
static void testVisible(void* object,unsigned char visible) {(void)object;if(!visible)++windHidden;}
static void testAttach(void* n,void* o) {(void)n;(void)o;++attachments;}
static void* testMesh(void* e) {(void)e;return &fakeMesh;}
static const OldString* testName(void* mesh) {(void)mesh;return &fakeName;}
static void* testCreator(void* n) {(void)n;return (void*)0x400;}
static void* testParent(void* n) {(void)n;return (void*)0x800;}
static Vec3* testPosition(void* n,Vec3* p) {(void)n;p->x=10;p->y=20;p->z=30;return p;}
static Vec3* testScale(void* n,Vec3* p) {(void)n;p->x=1;p->y=2;p->z=1;return p;}
static Quat* testOrientation(void* n,Quat* q) {(void)n;q->w=1;q->x=q->y=q->z=0;return q;}
static unsigned char testGround(Vec3* p) {p->y=44;return 1;}
static void* testManager(void) {return (void*)0x500;}
static void* testTemplate(void* m,const OldString* n) {(void)m;(void)n;return (void*)0x501;}
static void* testCreate(void* m,const OldString* unique,const OldString* n,void* scene) {
    (void)m;(void)n;
    if(unique->size!=13 || unique->capacity!=15 || scene!=(void*)0x400)return 0;
    ++creations;return (void*)0x600;
}
static void testDestroy(void* m,void* p,void* scene) {(void)m;(void)p;(void)scene;++destructions;}
static void testStart(void* p) {(void)p;}
static void testUpdate(void* p,float dt) {(void)p;if(dt>=0 && dt<=.1f)++updates;}
static void testKeep(void* p,unsigned char keep) {(void)p;(void)keep;}
static void testBounds(void* p,unsigned char automatic,float time) {(void)p;(void)automatic;(void)time;}
static U64 testCount(void* p) {(void)p;return 1;}
static void* testCreateNode(void* scene,int memory) {(void)scene;(void)memory;return (void*)0x700;}
static void* testRoot(void* scene,int memory) {(void)scene;(void)memory;return (void*)0x900;}
static void testAddChild(void* parent,void* child) {if(parent==(void*)0x800 && child==(void*)0x700)++parents;}
static void testSetPosition(void* node,const Vec3* p) {
    if(node==(void*)0x700 && p->x>10.65f && p->x<10.75f && p->y==44.25f && p->z>29.8f && p->z<29.85f)++positioned;
}
static void testDestroyNode(void* scene,void* node) {(void)scene;(void)node;++nodesRemoved;}
static void testQueue(void* p,unsigned char q) {(void)p;(void)q;}
static void testAabb(void* p,const float* b) {(void)p;(void)b;}
static void testClear(void* scene) {(void)scene;++clears;}
__declspec(dllexport) int runEarthTests(void) {
    void* objects[30];int i,count;
    entityVtable=(void*)0x1234;entityMesh=testMesh;resourceName=testName;
    entityMaterial=testEntityMaterial;getSubEntity=testSubEntity;subMaterialName=testMaterial;setVisible=testVisible;
    stringRef(&earthMaterial,"EV_EarthSpike_Charcoal_v001");stringRef(&earthGroup,"ArcaneEarthSpikeV001");
    stringRef(&testMaterialName,"AM_EarthSpike_Charcoal_v001");
    nodeCreator=testCreator;nodeParent=testParent;localPosition=testPosition;
    localScale=testScale;localOrientation=testOrientation;groundQuery=testGround;
    originalAttach=testAttach;originalClear=testClear;
    particleManager=testManager;particleTemplate=testTemplate;createParticles=testCreate;
    destroyParticles=testDestroy;startParticles=testStart;updateParticles=testUpdate;
    keepLocal=testKeep;autoBounds=testBounds;particleCount=testCount;
    createNode=testCreateNode;rootNode=testRoot;addChild=testAddChild;setPosition=testSetPosition;
    destroyNode=testDestroyNode;setQueue=testQueue;setAabb=testAabb;
    active=1;rockEnabled=0;
    fakeName.text.heap="AM_NoMagic_v001.mesh";fakeName.size=length(fakeName.text.heap);fakeName.capacity=31;
    objects[0]=entityVtable;attachHook((void*)0x1000,&objects[0]);
    if(bursts[0].used || attachments!=1)return 1;
    fakeName.text.heap="AM_EarthSpike_v001.mesh";fakeName.size=length(fakeName.text.heap);
    for(i=0;i<30;++i) {objects[i]=entityVtable;attachHook((void*)0x1000,&objects[i]);}
    count=0;for(i=0;i<BURSTS;++i)if(bursts[i].used)++count;
    if(count!=BURSTS || bursts[0].position.y!=20 || bursts[0].parent!=(void*)0x800 || rockAssignments!=30 || windHidden)return 2;
    update();
    if(creations!=BURSTS || parents!=BURSTS || positioned!=BURSTS || updates!=BURSTS)return 3;
    bursts[0].born=GetTickCount64()-1201;update();
    if(bursts[0].used || destructions!=1 || nodesRemoved!=1)return 4;
    clearHook((void*)0x400);
    for(i=0;i<BURSTS;++i)if(bursts[i].used)return 5;
    if(destructions!=BURSTS || nodesRemoved!=BURSTS || clears!=1)return 6;
    rockEnabled=1;return 0;
}
#include "FireVortexTests.inl"
#include "MissileFlameTests.inl"
#endif
