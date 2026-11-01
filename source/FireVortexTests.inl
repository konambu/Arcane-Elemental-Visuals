static int fades,detaches,allDetaches,nodeDestructions,stormPositionReads;
static Vec3* testStormPosition(void* n,Vec3* p) {
    if(n==(void*)0xdead) {stormPositionReads=-10000;return 0;}
    ++stormPositionReads;p->x=100;p->y=7;p->z=-30;return p;
}
static void testFade(void* p) {(void)p;++fades;}
static void testDetach(void* n,void* o) {(void)n;(void)o;++detaches;}
static void testDetachAll(void* n) {(void)n;++allDetaches;}
static void testDestroySource(void* scene,void* n) {(void)scene;(void)n;++nodeDestructions;}
static unsigned char testSlopeGround(Vec3* p) {p->y=5+p->x*.1f;return 1;}
static unsigned char testMissingGround(Vec3* p) {(void)p;return 0;}
static int rockCreates,rockDeletes;static Vec3 rockLastPosition,rockLastScale;
static void* testRockCreate(void* scene,const OldString* mesh,const OldString* group,int memory) {
    (void)scene;(void)group;(void)memory;if(mesh->size!=21)return 0;
    ++rockCreates;return (void*)(U64)(0xa000+rockCreates);
}
static void testRockDelete(void* scene,void* entity) {(void)scene;(void)entity;++rockDeletes;}
static void testRockPosition(void* node,const Vec3* v) {(void)node;rockLastPosition=*v;}
static void testRockScale(void* node,const Vec3* v) {(void)node;rockLastScale=*v;}
static void testRockOrientation(void* n,float w,float x,float y,float z) {(void)n;(void)w;(void)x;(void)y;(void)z;}
static void testRockShadows(void* entity,unsigned char shadows) {(void)entity;(void)shadows;}
static void* testRockFault(void* scene,const OldString* mesh,const OldString* group,int memory) {
    volatile int* invalid=(volatile int*)0;(void)scene;(void)mesh;(void)group;(void)memory;
    return (void*)(U64)*invalid;
}
__declspec(dllexport) int runRockTests(void) {
    int i;Vec3 position={0,-20,0},scale={1,2,1},ground={0,0,0},p,s;Quat q={1,0,0,0};
    rockPose(position,scale,q,ground,&p,&s);
    if(p.y!=-30 || s.y!=3 || p.y+20*s.y!=30 || position.y!=-20 || scale.y!=2)return 40;
    {Vec3 up={0,1,0},tip;float exposed;Quat tilted={.8660254f,0,0,.5f};
        up=rotateVector(tilted,up);ground.x=position.x+20*up.x;ground.y=position.y+20*up.y;
        rockPose(position,scale,tilted,ground,&p,&s);
        tip.x=p.x+20*s.y*up.x-ground.x;tip.y=p.y+20*s.y*up.y-ground.y;
        exposed=tip.x*up.x+tip.y*up.y;if(exposed<29.999f || exposed>30.001f)return 46;
    }
    createRockEntity=testRockCreate;destroyRockEntity=testRockDelete;setRockScale=testRockScale;
    setRockOrientation=testRockOrientation;rockShadows=testRockShadows;
    localPosition=testPosition;localScale=testScale;localOrientation=testOrientation;setPosition=testRockPosition;
    groundQuery=testGround;stringRef(&rockMesh,"AEV_EarthRock_v3.mesh");
    captureRock((void*)0x1000,(void*)0x2000,(void*)0x400,(void*)0x800);
    captureRock((void*)0x1000,(void*)0x2000,(void*)0x400,(void*)0x800);
    update();
    if(rockCreates!=1 || rockLastScale.y!=3 || rockLastPosition.y!=8 || !rocks[0].grounded)return 41;
    stopRocks((void*)0x1000,0);update();if(rockDeletes!=1 || rocks[0].used)return 42;
    for(i=0;i<ROCKS+3;++i)captureRock((void*)(U64)(0x3000+i),(void*)(U64)(0x4000+i),(void*)0x400,(void*)0x800);
    update();if(rockCreates!=1+ROCKS)return 43;
    clearHook((void*)0x400);if(rockDeletes!=1+ROCKS)return 44;
    for(i=0;i<ROCKS;++i)if(rocks[i].used)return 45;
    captureRock((void*)0x1000,(void*)0x2000,(void*)0x400,(void*)0x800);
    createRockEntity=testRockFault;update();
    if(rockEnabled || !active)return 47;
    update();if(rocks[0].used || !active)return 48;
    return 0;
}
static unsigned char testFlatGround(Vec3* p) {p->y=0;return 1;}
__declspec(dllexport) int probeNativeRockVisual(void* scene,void* source,void* object) {
    MODULE ogre=GetModuleHandleA("OgreMain_x64.dll");int ok=1;Vec3 p,s;void* owned;
    API(localPosition,ogre,"?getPosition@Node@Ogre@@QEBA?AVVector3@2@XZ");
    API(localScale,ogre,"?getScale@Node@Ogre@@QEBA?AVVector3@2@XZ");
    API(localOrientation,ogre,"?getOrientation@Node@Ogre@@QEBA?AVQuaternion@2@XZ");
    API(createRockEntity,ogre,"?createEntity@SceneManager@Ogre@@UEAAPEAVEntity@2@AEBV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@0W4SceneMemoryMgrTypes@2@@Z");
    API(destroyRockEntity,ogre,"?destroyEntity@SceneManager@Ogre@@UEAAXPEAVEntity@2@@Z");
    API(entityMaterial,ogre,"?setMaterialName@Entity@Ogre@@QEAAXAEBV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@0@Z");
    API(setVisible,ogre,"?setVisible@MovableObject@Ogre@@QEAAX_N@Z");
    API(setQueue,ogre,"?setRenderQueueGroup@MovableObject@Ogre@@UEAAXE@Z");
    API(rockShadows,ogre,"?setCastShadows@MovableObject@Ogre@@QEAAX_N@Z");
    API(nodeParent,ogre,"?getParent@Node@Ogre@@QEBAPEAV12@XZ");
    API(createNode,ogre,"?createSceneNode@SceneManager@Ogre@@UEAAPEAVSceneNode@2@W4SceneMemoryMgrTypes@2@@Z");
    API(addChild,ogre,"?addChild@Node@Ogre@@QEAAXPEAV12@@Z");
    API(setPosition,ogre,"?setPosition@Node@Ogre@@QEAAXAEBVVector3@2@@Z");
    API(setRockScale,ogre,"?setScale@Node@Ogre@@QEAAXAEBVVector3@2@@Z");
    API(setRockOrientation,ogre,"?setOrientation@Node@Ogre@@QEAAXMMMM@Z");
    API(destroyNode,ogre,"?destroySceneNode@SceneManager@Ogre@@UEAAXPEAVSceneNode@2@@Z");
    API(originalAttach,ogre,"?attachObject@SceneNode@Ogre@@QEAAXPEAVMovableObject@2@@Z");
    API(getSubEntity,ogre,"?getSubEntity@Entity@Ogre@@QEAAPEAVSubEntity@2@_K@Z");
    API(subMaterialName,ogre,"?getMaterialName@SubEntity@Ogre@@QEBAAEBV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@XZ");
    if(!ok)return 1;
    stringRef(&rockMesh,"AEV_EarthRock_v3.mesh");stringRef(&earthMaterial,"EV_EarthSpike_Charcoal_v001");stringRef(&earthGroup,"ArcaneEarthSpikeV001");
    groundQuery=testFlatGround;rockEnabled=1;rockLogged=1;
    captureRock(source,object,scene,nodeParent(source));updateRocks(GetTickCount64());
    if(!rocks[0].entity || !rocks[0].node)return 2;
    owned=rocks[0].node;localPosition(owned,&p);localScale(owned,&s);
    if(p.y!=-30 || s.y!=3)return 3;
    localPosition(source,&p);localScale(source,&s);if(p.y!=-20 || s.y!=2)return 4;
    stopRocks(source,object);updateRocks(GetTickCount64());if(rocks[0].used)return 5;
    return 0;
}
__declspec(dllexport) int probeNativeEarthBinding(void* object,void* materialAPI,void* subAPI,void* nameAPI) {
    void (*apply)(void*,const OldString*,const OldString*);
    void* (*sub)(void*,U64);const OldString* (*name)(void*);
    OldString mat,group;const OldString* actual;const char* text;U64 i;
    *(void**)(&apply)=materialAPI;*(void**)(&sub)=subAPI;*(void**)(&name)=nameAPI;
    stringRef(&mat,"EV_EarthSpike_Charcoal_v001");stringRef(&group,"ArcaneEarthSpikeV001");
    apply(object,&mat,&group);actual=name(sub(object,0));
    if(!actual || actual->size!=mat.size || actual->capacity<16)return 0;
    text=actual->text.heap;
    for(i=0;i<mat.size;++i)if(text[i]!=mat.text.heap[i])return 0;
    return 1;
}
__declspec(dllexport) void probeNativeSpikeAxis(void* node,void* scaleAPI,void* orientationAPI,float* output) {
    Vec3* (*savedScale)(void*,Vec3*)=localScale;
    Quat* (*savedOrientation)(void*,Quat*)=localOrientation;Vec3 value;
    *(void**)(&localScale)=scaleAPI;*(void**)(&localOrientation)=orientationAPI;
    value=spikeAxis(node);output[0]=value.x;output[1]=value.y;output[2]=value.z;
    localScale=savedScale;localOrientation=savedOrientation;
}
__declspec(dllexport) int runGroundTests(void) {
    Burst b;Vec3 axis;
    groundQuery=testSlopeGround;b.position.x=0;b.position.y=-10;b.position.z=7;
    b.axis.x=20;b.axis.y=40;b.axis.z=0;
    if(!groundBurst(&b) || b.position.x<7.5f || b.position.x>8.3f || b.position.y<6 || b.position.y>6.1f || b.position.z!=7)return 31;
    /* Same buried origin, strongly inclined spike ray. */
    b.position.x=0;b.position.y=-10;b.axis.x=60;b.axis.y=20;
    if(!groundBurst(&b) || b.position.x<63 || b.position.x>66 || b.position.y<11.5f || b.position.y>12)return 32;
    groundQuery=testMissingGround;if(groundBurst(&b))return 33;
    groundQuery=testGround;b.position.x=1;b.position.y=-10;b.axis.y=-2;
    if(groundBurst(&b))return 34;
    localScale=testScale;localOrientation=testOrientation;axis=spikeAxis((void*)0x1234);
    if(axis.x<1.15f || axis.x>1.17f || axis.y!=40 || axis.z>-.29f || axis.z<-.3f)return 35;
    groundQuery=testGround;return 0;
}
__declspec(dllexport) int runStormTests(void) {
    void* objects[STORMS+2];int i,initialCreates=creations,initialDestroys=destructions;
    OldString wrong={{"AEV_fire.mesh"},13,15};
    if(stormMatches(&wrong))return 10;
    fakeName.text.heap="AM_Whirlwind_v001.mesh";fakeName.size=length(fakeName.text.heap);fakeName.capacity=31;
    if(!stormMatches(&fakeName))return 11;
    stopFade=testFade;originalDetach=testDetach;originalDetachAll=testDetachAll;originalDestroyNode=testDestroySource;
    localPosition=testStormPosition;
    for(i=0;i<STORMS+2;++i) {objects[i]=entityVtable;attachHook((void*)(U64)(0x1000+i),&objects[i]);}
    if(!storms[STORMS-1].used)return 12;
    if(windHidden)return 23;
    attachHook((void*)0x1000,&objects[0]);
    for(i=0;i<STORMS;++i)if(storms[i].source!=(void*)(U64)(0x1000+i))return 13;
    update();
    if(creations!=initialCreates+STORMS || !stormEmitLogged || windHidden!=STORMS)return 14;
    stormRender((void*)0x600);if(!stormRenderLogged)return 15;
    detachHook((void*)0x1000,&objects[0]);
    if(storms[0].source || detaches!=1)return 16;
    i=stormPositionReads;update();
    if(fades!=1 || !storms[0].ending || stormPositionReads!=i+STORMS-1)return 17;
    storms[0].stopped=GetTickCount64()-1601;update();
    if(storms[0].used || destructions!=initialDestroys+1)return 18;
    /* A stale/dead node is never read after detachAll or destroy callbacks. */
    storms[1].source=(void*)0xdead;detachAllHook((void*)0xdead);
    storms[2].source=(void*)0xdead;destroyNodeHook((void*)0x400,(void*)0xdead);
    i=stormPositionReads;update();
    if(stormPositionReads!=i+STORMS-3 || fades!=3 || allDetaches!=1 || nodeDestructions!=1)return 19;
    /* Matching-scene clear removes active and fading effects once each. */
    clearHook((void*)0x400);
    for(i=0;i<STORMS;++i)if(storms[i].used)return 20;
    if(destructions!=initialDestroys+STORMS)return 21;
    /* Creation removed before first GPU callback allocates no particles. */
    attachHook((void*)0x1234,&objects[0]);detachAllHook((void*)0x1234);update();
    if(storms[0].used || creations!=initialCreates+STORMS)return 22;
    return 0;
}
