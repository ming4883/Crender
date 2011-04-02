-- Scene.Vertex
struct vs_in {
	float4 vertex : POSITION;
	float3 normal : NORMAL;
	float2 texcoord0 : TEXCOORD0;
};

struct vs_out {
	float4 vertex : POSITION;
	float2 texcoord : TEXCOORD0;
};

uniform float4x4 u_worldViewProjMtx;

vs_out main(vs_in i) {
	vs_out o;

	o.vertex = mul(u_worldViewProjMtx, i.vertex);
	o.texcoord = i.texcoord0;

	return o;
}

-- Scene.Fragment
struct ps_in {
	float2 texcoord : TEXCOORD0;
};

struct ps_out {
	float4 fragColor : COLOR0;
};

uniform sampler2D u_tex;

ps_out main(ps_in i) {
	ps_out o;

	o.fragColor = tex2D(u_tex, i.texcoord);

	return o;
}
