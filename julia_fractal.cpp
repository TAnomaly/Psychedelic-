#include <GL/glew.h>
#include <GL/freeglut.h>
#include <complex>
#include <cmath>
#include <vector>
#include <string>
#include <iostream>

// Pencere boyutları
const int WIDTH = 1920;
const int HEIGHT = 1080;

// Shader programları
GLuint shaderProgram;
GLuint fractalShader;
GLuint particleShader;
GLuint quadVAO, quadVBO;

// Uniform lokasyonları
GLint timeLocation;
GLint resolutionLocation;
GLint zoomLocation;
GLint offsetLocation;
GLint juliaParamLocation;
GLint colorParamsLocation;

// Fraktal parametreleri
float zoom = 1.0f;
float offsetX = 0.0f;
float offsetY = 0.0f;
float juliaX = -0.4f;
float juliaY = 0.6f;
float time_value = 0.0f;
bool autoRotate = true;
float rotationSpeed = 0.001f;

// Vertex shader kodu
const char *vertexShaderSource = R"(
    #version 330 core
    layout (location = 0) in vec3 aPos;
    layout (location = 1) in vec2 aTexCoord;
    out vec2 TexCoord;
    void main() {
        gl_Position = vec4(aPos, 1.0);
        TexCoord = aTexCoord;
    }
)";

// Fragment shader kodu
const char *fragmentShaderSource = R"(
    #version 330 core
    out vec4 FragColor;
    in vec2 TexCoord;
    
    uniform vec2 resolution;
    uniform float time;
    uniform float zoom;
    uniform vec2 offset;
    uniform vec2 juliaParam;
    uniform vec4 colorParams;
    
    #define MAX_ITER 1000
    #define PI 3.14159265359
    
    vec3 hsv2rgb(vec3 c) {
        vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
        vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
        return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
    }
    
    float mandala(vec2 uv, float time) {
        float angle = atan(uv.y, uv.x);
        float dist = length(uv);
        float symmetry = 12.0;
        return sin(symmetry * angle + dist * 5.0 + time) * 0.5 + 0.5;
    }
    
    void main() {
        vec2 uv = (gl_FragCoord.xy - 0.5 * resolution.xy) / min(resolution.x, resolution.y);
        uv = uv * 3.0 / zoom + offset;
        
        // Mandala ve spiral efektleri
        float mandalaEffect = mandala(uv, time * 0.2) * 0.1;
        float spiralEffect = sin(length(uv) * 10.0 - atan(uv.y, uv.x) * 2.0 + time) * 0.05;
        uv += vec2(mandalaEffect + spiralEffect);
        
        // Julia seti hesaplama
        vec2 z = uv;
        int iter;
        for(iter = 0; iter < MAX_ITER; iter++) {
            float x = z.x * z.x - z.y * z.y + juliaParam.x;
            float y = 2.0 * z.x * z.y + juliaParam.y;
            
            if(x*x + y*y > 4.0) break;
            z = vec2(x, y);
            
            // Deformasyon efekti
            float deform = sin(time * 0.5) * 0.01;
            z += vec2(sin(z.y * 5.0 + time) * deform, cos(z.x * 5.0 + time) * deform);
        }
        
        if(iter == MAX_ITER) {
            FragColor = vec4(0.0, 0.0, 0.0, 1.0);
        } else {
            float smooth_iter = float(iter) + 1.0 - log2(log2(dot(z,z)));
            float hue = mod(smooth_iter * 0.01 + time * 0.1, 1.0);
            float sat = 0.8 + sin(time + smooth_iter * 0.1) * 0.2;
            float val = 1.0 - smooth_iter / float(MAX_ITER);
            val = pow(val, 0.5); // Gamma düzeltme
            
            // Glow efekti
            float glow = exp(-smooth_iter * 0.02);
            vec3 color = hsv2rgb(vec3(hue, sat, val));
            color += vec3(1.0, 0.7, 0.3) * glow;
            
            // Kenar vurgulama
            float edge = 1.0 - abs(mod(smooth_iter * 0.1 + time, 2.0) - 1.0);
            color += vec3(0.2, 0.5, 1.0) * edge * edge * 0.5;
            
            FragColor = vec4(color, 1.0);
        }
        
        // Post-processing efektleri
        vec2 vigUV = (gl_FragCoord.xy / resolution.xy - 0.5) * 2.0;
        float vignette = 1.0 - dot(vigUV, vigUV) * 0.3;
        FragColor.rgb *= vignette;
        
        // Bloom efekti
        float brightness = dot(FragColor.rgb, vec3(0.2126, 0.7152, 0.0722));
        float bloom = smoothstep(0.7, 1.0, brightness);
        FragColor.rgb += FragColor.rgb * bloom * 0.5;
        
        // Final renk ayarları
        FragColor.rgb = pow(FragColor.rgb, vec3(0.9)); // Gamma
        FragColor.rgb = mix(FragColor.rgb, vec3(dot(FragColor.rgb, vec3(0.299, 0.587, 0.114))), 0.1); // Kontrast
    }
)";

