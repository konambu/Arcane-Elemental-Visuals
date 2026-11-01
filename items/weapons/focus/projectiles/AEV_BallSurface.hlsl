// Deferred fire surface. Keep the game's Object_VP for all geometry transforms.
struct FireBuffers {
    float4 albedo : SV_Target0;
    float4 normal : SV_Target1;
    float4 depth : SV_Target2;
};
Texture2D flameMap : register(t0);
SamplerState flameSampler : register(s0);
FireBuffers fire_surface(float4 screen:SV_Position,float2 uv:TEXCOORD0,
    float3 worldNormal:TEXCOORD1,float3 worldPosition:TEXCOORD5,
    uniform float fireTime,uniform float3 cameraPos,uniform float farClip)
{
    FireBuffers output;
    float2 flow=uv+float2(-fireTime*0.32,fireTime*0.14);
    float3 rgb=flameMap.Sample(flameSampler,flow).rgb;
    rgb*=0.92+0.08*sin(fireTime*8+uv.y*12);
    // Match Kenshi's deferred YCoCg albedo packing and checkerboard chroma.
    float Y=dot(rgb,float3(0.25,0.5,0.25));
    float Co=0.5*rgb.r-0.5*rgb.b+0.5;
    float Cg=-0.25*rgb.r+0.5*rgb.g-0.25*rgb.b+0.5;
    output.albedo=float4(Y,fmod(screen.x,2)==fmod(screen.y,2)?Cg:Co,0,0);
    output.normal=float4(normalize(worldNormal)*0.5+0.5,0.925);
    float depth=length(worldPosition-cameraPos)/max(farClip,0.001);
    output.depth=depth.xxxx;
    return output;
}
