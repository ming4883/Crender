-- Create.Vertex
in vec4 i_vertex;
in vec3 i_normal;
in vec2 i_texcoord0;

out vec2 v_texcoord;
out vec4 v_clipPos;

uniform mat4 u_worldViewProjMtx;

void main() {
	vec4 clipPos = u_worldViewProjMtx * i_vertex;
	
	gl_Position = clipPos;
	v_clipPos = clipPos;
	v_texcoord = i_texcoord0;
}

-- Create.Fragment
in vec2 v_texcoord;
in vec4 v_clipPos;

out vec4 o_fragColor;

uniform vec4 u_shadowMapParam;

void main() {

	float slopScale = u_shadowMapParam[2];
	
	float depth = v_clipPos.z / v_clipPos.w;
	float dz = max( abs(dFdx(depth)), abs(dFdy(depth)) ) * slopScale;
	o_fragColor = vec4(depth + dz, 0, 0, 0);
}
