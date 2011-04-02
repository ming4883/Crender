-- Bg.Vertex
struct vs_in {
	float4 vertex : POSITION;
	float2 texcoord0 : TEXCOORD0;
};

struct vs_out {
	float4 vertex : POSITION;
	float2 texcoord : TEXCOORD0;
};

vs_out main(vs_in i) {
	vs_out o;

	o.vertex = i.vertex;
	o.texcoord = i.texcoord0;

	return o;
}

-- Bg.Fragment
struct ps_in {
	float2 texcoord : TEXCOORD0;
};

struct ps_out {
	float4 fragColor : COLOR0;
};

uniform float4 u_colors[4];

ps_out main(ps_in i) {
	ps_out o;

	float4 c0 = lerp(u_colors[0], u_colors[1], i.texcoord.x);
	float4 c1 = lerp(u_colors[2], u_colors[3], i.texcoord.x);
	o.fragColor = lerp(c0, c1, i.texcoord.y);

	return o;
}

-- Ui.Vertex
struct vs_in {
	float4 vertex : POSITION;
	float2 texcoord0 : TEXCOORD0;
};

struct vs_out {
	float4 vertex : POSITION;
	float2 texcoord : TEXCOORD0;
};

vs_out main(vs_in i) {
	vs_out o;

	o.vertex = i.vertex;
	o.texcoord = i.texcoord0;

	return o;
}

-- Ui.Fragment
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

-- Text.Fragment
struct ps_in {
	float2 texcoord : TEXCOORD0;
};

struct ps_out {
	float4 fragColor : COLOR0;
};

uniform sampler2D u_tex;
uniform float4 u_textColor;

ps_out main(ps_in i) {
	ps_out o;

	o.fragColor = float4(u_textColor.xyz, u_textColor.w * tex2D(u_tex, i.texcoord).x);
	return o;
}