let gl;
let program;
let startTime;
let texture;
let video;

// Vertex shader program
const vsSource = `
    attribute vec4 aVertexPosition;
    attribute vec2 aTextureCoord;
    varying vec2 vTextureCoord;
    void main() {
        gl_Position = aVertexPosition;
        vTextureCoord = aTextureCoord;
    }
`;

// Fragment shader program
const fsSource = `
    precision highp float;
    uniform float iTime;
    uniform vec2 iResolution;
    uniform sampler2D iChannel0;
    varying vec2 vTextureCoord;

    #define RAIN_SPEED 1.75
    #define DROP_SIZE  3.0

    float rand(vec2 co){
        return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
    }

    float rchar(vec2 outer, vec2 inner, float globalTime) {
        vec2 seed = floor(inner * 4.0) + outer.y;
        if (rand(vec2(outer.y, 23.0)) > 0.98) {
            seed += floor((globalTime + rand(vec2(outer.y, 49.0))) * 3.0);
        }
        return float(rand(seed) > 0.5);
    }

    void main() {
        vec2 fragCoord = gl_FragCoord.xy;
        vec2 position = vTextureCoord;
        vec2 uv = vec2(position.x, position.y);
        position.x *= iResolution.x / iResolution.y;
        float globalTime = iTime * RAIN_SPEED;
        
        float scaledown = DROP_SIZE;
        float rx = fragCoord.x / (40.0 * scaledown);
        float mx = 40.0*scaledown*fract(position.x * 30.0 * scaledown);
        vec4 result;
        
        if (mx > 12.0 * scaledown) {
            result = vec4(0.0);
        } else {
            float x = floor(rx);
            float r1x = floor(fragCoord.x / (15.0));
            
            float ry = position.y*600.0 + rand(vec2(x, x * 3.0)) * 100000.0 + globalTime* rand(vec2(r1x, 23.0)) * 120.0;
            float my = mod(ry, 15.0);
            if (my > 12.0 * scaledown) {
                result = vec4(0.0);
            } else {
                float y = floor(ry / 15.0);
                
                float b = rchar(vec2(rx, floor((ry) / 15.0)), vec2(mx, my) / 12.0, globalTime);
                float col = max(mod(-y, 24.0) - 4.0, 0.0) / 20.0;
                vec3 c = col < 0.8 ? vec3(0.0, col / 0.8, 0.0) : mix(vec3(0.0, 1.0, 0.0), vec3(1.0), (col - 0.8) / 0.2);
                
                result = vec4(c * b, 1.0);
            }
        }
        
        position.x += 0.05;
        
        scaledown = DROP_SIZE;
        rx = fragCoord.x / (40.0 * scaledown);
        mx = 40.0*scaledown*fract(position.x * 30.0 * scaledown);
        
        if (mx > 12.0 * scaledown) {
            result += vec4(0.0);
        } else {
            float x = floor(rx);
            float r1x = floor(fragCoord.x / (12.0));
            
            float ry = position.y*700.0 + rand(vec2(x, x * 3.0)) * 100000.0 + globalTime* rand(vec2(r1x, 23.0)) * 120.0;
            float my = mod(ry, 15.0);
            if (my > 12.0 * scaledown) {
                result += vec4(0.0);
            } else {
                float y = floor(ry / 15.0);
                
                float b = rchar(vec2(rx, floor((ry) / 15.0)), vec2(mx, my) / 12.0, globalTime);
                float col = max(mod(-y, 24.0) - 4.0, 0.0) / 20.0;
                vec3 c = col < 0.8 ? vec3(0.0, col / 0.8, 0.0) : mix(vec3(0.0, 1.0, 0.0), vec3(1.0), (col - 0.8) / 0.2);
                
                result += vec4(c * b, 1.0);
            }
        }
        
        vec4 videoColor = texture2D(iChannel0, vTextureCoord);
        result = result + videoColor * 0.5;
        if(result.b < 0.5)
            result.b = result.g * 0.5;
        gl_FragColor = result;
    }
`;

function initVideo() {
    video = document.createElement('video');
    video.src = 'video.mp4';
    video.autoplay = true;
    video.loop = true;
    video.muted = true;
    video.crossOrigin = "anonymous";

    // Video yükleme hatası yakalama
    video.onerror = function (e) {
        console.error("Video yükleme hatası:", e);
    };

    // Video yüklendiğinde bilgi verme
    video.onloadeddata = function () {
        console.log("Video başarıyla yüklendi");
    };

    video.play().catch(function (error) {
        console.log("Video oynatma hatası:", error);
    });
    return video;
}

