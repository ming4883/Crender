var Cloth_squad;

function Cloth_init() {

	Cloth_squad = Cloth_squad || crCreateScreenQuad();
	var p = this.prog.init;
	var sz = this.size;

	crRenderToTexture(this.posBuf[0], function() {
	
		gl.disable(gl.CULL_FACE);
		gl.disable(gl.DEPTH_TEST);
		
		gl.useProgram(p);
		gl.uniform2fv(p.u_clothSize, sz);
		
		Cloth_squad.drawBegin(p);
		Cloth_squad.draw();
		Cloth_squad.drawEnd();
	});
}

function Cloth_draw(prog) {

	gl.uniform1i(prog.u_posBuf, 0);
	gl.activeTexture(gl.TEXTURE0);
	gl.bindTexture(gl.TEXTURE_2D, this.posBuf[0]);
	
	this._drawBegin(prog);
	this._draw();
	this._drawEnd();
	
}

function Cloth(width, height, segments) {
	
	this.size = new Float32Array([width, height]);
	this.segments = segments;
	
	var stride = segments + 1;
	var segments_rcp = 1.0 / segments;
	
	var indices = new Array(segments * segments * 6);
	var vertices = new Array(stride * stride * 2);
	
	var idx = 0;
	
	// indices
	idx = 0;
	for(r=0; r<segments; ++r) {
		for(c=0; c<segments; ++c) {
			var p0 = (r * stride + (c+1));
			var p1 = ((r+1) * stride + (c+1));
			var p2 = (r * stride + c);
			var p3 = ((r+1) * stride + c);

			indices[idx++] = p0;
			indices[idx++] = p1;
			indices[idx++] = p2;

			indices[idx++] = p3;
			indices[idx++] = p2;
			indices[idx++] = p1;
		}
	}
	
	// vertices
	idx = 0;
	for(r=0; r<stride; ++r) {
		for(c=0; c<stride; ++c) {
			vertices[idx++] = c * segments_rcp;
			vertices[idx++] = r * segments_rcp;
		}
	}
	
	this.vb = crCreateVertexBuffer(new Float32Array(vertices), gl.STATIC_DRAW);
	this.ib = crCreateIndexBuffer(new Uint16Array(indices), gl.STATIC_DRAW);
	this.indexCount = indices.length;
	this.vbStride = 8;
	this.attributes = [{name : "i_vertex", count : 2, byteOffset : 0}];
	this._drawBegin = crMeshDrawBegin;
	this._draw = crMeshDraw;
	this._drawEnd = crMeshDrawEnd;
	
	this.posBuf = [
		crCreateFBOTexture2D(stride, stride, {type:gl.FLOAT, mag_filter:gl.NEAREST, min_filter:gl.NEAREST}),
		crCreateFBOTexture2D(stride, stride, {type:gl.FLOAT, mag_filter:gl.NEAREST, min_filter:gl.NEAREST})
	];
	
	this.prog = {};
	{
		var p = crCreateProgramDOM(["cloth-process-vs", "cloth-init-fs"]);
		p.u_clothSize = gl.getUniformLocation(p, "u_clothSize");
		this.prog.init = p;
	}
	
	this.init = Cloth_init;
	this.draw = Cloth_draw;
}