static OldString missileTestTemplate;
static Vec3 missileTestPosition;
static Quat missileTestRotation;
static int missilePositionReads,missileNativeDestroys;
static float missileTestSqrt(float x) {float v=x>1?x:1;int i;for(i=0;i<12;++i)v=(v+x/v)*.5f;return v;}
static float missileTestNormalise(Vec3* v) {
    float n=missileTestSqrt(v->x*v->x+v->y*v->y+v->z*v->z);
    if(n>.00001f) {v->x/=n;v->y/=n;v->z/=n;}return n;
}
static float missileTestNormaliseQ(Quat* q) {
    float n=missileTestSqrt(q->w*q->w+q->x*q->x+q->y*q->y+q->z*q->z);
    q->w/=n;q->x/=n;q->y/=n;q->z/=n;return n;
}
static const OldString* missileTestName(void* p) {(void)p;return &missileTestTemplate;}
static void* missileTestParent(void* p) {(void)p;return (void*)0x2222;}
static Vec3* missileTestGetPosition(void* node,Vec3* p) {(void)node;++missilePositionReads;*p=missileTestPosition;return p;}
static void missileTestSetRotation(void* node,float w,float x,float y,float z) {
    (void)node;missileTestRotation.w=w;missileTestRotation.x=x;missileTestRotation.y=y;missileTestRotation.z=z;
}
static void missileTestDestroy(void* manager,void* particles,void* scene) {(void)manager;(void)particles;(void)scene;++missileNativeDestroys;}
__declspec(dllexport) int probeNativeMissileFlame(void* scene,void* sourceNode,void* source) {
    MODULE pu=GetModuleHandleA("Plugin_ParticleUniverse_x64.dll"),ogre=GetModuleHandleA("OgreMain_x64.dll");
    int ok=1,i;U64 tick;Vec3 position={0,0,0},direction,up={0,1,0};Quat rotation;
    API(originalPuStart,pu,"?start@ParticleSystem@ParticleUniverse@@QEAAXXZ");
    API(originalPuFade,pu,"?stopFade@ParticleSystem@ParticleUniverse@@QEAAXXZ");
    API(originalPuDestroy,pu,"?destroyParticleSystem@ParticleSystemManager@ParticleUniverse@@QEAAXPEAVParticleSystem@2@PEAVSceneManager@Ogre@@@Z");
    API(puTemplateName,pu,"?getTemplateName@ParticleSystem@ParticleUniverse@@QEBAAEBV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@XZ");
    API(puParentNode,ogre,"?getParentSceneNode@MovableObject@Ogre@@QEBAPEAVSceneNode@2@XZ");
    API(nodeCreator,ogre,"?getCreator@SceneNode@Ogre@@QEBAPEAVSceneManager@2@XZ");
    API(nodeParent,ogre,"?getParent@Node@Ogre@@QEBAPEAV12@XZ");
    API(rootNode,ogre,"?getRootSceneNode@SceneManager@Ogre@@QEAAPEAVSceneNode@2@W4SceneMemoryMgrTypes@2@@Z");
    API(normaliseVector,ogre,"?normalise@Vector3@Ogre@@QEAAMXZ");
    API(normaliseQuaternion,ogre,"?normalise@Quaternion@Ogre@@QEAAMXZ");
    API(particleManager,pu,"?getSingletonPtr@ParticleSystemManager@ParticleUniverse@@SAPEAV12@XZ");
    API(particleTemplate,pu,"?getParticleSystemTemplate@ParticleSystemManager@ParticleUniverse@@QEAAPEAVParticleSystem@2@AEBV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@@Z");
    API(createParticles,pu,"?createParticleSystem@ParticleSystemManager@ParticleUniverse@@QEAAPEAVParticleSystem@2@AEBV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@0PEAVSceneManager@Ogre@@@Z");
    API(updateParticles,pu,"?_update@ParticleSystem@ParticleUniverse@@QEAAXM@Z");
    API(particleCount,pu,"?getNumberOfEmittedParticles@ParticleSystem@ParticleUniverse@@QEAA_KXZ");
    API(keepLocal,pu,"?setKeepLocal@ParticleSystem@ParticleUniverse@@QEAAX_N@Z");
    API(autoBounds,pu,"?setBoundsAutoUpdated@ParticleSystem@ParticleUniverse@@QEAAX_NM@Z");
    API(setAabb,ogre,"?setLocalAabb@MovableObject@Ogre@@QEAAXUAabb@2@@Z");
    if(!ok)return 1;
    startParticles=originalPuStart;stopFade=originalPuFade;destroyParticles=originalPuDestroy;
    active=1;missileEnabled=1;stringRef(&missileTemplate,"AEV_MissileFlame");
    puStartHook(source);if(!missileFlames[0].used)return 2;
    tick=GetTickCount64();
    for(i=1;i<=50;++i) {position.x=i*.75f;setPosition(sourceNode,&position);updateMissileFlames(tick+i*16,.016f);}
    if(!missileFlames[0].particles || particleCount(missileFlames[0].particles)==0)return 3;
    localOrientation(missileFlames[0].node,&rotation);direction=rotateVector(rotation,up);
    if(direction.x>-.999f || direction.y>.001f || direction.y<-.001f)return 4;
    puDestroyHook(particleManager(),source,scene);destroyNode(scene,sourceNode);
    tick=GetTickCount64();
    for(i=1;i<=50;++i)updateMissileFlames(tick+i*16,.016f);
    if(missileFlames[0].used)return 5;
    return 0;
}
__declspec(dllexport) int runMissileFlameTests(void) {
    int i,initialCreates=creations,reads;U64 now;Vec3 zero={0,0,0},up={0,1,0},direction;Quat q;
    normaliseVector=missileTestNormalise;normaliseQuaternion=missileTestNormaliseQ;
    missileTestPosition.x=10;missileTestPosition.y=20;missileTestPosition.z=30;
    puTemplateName=missileTestName;puParentNode=missileTestParent;localPosition=missileTestGetPosition;
    originalPuStart=testStart;originalPuFade=testFade;originalPuDestroy=missileTestDestroy;
    setRockOrientation=missileTestSetRotation;setPosition=testRockPosition;
    stringRef(&missileTemplate,"AEV_MissileFlame");stringRef(&missileTestTemplate,"AEV_Holy_FireBurst");
    puStartHook((void*)0x1111);if(missileFlames[0].used)return 50;
    stringRef(&missileTestTemplate,"AEV_Embers");puStartHook((void*)0x1111);puStartHook((void*)0x1111);
    if(!missileFlames[0].used || missileFlames[1].used)return 51;
    now=GetTickCount64();updateMissileFlames(now,.016f);
    if(creations!=initialCreates || missileFlames[0].heading)return 52;
    missileTestPosition.x=20;updateMissileFlames(now+16,.016f);
    direction=rotateVector(missileTestRotation,up);
    if(creations!=initialCreates+1 || direction.x>-.999f || direction.y>.001f || direction.y<-.001f)return 53;
    missileFlameRender((void*)0x600);if(!missileRenderLogged)return 54;
    reads=missilePositionReads;puDestroyHook((void*)0x500,(void*)0x1111,(void*)0x400);
    updateMissileFlames(GetTickCount64()+16,.016f);
    if(missilePositionReads!=reads || !missileFlames[0].ending || missileNativeDestroys!=1)return 55;
    updateMissileFlames(GetTickCount64()+701,.016f);if(missileFlames[0].used)return 56;
    direction.x=0;direction.y=10;direction.z=0;
    if(!missileHeading(zero,direction,&q))return 57;direction=rotateVector(q,up);
    if(direction.y>-.999f)return 58;
    for(i=0;i<MISSILE_FLAMES+4;++i)puStartHook((void*)(U64)(0x3000+i));
    for(i=0;i<MISSILE_FLAMES;++i)if(!missileFlames[i].used)return 59;
    clearMissileFlames((void*)0x400);
    for(i=0;i<MISSILE_FLAMES;++i)if(missileFlames[i].used)return 60;
    puStartHook((void*)0x1111);puFadeHook((void*)0x1111);updateMissileFlames(GetTickCount64(),.016f);
    if(missileFlames[0].used)return 61;
    return 0;
}
