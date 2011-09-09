function Cloth(width, height, segments) {
	
	this.width = width;
	this.height = height;
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
	this.drawBegin = crMeshDrawBegin;
	this.draw = crMeshDraw;
	this.drawEnd = crMeshDrawEnd;
}