/* Follow the accepted fireball DLL's AEV_Embers node, never projectile internals. */
#define MISSILE_FLAMES 24
typedef struct {
    void *source,*sourceNode,*scene,*parent,*node,*particles;
    Vec3 position,previous;Quat rotation;U64 born,stopped;
    int used,heading,ending;
} MissileFlame;
static MissileFlame missileFlames[MISSILE_FLAMES];
static OldString missileTemplate;
static int missileEnabled=1,missileCaptureLogged,missileEmitLogged,missileRenderLogged,missileFailureLogged;
static const OldString* (*puTemplateName)(void*);
static void* (*puParentNode)(void*);
static float (*normaliseVector)(Vec3*);
static float (*normaliseQuaternion)(Quat*);
static void (*originalPuStart)(void*);
static void (*originalPuFade)(void*);
static void (*originalPuDestroy)(void*,void*,void*);
static int isMissileEmbers(const OldString* name) {
    const char* expected="AEV_Embers";const char* data;U64 i;
    if(!name || name->size!=10 || name->capacity<10)return 0;
    data=name->capacity<16?name->text.small:name->text.heap;if(!data)return 0;
    for(i=0;i<10;++i)if(data[i]!=expected[i])return 0;return 1;
}
static void captureMissileFlame(void* source) {
    void* node=puParentNode(source);void* scene;void* parent;Vec3 p;int i,slot=-1,duplicate=0;
    if(!node)return;scene=nodeCreator(node);parent=nodeParent(node);localPosition(node,&p);
    lock();
    __try {
        for(i=0;i<MISSILE_FLAMES;++i) {
            if(missileFlames[i].used && missileFlames[i].source==source)duplicate=1;
            if(!missileFlames[i].used && slot<0)slot=i;
        }
        if(!duplicate && slot>=0) {
            MissileFlame* f=&missileFlames[slot];f->used=1;f->source=source;f->sourceNode=node;
            f->scene=scene;f->parent=parent;f->position=p;f->previous=p;f->heading=0;f->ending=0;
            f->born=GetTickCount64();f->stopped=0;f->rotation.w=1;f->rotation.x=f->rotation.y=f->rotation.z=0;
            if(!missileCaptureLogged) {missileCaptureLogged=1;logLine("MISSILE CAPTURE: accepted AEV_Embers flight node; horizontal flame queued.\r\n");}
        }
    } __finally {unlock();}
}
static void stopMissileFlame(void* source,void* node) {
    int i;for(i=0;i<MISSILE_FLAMES;++i)if(missileFlames[i].used && missileFlames[i].source &&
        ((source && missileFlames[i].source==source) || (node && missileFlames[i].sourceNode==node))) {
        missileFlames[i].source=0;missileFlames[i].sourceNode=0;missileFlames[i].stopped=GetTickCount64();
    }
}
static void puStartHook(void* particles) {
    originalPuStart(particles);
    if(!active || !missileEnabled || internalThread==GetCurrentThreadId())return;
    __try {if(isMissileEmbers(puTemplateName(particles)))captureMissileFlame(particles);}
    __except(1) {logLine("MISSILE SKIP: source particle metadata unavailable.\r\n");}
}
static void puFadeHook(void* particles) {
    if(internalThread!=GetCurrentThreadId()) {lock();stopMissileFlame(particles,0);unlock();}
    originalPuFade(particles);
}
static void puDestroyHook(void* manager,void* particles,void* scene) {
    /* The accepted DLL destroys PU first, then bypasses the outer node hook. */
    if(internalThread!=GetCurrentThreadId()) {lock();stopMissileFlame(particles,0);unlock();}
    originalPuDestroy(manager,particles,scene);
}
static void eraseMissileFlame(MissileFlame* f) {
    if(f->particles)destroyParticles(particleManager(),f->particles,f->scene);
    if(f->node)destroyNode(f->scene,f->node);
    f->particles=0;f->node=0;f->source=0;f->sourceNode=0;f->scene=0;f->parent=0;f->used=0;
}
static void clearMissileFlames(void* scene) {
    int i;for(i=0;i<MISSILE_FLAMES;++i)if(missileFlames[i].used && missileFlames[i].scene==scene)eraseMissileFlame(&missileFlames[i]);
}
static void missileFlameRender(void* particles) {
    int i;if(!missileRenderLogged)for(i=0;i<MISSILE_FLAMES;++i)if(missileFlames[i].used && missileFlames[i].particles==particles) {
        missileRenderLogged=1;logLine("MISSILE RENDER: enlarged forward-aligned flame reached native queue.\r\n");break;
    }
}
static int missileHeading(Vec3 previous,Vec3 current,Quat* result) {
    Vec3 back={previous.x-current.x,previous.y-current.y,previous.z-current.z};float moved=normaliseVector(&back);
    if(moved<.01f || moved>2000)return 0;
    if(back.y<-.9999f) {result->w=0;result->x=1;result->y=0;result->z=0;}
    else {result->w=1+back.y;result->x=back.z;result->y=0;result->z=-back.x;normaliseQuaternion(result);}
    return 1;
}
static void updateMissileFlames(U64 now,float dt) {
    int i;const float bounds[6]={0,5,0,24,24,24};
    for(i=0;i<MISSILE_FLAMES;++i)if(missileFlames[i].used) {
        MissileFlame* f=&missileFlames[i];
        if(!missileEnabled) {eraseMissileFlame(f);continue;}
        if(f->source && now-f->born>12000)stopMissileFlame(f->source,0);
        if(!f->source && (!f->particles || now-f->stopped>=700)) {eraseMissileFlame(f);continue;}
        if(f->sourceNode) {
            localPosition(f->sourceNode,&f->position);
            if(missileHeading(f->previous,f->position,&f->rotation))f->heading=1;
            f->previous=f->position;
        }
        if(!f->heading)continue;
        if(!f->particles) {
            OldString unique={{"AEV_M00000000"},13,15};unsigned int j,n=++serial;void* manager=particleManager();
            if(!manager || !particleTemplate(manager,&missileTemplate)) {eraseMissileFlame(f);continue;}
            for(j=0;j<8;++j)unique.text.small[5+j]="0123456789abcdef"[(n>>(28-j*4))&15];
            f->particles=createParticles(manager,&unique,&missileTemplate,f->scene);f->node=createNode(f->scene,0);
            if(!f->particles || !f->node) {eraseMissileFlame(f);continue;}
            addChild(f->parent?f->parent:rootNode(f->scene,0),f->node);
            setPosition(f->node,&f->position);setRockOrientation(f->node,f->rotation.w,f->rotation.x,f->rotation.y,f->rotation.z);
            setQueue(f->particles,84);originalAttach(f->node,f->particles);keepLocal(f->particles,1);
            startParticles(f->particles);autoBounds(f->particles,0,0);
        }
        setPosition(f->node,&f->position);setRockOrientation(f->node,f->rotation.w,f->rotation.x,f->rotation.y,f->rotation.z);
        if(!f->source && !f->ending) {stopFade(f->particles);f->ending=1;}
        updateParticles(f->particles,dt);setAabb(f->particles,bounds);
        if(!missileEmitLogged && particleCount(f->particles)>0) {missileEmitLogged=1;logLine("MISSILE EMIT: enlarged flame shell and short horizontal tail are alive.\r\n");}
    }
}
static void updateMissileFlamesSafe(U64 now,float dt) {
    __try {updateMissileFlames(now,dt);}
    __except(1) {
        missileEnabled=0;
        if(!missileFailureLogged) {missileFailureLogged=1;logLine("MISSILE DISABLED: extra flame failed; accepted fireball/embers remain unchanged.\r\n");}
    }
}
