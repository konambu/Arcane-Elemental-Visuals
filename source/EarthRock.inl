/* Owned visual proxy only: source spike node/combat fields are never written. */
#define ROCKS 24
typedef struct {
    void *source,*object,*scene,*parent,*node,*entity;
    Vec3 ground;U64 born,lastGround;int used,grounded;
} RockVisual;
static RockVisual rocks[ROCKS];
static int rockEnabled=1,rockLogged,rockFailureLogged;
static OldString rockMesh;
static Vec3 rotateVector(Quat q,Vec3 v) {
    Vec3 qv={q.x,q.y,q.z},t=cross(qv,v),u=cross(qv,t);
    v.x+=2*(q.w*t.x+u.x);v.y+=2*(q.w*t.y+u.y);v.z+=2*(q.w*t.z+u.z);return v;
}
static void rockPose(Vec3 position,Vec3 scale,Quat q,Vec3 ground,Vec3* outPosition,Vec3* outScale) {
    Vec3 up={0,1,0},delta;float distance;
    up=rotateVector(q,up);delta.x=ground.x-position.x;delta.y=ground.y-position.y;delta.z=ground.z-position.z;
    distance=-.5f*(delta.x*up.x+delta.y*up.y+delta.z*up.z);
    outPosition->x=position.x+up.x*distance;outPosition->y=position.y+up.y*distance;outPosition->z=position.z+up.z*distance;
    *outScale=scale;outScale->y*=1.5f;
}
static void captureRock(void* node,void* object,void* scene,void* parent) {
    int i;if(!rockEnabled)return;
    lock();
    for(i=0;i<ROCKS;++i)if(rocks[i].used && rocks[i].source==node)break;
    if(i==ROCKS)for(i=0;i<ROCKS;++i)if(!rocks[i].used) {
        RockVisual* r=&rocks[i];r->used=1;r->source=node;r->object=object;r->scene=scene;r->parent=parent;
        r->born=GetTickCount64();r->lastGround=0;r->grounded=0;break;
    }
    unlock();
}
static void stopRocks(void* node,void* object) {
    int i;for(i=0;i<ROCKS;++i)if(rocks[i].used &&
        ((node && rocks[i].source==node) || (object && rocks[i].object==object))) {
        rocks[i].source=0;rocks[i].object=0;
    }
}
static void eraseRock(RockVisual* r) {
    if(r->entity)destroyRockEntity(r->scene,r->entity);
    if(r->node)destroyNode(r->scene,r->node);
    r->entity=0;r->node=0;r->source=0;r->object=0;r->scene=0;r->parent=0;r->used=0;
}
static void clearRocks(void* scene) {
    int i;for(i=0;i<ROCKS;++i)if(rocks[i].used && rocks[i].scene==scene)eraseRock(&rocks[i]);
}
static void updateRocks(U64 now) {
    int i;
    for(i=0;i<ROCKS;++i)if(rocks[i].used) {
        RockVisual* r=&rocks[i];Vec3 position,scale,outPosition,outScale;Quat q;
        if(!r->source) {eraseRock(r);continue;}
        localPosition(r->source,&position);localScale(r->source,&scale);localOrientation(r->source,&q);
        if(!r->grounded || (now-r->born<400 && now-r->lastGround>=100)) {
            Burst ray;ray.position=position;ray.axis=spikeAxis(r->source);r->lastGround=now;
            if(groundBurst(&ray)) {r->ground=ray.position;r->ground.y-=.25f;r->grounded=1;}
        }
        if(!r->grounded)continue;
        if(!r->entity) {
            r->entity=createRockEntity(r->scene,&rockMesh,&earthGroup,0);
            r->node=createNode(r->scene,0);
            if(!r->entity || !r->node) {eraseRock(r);continue;}
            entityMaterial(r->entity,&earthMaterial,&earthGroup);rockShadows(r->entity,0);
            addChild(r->parent?r->parent:rootNode(r->scene,0),r->node);
            setQueue(r->entity,50);originalAttach(r->node,r->entity);
            setVisible(r->object,0);
            if(!rockLogged) {rockLogged=1;logLine("ROCK v3: unique branched visual created; source mesh hidden, source node/combat unchanged.\r\n");}
        }
        rockPose(position,scale,q,r->ground,&outPosition,&outScale);
        setPosition(r->node,&outPosition);setRockScale(r->node,&outScale);
        setRockOrientation(r->node,q.w,q.x,q.y,q.z);
    }
}
static void updateRocksSafe(U64 now) {
    int i;
    __try {
        if(rockEnabled)updateRocks(now);
        else for(i=0;i<ROCKS;++i)if(rocks[i].used) {
            if(rocks[i].source && rocks[i].object)setVisible(rocks[i].object,1);
            eraseRock(&rocks[i]);
        }
    } __except(1) {
        rockEnabled=0;
        if(!rockFailureLogged) {rockFailureLogged=1;logLine("ROCK DISABLED: proxy failed; restore source visuals; fire and dust updates continue.\r\n");}
    }
}