// Shader derleme ve bağlama
GLuint createShader(const char *source, GLenum type)
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        std::cerr << "Shader compilation error: " << infoLog << std::endl;
    }
    return shader;
}

void initShaders()
{
    GLuint vertexShader = createShader(vertexShaderSource, GL_VERTEX_SHADER);
    GLuint fragmentShader = createShader(fragmentShaderSource, GL_FRAGMENT_SHADER);

    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    GLint success;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success)
    {
        char infoLog[512];
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cerr << "Shader program linking error: " << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Uniform lokasyonlarını al
    timeLocation = glGetUniformLocation(shaderProgram, "time");
    resolutionLocation = glGetUniformLocation(shaderProgram, "resolution");
    zoomLocation = glGetUniformLocation(shaderProgram, "zoom");
    offsetLocation = glGetUniformLocation(shaderProgram, "offset");
    juliaParamLocation = glGetUniformLocation(shaderProgram, "juliaParam");
    colorParamsLocation = glGetUniformLocation(shaderProgram, "colorParams");
}

// Quad mesh oluşturma
void initQuad()
{
    float quadVertices[] = {
        // Pozisyonlar   // Texture koordinatları
        -1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
        1.0f, -1.0f, 0.0f, 1.0f, 0.0f,

        -1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 0.0f, 1.0f, 1.0f};

    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);

    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
}

// Fare kontrolü için değişkenler
int lastX = 0, lastY = 0;
bool isDragging = false;

void display()
{
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(shaderProgram);

    // Uniform değişkenleri güncelle
    glUniform1f(timeLocation, time_value);
    glUniform2f(resolutionLocation, WIDTH, HEIGHT);
    glUniform1f(zoomLocation, zoom);
    glUniform2f(offsetLocation, offsetX, offsetY);
    glUniform2f(juliaParamLocation, juliaX, juliaY);
    glUniform4f(colorParamsLocation, 1.0f, 1.0f, 1.0f, 1.0f);

    // Quad'ı çiz
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glutSwapBuffers();
}

void reshape(int w, int h)
{
    glViewport(0, 0, w, h);
}

void motion(int x, int y)
{
    if (isDragging)
    {
        float dx = (x - lastX) * 2.0f / WIDTH / zoom;
        float dy = (y - lastY) * 2.0f / HEIGHT / zoom;
        offsetX -= dx;
        offsetY += dy;
        lastX = x;
        lastY = y;
        glutPostRedisplay();
    }
}

void mouse(int button, int state, int x, int y)
{
    if (button == GLUT_LEFT_BUTTON)
    {
        if (state == GLUT_DOWN)
        {
            isDragging = true;
            lastX = x;
            lastY = y;
        }
        else
        {
            isDragging = false;
        }
    }
    else if (button == 3)
    { // Fare tekerleği yukarı
        zoom *= 1.1f;
        glutPostRedisplay();
    }
    else if (button == 4)
    { // Fare tekerleği aşağı
        zoom /= 1.1f;
        glutPostRedisplay();
    }
}

void keyboard(unsigned char key, int x, int y)
{
    switch (key)
    {
    case 27: // ESC
        glutLeaveMainLoop();
        break;
    case 'r':
        autoRotate = !autoRotate;
        break;
    case '+':
        rotationSpeed *= 1.2f;
        break;
    case '-':
        rotationSpeed /= 1.2f;
        break;
    case ' ':
        zoom = 1.0f;
        offsetX = 0.0f;
        offsetY = 0.0f;
        break;
    }
    glutPostRedisplay();
}

void update(int value)
{
    time_value += 0.016f;

    if (autoRotate)
    {
        float angle = rotationSpeed;
        float tempX = juliaX * cos(angle) - juliaY * sin(angle);
        float tempY = juliaX * sin(angle) + juliaY * cos(angle);
        juliaX = tempX;
        juliaY = tempY;
    }

    glutPostRedisplay();
    glutTimerFunc(16, update, 0);
}

int main(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowSize(WIDTH, HEIGHT);
    glutCreateWindow("Ultra HD Julia Fractal");

    // GLEW başlatma
    GLenum err = glewInit();
    if (GLEW_OK != err)
    {
        std::cerr << "GLEW initialization failed: " << glewGetErrorString(err) << std::endl;
        return 1;
    }

    // OpenGL ayarları
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // Shader ve quad başlatma
    initShaders();
    initQuad();

    // GLUT callback fonksiyonları
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutMotionFunc(motion);
    glutMouseFunc(mouse);
    glutKeyboardFunc(keyboard);
    glutTimerFunc(0, update, 0);

    glutMainLoop();

    // Temizlik
    glDeleteVertexArrays(1, &quadVAO);
    glDeleteBuffers(1, &quadVBO);
    glDeleteProgram(shaderProgram);

    return 0;
}