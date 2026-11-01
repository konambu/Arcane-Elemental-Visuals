/* Included after the common Ogre/PU API declarations. No combat or actor writes. */
typedef struct {
    void* source;void* object;void* scene;void* parent;void* node;void* particles;
    Vec3 position;U64 stopped;int used,ending;
} Storm;
#define STORMS 8
static Storm storms[STORMS];
static OldString stormTemplate={{"AEV_FireVortex"},14,15};
static int stormCaptureLogged,stormEmitLogged,stormRenderLogged,stormLimitLogged;
static int stormMissingLogged,stormStopLogged;
static int stormMatches(const OldString* s) {
    const char* expected="AM_Whirlwind_v001.mesh";const char* data;U64 i,n=length(expected);
    if(!s || s->size<n || s->size>1024 || s->capacity<s->size)return 0;
    data=s->capacity<16?s->text.small:s->text.heap;if(!data)return 0;
    for(i=0;i<n;++i)if(data[s->size-n+i]!=expected[i])return 0;
    return 1;
}
static void captureStorm(void* node,void* object) {
    void* scene=nodeCreator(node);void* parent=nodeParent(node);Vec3 position;int i,slot=-1,duplicate=0;
    localPosition(node,&position);
    /* This is the original mesh's transparent fire shell, not its damage logic. */
    setQueue(object,84);
    lock();
    __try {
        for(i=0;i<STORMS;++i) {
            if(storms[i].used && storms[i].source==node)duplicate=1;
            if(!storms[i].used && slot<0)slot=i;
        }
        if(!duplicate && slot>=0) {
            Storm* s=&storms[slot];s->used=1;s->source=node;s->object=object;s->scene=scene;
            s->parent=parent;s->position=position;s->stopped=0;s->ending=0;
            if(!stormCaptureLogged) {stormCaptureLogged=1;logLine("WIND CAPTURE: actual Whirlwind mesh; native fire vortex queued.\r\n");}
        }
        if(!duplicate && slot<0 && !stormLimitLogged) {stormLimitLogged=1;logLine("WIND LIMIT: all 8 effect slots busy; gameplay continues normally.\r\n");}
    } __finally {unlock();}
}
static void stopStorms(void* node,void* object) {
    int i;
    for(i=0;i<STORMS;++i)if(storms[i].used && storms[i].source &&
        ((node && storms[i].source==node) || (object && storms[i].object==object))) {
        /* Forget the source before the engine frees it. Only owned effects remain. */
        storms[i].source=0;storms[i].object=0;storms[i].stopped=GetTickCount64();
    }
}
static void detachAllHook(void* node) {
    if(internalThread!=GetCurrentThreadId()) {lock();stopStorms(node,0);stopRocks(node,0);stopMissileFlame(0,node);unlock();}
    originalDetachAll(node);
}
static void detachHook(void* node,void* object) {
    if(internalThread!=GetCurrentThreadId()) {lock();stopStorms(0,object);stopRocks(0,object);unlock();}
    originalDetach(node,object);
}
static void destroyNodeHook(void* scene,void* node) {
    if(internalThread!=GetCurrentThreadId()) {lock();stopStorms(node,0);stopRocks(node,0);stopMissileFlame(0,node);unlock();}
    originalDestroyNode(scene,node);
}
static void eraseStorm(Storm* s) {
    if(s->particles)destroyParticles(particleManager(),s->particles,s->scene);
    if(s->node)destroyNode(s->scene,s->node);
    s->particles=0;s->node=0;s->source=0;s->object=0;s->parent=0;s->scene=0;
    s->used=0;s->ending=0;s->stopped=0;
}
static void clearStorms(void* scene) {
    int i;for(i=0;i<STORMS;++i)if(storms[i].used && storms[i].scene==scene)eraseStorm(&storms[i]);
}
static void stormRender(void* particle) {
    int i;
    if(!stormRenderLogged)for(i=0;i<STORMS;++i)if(storms[i].used && storms[i].particles==particle) {
        stormRenderLogged=1;logLine("WIND RENDER: own fire vortex reached native particle render queue.\r\n");break;
    }
}
static void updateStorms(U64 now,float dt) {
    const float bounds[6]={0,36,0,80,80,80};int i;
    for(i=0;i<STORMS;++i)if(storms[i].used) {
        Storm* s=&storms[i];void* manager=particleManager();
        if(!s->source && (!s->particles || now-s->stopped>=1600)) {eraseStorm(s);continue;}
        if(s->source) {
            /* Same parent + LOCAL position; never apply root/world offset twice. */
            localPosition(s->source,&s->position);
        }
        if(!s->particles) {
            OldString unique={{"AEV_W00000000"},13,15};unsigned int j,n=++serial;
            if(!manager || !particleTemplate(manager,&stormTemplate)) {
                if(!stormMissingLogged) {stormMissingLogged=1;logLine("WIND SKIP: AEV_FireVortex template missing; fire shell still works.\r\n");}
                eraseStorm(s);continue;
            }
            for(j=0;j<8;++j)unique.text.small[5+j]="0123456789abcdef"[(n>>(28-j*4))&15];
            s->particles=createParticles(manager,&unique,&stormTemplate,s->scene);
            s->node=createNode(s->scene,0);
            if(!s->particles || !s->node) {eraseStorm(s);continue;}
            /* Sibling node avoids inheriting the original mesh's 10x scale/yaw. */
            addChild(s->parent?s->parent:rootNode(s->scene,0),s->node);
            setPosition(s->node,&s->position);setQueue(s->particles,84);
            originalAttach(s->node,s->particles);keepLocal(s->particles,1);
            startParticles(s->particles);autoBounds(s->particles,0,0);
            /* Hide only the native mesh. Its node, hit logic and our separate particles stay. */
            if(s->object) {
                if(!windMaterialLogged) {logMaterialName("WIND MESH material hidden: ",s->object);windMaterialLogged=1;}
                setVisible(s->object,0);
            }
        }
        setPosition(s->node,&s->position);
        if(!s->source && !s->ending) {
            stopFade(s->particles);s->ending=1;
            if(!stormStopLogged) {stormStopLogged=1;logLine("WIND STOP: native projection removed; particles fade for at most 1.6s.\r\n");}
        }
        updateParticles(s->particles,dt);setAabb(s->particles,bounds);
        if(!stormEmitLogged && particleCount(s->particles)>0) {
            stormEmitLogged=1;logLine("WIND EMIT: fire/dust/ember particles are alive.\r\n");
        }
    }
}