function initTexture(gl) {
    const texture = gl.createTexture();
    gl.bindTexture(gl.TEXTURE_2D, texture);

    // Fill the texture with a 1x1 blue pixel until the video loads
    gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, 1, 1, 0, gl.RGBA, gl.UNSIGNED_BYTE,
        new Uint8Array([0, 0, 255, 255]));

    // Set texture parameters
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);

    return texture;
}

function updateTexture(gl, texture, video) {
    gl.bindTexture(gl.TEXTURE_2D, texture);
    gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE, video);
}

function initGL() {
    const canvas = document.querySelector("#glCanvas");
    gl = canvas.getContext("webgl");

    if (!gl) {
        alert("Unable to initialize WebGL. Your browser may not support it.");
        return;
    }

    // Initialize video and texture
    video = initVideo();
    texture = initTexture(gl);

    // Initialize shaders
    const vertexShader = loadShader(gl, gl.VERTEX_SHADER, vsSource);
    const fragmentShader = loadShader(gl, gl.FRAGMENT_SHADER, fsSource);

    // Create the shader program
    program = gl.createProgram();
    gl.attachShader(program, vertexShader);
    gl.attachShader(program, fragmentShader);
    gl.linkProgram(program);

    if (!gl.getProgramParameter(program, gl.LINK_STATUS)) {
        alert('Unable to initialize the shader program: ' + gl.getProgramInfoLog(program));
        return;
    }

    // Initialize buffers
    const positions = [
        -1.0, -1.0,
        1.0, -1.0,
        -1.0, 1.0,
        1.0, 1.0,
    ];

    const positionBuffer = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, positionBuffer);
    gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(positions), gl.STATIC_DRAW);

    const textureCoordinates = [
        0.0, 1.0,
        1.0, 1.0,
        0.0, 0.0,
        1.0, 0.0,
    ];

    const textureCoordBuffer = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, textureCoordBuffer);
    gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(textureCoordinates), gl.STATIC_DRAW);

    return {
        position: positionBuffer,
        textureCoord: textureCoordBuffer,
    };
}

function loadShader(gl, type, source) {
    const shader = gl.createShader(type);
    gl.shaderSource(shader, source);
    gl.compileShader(shader);

    if (!gl.getShaderParameter(shader, gl.COMPILE_STATUS)) {
        alert('An error occurred compiling the shaders: ' + gl.getShaderInfoLog(shader));
        gl.deleteShader(shader);
        return null;
    }

    return shader;
}

function resizeCanvasToDisplaySize(canvas) {
    const displayWidth = canvas.clientWidth;
    const displayHeight = canvas.clientHeight;

    if (canvas.width !== displayWidth || canvas.height !== displayHeight) {
        canvas.width = displayWidth;
        canvas.height = displayHeight;
        gl.viewport(0, 0, canvas.width, canvas.height);
    }
}

function render(buffers, now) {
    if (!startTime) startTime = now;
    const time = (now - startTime) * 0.001; // Convert to seconds

    resizeCanvasToDisplaySize(gl.canvas);

    gl.clearColor(0.0, 0.0, 0.0, 1.0);
    gl.clear(gl.COLOR_BUFFER_BIT);

    gl.useProgram(program);

    // Update video texture
    if (video.readyState >= 2) {
        updateTexture(gl, texture, video);
    }

    // Set uniforms
    const timeLocation = gl.getUniformLocation(program, 'iTime');
    const resolutionLocation = gl.getUniformLocation(program, 'iResolution');
    const textureLocation = gl.getUniformLocation(program, 'iChannel0');

    gl.uniform1f(timeLocation, time);
    gl.uniform2f(resolutionLocation, gl.canvas.width, gl.canvas.height);

    // Set up texture unit 0
    gl.activeTexture(gl.TEXTURE0);
    gl.bindTexture(gl.TEXTURE_2D, texture);
    gl.uniform1i(textureLocation, 0);

    // Set position attribute
    const positionLocation = gl.getAttribLocation(program, 'aVertexPosition');
    gl.bindBuffer(gl.ARRAY_BUFFER, buffers.position);
    gl.vertexAttribPointer(positionLocation, 2, gl.FLOAT, false, 0, 0);
    gl.enableVertexAttribArray(positionLocation);

    // Set texture coordinates attribute
    const textureCoordLocation = gl.getAttribLocation(program, 'aTextureCoord');
    gl.bindBuffer(gl.ARRAY_BUFFER, buffers.textureCoord);
    gl.vertexAttribPointer(textureCoordLocation, 2, gl.FLOAT, false, 0, 0);
    gl.enableVertexAttribArray(textureCoordLocation);

    // Draw
    gl.drawArrays(gl.TRIANGLE_STRIP, 0, 4);

    requestAnimationFrame((now) => render(buffers, now));
}

// Initialize and start rendering
const buffers = initGL();
if (buffers) {
    requestAnimationFrame((now) => render(buffers, now));
} 