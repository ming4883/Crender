-- Create.Vertex
struct vs_in {
	float4 vertex : POSITION;
	float3 normal : NORMAL;
	float2 texcoord0 : TEXCOORD0;
};

struct vs_out {
	float4 vertex : POSITION;
	float2 texcoord : TEXCOORD0;
	float4 clipPos : TEXCOORD1;
};

uniform float4x4 u_worldViewProjMtx;

vs_out main(vs_in i) {
	vs_out o;

	o.vertex = mul(u_worldViewProjMtx, i.vertex);
	o.texcoord = i.texcoord0;
	o.clipPos = o.vertex;

	return o;
}

-- Create.Fragment
struct ps_in {
	float2 texcoord : TEXCOORD0;
	float4 clipPos : TEXCOORD1;
};

struct ps_out {
	float4 fragColor : COLOR0;
};

uniform float4 u_shadowMapParam;

ps_out main(ps_in i) {
	ps_out o;

	float slopScale = u_shadowMapParam[2];

	float depth = i.clipPos.z / i.clipPos.w;
	float dz = max( abs(ddx(depth)), abs(ddy(depth)) ) * slopScale;
	o.fragColor = float4(depth + dz, 0, 0, 0);

	return o;
}
