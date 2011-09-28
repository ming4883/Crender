var Water_squad;

function Water_init() {

	Water_squad = Water_squad || crCreateScreenQuad();
	var self = this;
	var p = this.prog.init;
	
	crRenderToTexture(this.posBuf[0], function() {
	
		gl.disable(gl.CULL_FACE);
		gl.disable(gl.DEPTH_TEST);
		
		gl.useProgram(p);
		
		Water_squad.drawBegin(p);
		Water_squad.draw();
		Water_squad.drawEnd();
	});
	
	crRenderToTexture(this.posBuf[1], function() {
	
		gl.disable(gl.CULL_FACE);
		gl.disable(gl.DEPTH_TEST);
		
		gl.useProgram(p);
		
		Water_squad.drawBegin(p);
		Water_squad.draw();
		Water_squad.drawEnd();
	});
}

function Water_step() {

	Water_squad = Water_squad || crCreateScreenQuad();
	var self = this;
	var currIdx = this.currIdx;
	var lastIdx = (this.currIdx == 0) ? 1 : 0;
	
	{
		var p = self.prog.step;
		crRenderToTexture(self.posBuf[currIdx], function() {
		
			gl.disable(gl.CULL_FACE);
			gl.disable(gl.DEPTH_TEST);
			
			gl.useProgram(p);
			
			// u_buffer
			gl.uniform1i(p.u_buffer, 0);
			gl.activeTexture(gl.TEXTURE0);
			gl.bindTexture(gl.TEXTURE_2D, self.posBuf[lastIdx]);
			
			// u_delta
			gl.uniform2fv(p.u_delta, self.delta);
			
			Water_squad.drawBegin(p);
			Water_squad.draw();
			Water_squad.drawEnd();
		});
	}
	this.currIdx = lastIdx;
}

function Water_updateNormals() {

	Water_squad = Water_squad || crCreateScreenQuad();
	var self = this;
	var currIdx = this.currIdx;
	var lastIdx = (this.currIdx == 0) ? 1 : 0;
	
	{
		var p = self.prog.normal;
		crRenderToTexture(self.posBuf[lastIdx], function() {
		
			gl.disable(gl.CULL_FACE);
			gl.disable(gl.DEPTH_TEST);
			
			gl.useProgram(p);
			
			// u_buffer
			gl.uniform1i(p.u_buffer, 0);
			gl.activeTexture(gl.TEXTURE0);
			gl.bindTexture(gl.TEXTURE_2D, self.posBuf[currIdx]);
			
			// u_delta
			gl.uniform2fv(p.u_delta, self.delta);
			
			Water_squad.drawBegin(p);
			Water_squad.draw();
			Water_squad.drawEnd();
		});
	}
	this.currIdx = lastIdx;
}

function Water_addDrop(x, y, radius, strength) {

	Water_squad = Water_squad || crCreateScreenQuad();
	var self = this;
	var currIdx = this.currIdx;
	var lastIdx = (this.currIdx == 0) ? 1 : 0;
	
	{
		var p = self.prog.adddrop;
		crRenderToTexture(self.posBuf[lastIdx], function() {
		
			gl.disable(gl.CULL_FACE);
			gl.disable(gl.DEPTH_TEST);
			
			gl.useProgram(p);
			
			// u_buffer
			gl.uniform1i(p.u_buffer, 0);
			gl.activeTexture(gl.TEXTURE0);
			gl.bindTexture(gl.TEXTURE_2D, self.posBuf[currIdx]);
			
			// u_center
			gl.uniform2f(p.u_center, x, y);
			
			// u_radius
			gl.uniform1f(p.u_radius, radius);
			
			// u_strength
			gl.uniform1f(p.u_strength, strength);
			
			Water_squad.drawBegin(p);
			Water_squad.draw();
			Water_squad.drawEnd();
		});
	}
	this.currIdx = lastIdx;
}

function Water_draw() {

	var p = this.prog.draw;
	var currIdx = this.currIdx;

	gl.disable(gl.CULL_FACE);
	gl.disable(gl.DEPTH_TEST);
	
	gl.useProgram(p);
	
	// u_buffer
	gl.uniform1i(p.u_buffer, 0);
	gl.activeTexture(gl.TEXTURE0);
	gl.bindTexture(gl.TEXTURE_2D, this.posBuf[currIdx]);
	
	Water_squad.drawBegin(p);
	Water_squad.draw();
	Water_squad.drawEnd();
	
}

function Water(width, height, segments) {
	
	this.size = new Float32Array([width, height]);
	this.delta = new Float32Array([1.0 / (segments+1.0), 1.0 / (segments+1.0)]);
	this.segments = segments;
	this.currIdx = 0;
	
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
		var p = crCreateProgramDOM(["water-process-vs", "water-init-fs"]);
		this.prog.init = p;
	}
	{
		var p = crCreateProgramDOM(["water-process-vs", "water-step-fs"]);
		p.u_buffer = gl.getUniformLocation(p, "u_buffer");
		p.u_delta = gl.getUniformLocation(p, "u_delta");
		this.prog.step = p;
	}
	{
		var p = crCreateProgramDOM(["water-process-vs", "water-normal-fs"]);
		p.u_buffer = gl.getUniformLocation(p, "u_buffer");
		p.u_delta = gl.getUniformLocation(p, "u_delta");
		this.prog.normal = p;
	}
	{
		var p = crCreateProgramDOM(["water-process-vs", "water-adddrop-fs"]);
		p.u_buffer = gl.getUniformLocation(p, "u_buffer");
		p.u_center = gl.getUniformLocation(p, "u_center");
		p.u_radius = gl.getUniformLocation(p, "u_radius");
		p.u_strength = gl.getUniformLocation(p, "u_strength");
		this.prog.adddrop = p;
	}
	{
		var p = crCreateProgramDOM(["water-process-vs", "water-draw-fs"]);
		p.u_buffer = gl.getUniformLocation(p, "u_buffer");
		this.prog.draw = p;
	}
	
	this.init = Water_init;
	this.step = Water_step;
	this.addDrop = Water_addDrop;
	this.updateNormals = Water_updateNormals;
	this.draw = Water_draw;
}