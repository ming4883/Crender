var gl;
var crLog;

function crInit(canvas) {
	try {
		//gl = canvas.getContext("experimental-webgl");
		gl = WebGLUtils.setupWebGL(canvas);
		gl.viewport(0, 0, canvas.width, canvas.height);
	}
	catch(e) {
	}
	if (!gl) {
		alert("Could not initialise WebGL, sorry :-(");
	}
}

function crClearColorBuffer(r, g, b, a) {
	gl.clearColor(r, g, b, a);
	gl.clear(gl.COLOR_BUFFER_BIT);
}

function crClearDepthBuffer(z) {
	gl.clearDepth(z);
	gl.clear(gl.DEPTH_BUFFER_BIT);
}

function crCreateShaderDOM(id) {
	var shaderScript = document.getElementById(id);
	if (!shaderScript) {
		return null;
	}
	
	var str = "";
	var k = shaderScript.firstChild;
	while (k) {
		if (k.nodeType == 3)
			str += k.textContent;
		k = k.nextSibling;
	}
	
	var shader;
	if (shaderScript.type == "x-shader/x-fragment") {
		shader = gl.createShader(gl.FRAGMENT_SHADER);
	} else if (shaderScript.type == "x-shader/x-vertex") {
		shader = gl.createShader(gl.VERTEX_SHADER);
	} else {
		if(crLog) crLog("none supported shader type:" + shaderScript.type);
		return null;
	}

	gl.shaderSource(shader, str);
	gl.compileShader(shader);

	if (!gl.getShaderParameter(shader, gl.COMPILE_STATUS)) {
		if(crLog) crLog(gl.getShaderInfoLog(shader));
		return null;
	}

	return shader;
}

function crCreateProgram(shaders) {
	var shaderProgram = gl.createProgram();
	
	for (i=0; i<shaders.length; ++i) {
		var sh = shaders[i];
		gl.attachShader(shaderProgram, sh);
	}
	gl.linkProgram(shaderProgram);

	if (!gl.getProgramParameter(shaderProgram, gl.LINK_STATUS)) {
		if(crLog) crLog(gl.getProgramInfoLog(shaderProgram));
		return null;
	}
	
	return shaderProgram;
}

function crCreateVertexBuffer(data, hint) {
	var buffer = gl.createBuffer();
	gl.bindBuffer(gl.ARRAY_BUFFER, buffer);
	gl.bufferData(gl.ARRAY_BUFFER, data, hint);
	gl.bindBuffer(gl.ARRAY_BUFFER, null);
	
	return buffer;
}

function crCreateIndexBuffer(data, hint) {
	var buffer = gl.createBuffer();
	gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, buffer);
	gl.bufferData(gl.ELEMENT_ARRAY_BUFFER, data, hint);
	gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, null);
	
	return buffer;
}

function crLoadJson(url, callback) {
	var request = new XMLHttpRequest();
	request.open("GET", url);
	request.onreadystatechange = function () {
		if (request.readyState == 4) {
			try {
				var ret = JSON.parse(request.responseText);
				callback(ret);
				if(crLog) crLog("loaded json '" + url + "'");
			}
			catch(err) {
				if(crLog) crLog("failed to load json '" + url + "'");
				if(crLog) crLog(err);
			}
		}
	}
	
	if(crLog) crLog("loading json '" + url + "'");
	request.send();
}

function crCreateTexture2DFromUrl(url, flipY) {
	var tex = gl.createTexture();
	tex.ready = false;
	tex.image = new Image();
	
	tex.image.onload = function() {
	
		try {
			gl.bindTexture(gl.TEXTURE_2D, tex);
			gl.pixelStorei(gl.UNPACK_FLIP_Y_WEBGL, flipY);
			gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE, tex.image);
			gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.NEAREST);
			gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST);
			gl.bindTexture(gl.TEXTURE_2D, null);
		}
		catch(err) {
			if(crLog) crLog("failed to load texture2d '" + url + "'");
			if(crLog) crLog(err);
		}
		
		if(crLog) crLog("loaded texture2d '" + url + "'");
		tex.ready = true;
	}
	
	if(crLog) crLog("loading texture2d '" + url + "'...");
	
	tex.image.src = url;
	return tex;
}

function crCreateFBOTexture2D(width, height, options) {

	options = options || {};
	
	tex = gl.createTexture();
	tex.width = width;
	tex.height = height;
	tex.format = options.format || gl.RGBA;
	tex.type = options.type || gl.UNSIGNED_BYTE;
	
	gl.bindTexture(gl.TEXTURE_2D, tex);
	gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.NEAREST);
	gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST);
	
	gl.texImage2D(gl.TEXTURE_2D, 0, tex.format, width, height, 0, tex.format, tex.type, null);
	
	gl.bindTexture(gl.TEXTURE_2D, null);
	
	return tex;
}

var framebuffer;
var renderbuffer;

function crRenderToTexture(fboTex, callback)
{
	var v = gl.getParameter(gl.VIEWPORT);
	framebuffer = framebuffer || gl.createFramebuffer();
	renderbuffer = renderbuffer || gl.createRenderbuffer();
	
	gl.bindFramebuffer(gl.FRAMEBUFFER, framebuffer);
	gl.bindRenderbuffer(gl.RENDERBUFFER, renderbuffer);
	
	if (fboTex.width != renderbuffer.width || fboTex.height != renderbuffer.height) {
		renderbuffer.width = fboTex.width;
		renderbuffer.height = fboTex.height;
		gl.renderbufferStorage(gl.RENDERBUFFER, gl.DEPTH_COMPONENT16, fboTex.width, fboTex.height);
	}
	gl.framebufferTexture2D(gl.FRAMEBUFFER, gl.COLOR_ATTACHMENT0, gl.TEXTURE_2D, fboTex, 0);
	gl.framebufferRenderbuffer(gl.FRAMEBUFFER, gl.DEPTH_ATTACHMENT, gl.RENDERBUFFER, renderbuffer);
	gl.viewport(0, 0, fboTex.width, fboTex.height);

	callback();

	gl.bindFramebuffer(gl.FRAMEBUFFER, null);
	gl.bindRenderbuffer(gl.RENDERBUFFER, null);
	gl.viewport(v[0], v[1], v[2], v[3]);
}

