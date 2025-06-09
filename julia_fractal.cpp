#include <GL/glew.h>
#include <GL/freeglut.h>
#include <complex>
#include <cmath>
#include <vector>
#include <string>
#include <iostream>

// Pencere boyutları - 4K destekli
const int WIDTH = 1920;
const int HEIGHT = 1080;

// Shader programı
GLuint shaderProgram;
GLuint quadVAO, quadVBO;

// Uniform lokasyonları
GLint timeLocation;
GLint resolutionLocation;
GLint zoomLocation;
GLint offsetLocation;
GLint juliaParamLocation;
GLint modeLocation;
GLint complexityLocation;

// Fraktal parametreleri
float zoom = 2.5f;
float offsetX = 0.0f;
float offsetY = 0.0f;
float juliaX = -0.4f;
float juliaY = 0.6f;
float time_value = 0.0f;
bool autoRotate = true;
float rotationSpeed = 0.0003f;
int colorMode = 0;       // Farklı renk paletleri için
float complexity = 1.0f; // Karmaşıklık seviyesi

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

// Fragment shader kodu - Sanatsal ve Psychedelic geliştirmeler
const char *fragmentShaderSource = R"(
    #version 330 core
    out vec4 FragColor;
    in vec2 TexCoord;
    
    uniform vec2 resolution;
    uniform float time;
    uniform float zoom;
    uniform vec2 offset;
    uniform vec2 juliaParam;
    uniform int mode;
    uniform float complexity;
    
    #define MAX_ITER 200 // Daha hızlı iterasyonlar için düşürüldü, daha akışkan hareket
    #define PI 3.14159265359
    #define TAU 6.28318530718
    
    // Psychedelic renk paletleri
    vec3 psychedelicPalette1(float t) {
        // Hızlı, canlı, dönen tonlar
        vec3 color = 0.5 + 0.5 * cos(TAU * (t * 3.0 + vec3(0.0, 0.333, 0.666) + time * 0.5));
        return clamp(color, 0.0, 1.0); // Renk değerlerini 0-1 aralığında tutmak için
    }

    vec3 psychedelicPalette2(float t) {
        // Kontrastlı, şok edici renk geçişleri
        vec3 c1 = vec3(1.0, 0.0, 0.5); // Magenta
        vec3 c2 = vec3(0.0, 1.0, 0.8); // Turkuaz
        vec3 c3 = vec3(1.0, 0.8, 0.0); // Turuncu
        vec4 c4 = vec4(0.5, 0.0, 1.0, 1.0); // Mor (alpha ile uyum için)

        t = fract(t + time * 0.2); // Daha hızlı kayma
        if (t < 0.25) return mix(c1, c2, smoothstep(0.0, 1.0, t * 4.0));
        else if (t < 0.5) return mix(c2, c3, smoothstep(0.0, 1.0, (t - 0.25) * 4.0));
        else if (t < 0.75) return mix(c3, c4.rgb, smoothstep(0.0, 1.0, (t - 0.5) * 4.0));
        else return mix(c4.rgb, c1, smoothstep(0.0, 1.0, (t - 0.75) * 4.0));
    }

    vec3 quantumFlux(float t) {
        // Kuantum fiziği esinlenmesi, daha dinamik ve parlayan
        float wave = sin(t * 30.0 + time * 5.0) * 0.5 + 0.5; // Daha hızlı titreşim
        vec3 photon = vec3(1.0, 1.0, 0.8) * (1.0 + sin(time * 7.0) * 0.1); // Parlama
        vec3 electron = vec3(0.2, 0.4, 1.0) * (1.0 + cos(time * 6.0) * 0.1);
        vec3 quantum = vec3(0.8, 0.2, 0.8) * (1.0 + sin(time * 8.0) * 0.1);
        
        return mix(mix(photon, electron, wave), quantum, sin(t * 10.0 + time * 3.0) * 0.5 + 0.5);
    }

    vec3 cosmicPalette(float t) {
        // Derin uzay ve nebula renkleri, daha akışkan
        vec3 deep = vec3(0.05, 0.0, 0.2); 
        vec3 nebula = vec3(0.8, 0.2, 0.9); 
        vec3 star = vec3(1.0, 0.9, 0.3); 
        vec3 plasma = vec3(0.0, 0.8, 1.0); 
        
        t = fract(t + time * 0.05) * 4.0; // Hafif kayma
        if(t < 1.0) return mix(deep, nebula, smoothstep(0.0, 1.0, t));
        else if(t < 2.0) return mix(nebula, star, smoothstep(0.0, 1.0, t-1.0));
        else if(t < 3.0) return mix(star, plasma, smoothstep(0.0, 1.0, t-2.0));
        return mix(plasma, deep, smoothstep(0.0, 1.0, t-3.0));
    }
    
    // Ana color function
    vec3 getColor(float t, int colorMode, float time) {
        switch(colorMode) {
            case 0: return psychedelicPalette1(t);
            case 1: return psychedelicPalette2(t);
            case 2: return quantumFlux(t);
            case 3: return cosmicPalette(t);
            default: return psychedelicPalette1(t);
        }
    }
    
    // Gelişmiş geometrik transformasyonlar
    vec2 kaleidoscope(vec2 uv, float segments) {
        float angle = atan(uv.y, uv.x);
        float radius = length(uv);
        angle = mod(angle, TAU / segments);
        angle = abs(angle - PI / segments);
        return vec2(cos(angle), sin(angle)) * radius;
    }
    
    vec2 fractalDistortion(vec2 uv, float time, float intensity) {
        // Çoklu fraktal katmanları ve girdap etkisi
        float scale1 = 3.0, scale2 = 7.0, scale3 = 13.0;
        
        vec2 distort = vec2(
            sin(uv.y * scale1 + time * 1.5) * sin(uv.x * scale2 + time * 1.3) * intensity,
            cos(uv.x * scale1 + time * 1.7) * cos(uv.y * scale3 + time * 1.9) * intensity
        );
        
        // Girdap deformasyonu
        float angle = atan(uv.y, uv.x);
        float dist = length(uv);
        float swirl = sin(dist * 10.0 - time * 2.0) * 0.05 * intensity;
        angle += swirl;
        distort += vec2(cos(angle), sin(angle)) * dist * 0.1 * intensity;

        return uv + distort;
    }
    
    void main() {
        vec2 uv = (gl_FragCoord.xy - 0.5 * resolution.xy) / min(resolution.x, resolution.y);
        vec2 originalUV = uv;
        
        // Dinamik zoom ve solunum efekti - daha belirgin
        float breathe = sin(time * 0.7) * 0.2 + 1.0;
        float dynamicZoom = zoom * (1.0 + sin(time * 0.1) * 0.5);
        uv = uv * (3.0 / dynamicZoom) * breathe;
        
        // Karmaşıklık seviyesine göre transformasyonlar - daha etkileşimli
        uv = kaleidoscope(uv, 4.0 + sin(time * 0.4) * 3.0 + complexity * 5.0);
        
        // Fraktal distorsiyon - karmaşıklıkla daha yoğun
        uv = fractalDistortion(uv, time, complexity * 0.5 + sin(time * 0.8) * 0.1);
        uv += offset;
        
        // Julia parametrelerinde harmonic motion - daha hızlı ve geniş
        vec2 c = juliaParam;
        c.x += sin(time * 0.25) * 0.2 * complexity;
        c.y += cos(time * 0.35) * 0.2 * complexity;
        
        // Ana fraktal hesaplama
        vec2 z = uv;
        int iter;
        float smoothIter = 0.0;
        
        for(iter = 0; iter < MAX_ITER; iter++) {
            float x = z.x * z.x - z.y * z.y + c.x;
            float y = 2.0 * z.x * z.y + c.y;
            
            float magnitudeSq = x*x + y*y;
            if(magnitudeSq > 4.0) {
                smoothIter = float(iter) + 1.0 - log2(log2(magnitudeSq));
                break;
            }
            z = vec2(x, y);
        }
        
        if(iter == MAX_ITER) {
            // İç bölge için hareketli bir desen
            float innerPattern = sin(length(originalUV) * 30.0 + time * 10.0) * 0.5 + 0.5;
            vec3 innerColor = getColor(innerPattern, mode, time) * 0.2;
            FragColor = vec4(innerColor, 1.0);
        } else {
            // Gelişmiş renk hesaplaması
            float normalizedIter = smoothIter / float(MAX_ITER);
            vec3 color = getColor(normalizedIter, mode, time);
            
            // Artistik efektler - daha fazla parıltı ve titreşim
            float glow = exp(-smoothIter * 0.01) * (0.5 + sin(time * 5.0) * 0.5);
            color += getColor(time * 0.2, (mode + 1) % 4, time) * glow * 2.0;
            
            // Vignette efekti - daha dramatik
            float vignette = 1.0 - length(originalUV) * 0.8;
            vignette = smoothstep(0.0, 1.0, vignette);
            vignette = pow(vignette, 2.0);
            
            // Dynamic brightness - daha belirgin nabız atışı
            float pulse = sin(time * 4.0) * 0.3 + 0.7;
            color *= vignette * pulse;
            
            // HDR ve Tonemap - daha parlak ve dinamik
            color = color / (0.1 + color);
            color = pow(color, vec3(1.0 / 2.0));

            // Film grain effect - hafif kumlanma
            float grain = fract(sin(dot(originalUV * resolution, vec2(12.9898, 78.233))) * 43758.5453);
            color += (grain - 0.5) * 0.03;
            
            FragColor = vec4(color, 1.0);
        }
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
    modeLocation = glGetUniformLocation(shaderProgram, "mode");
    complexityLocation = glGetUniformLocation(shaderProgram, "complexity");
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
    glUniform1i(modeLocation, colorMode);
    glUniform1f(complexityLocation, complexity);

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
        std::cout << "Auto rotation: " << (autoRotate ? "ON" : "OFF") << std::endl;
        break;
    case '+':
        rotationSpeed *= 1.2f;
        std::cout << "Rotation speed: " << rotationSpeed << std::endl;
        break;
    case '-':
        rotationSpeed /= 1.2f;
        std::cout << "Rotation speed: " << rotationSpeed << std::endl;
        break;
    case ' ':
        zoom = 2.5f;
        offsetX = 0.0f;
        offsetY = 0.0f;
        juliaX = -0.4f; // Julia parametresini de sıfırla
        juliaY = 0.6f;
        std::cout << "Reset view" << std::endl;
        break;
    case 'c':
        colorMode = (colorMode + 1) % 4; // Toplam 4 renk modu var
        std::cout << "Color mode: " << colorMode << std::endl;
        break;
    case 'C':
        colorMode = (colorMode + 3) % 4; // Geriye doğru
        std::cout << "Color mode: " << colorMode << std::endl;
        break;
    case 'x':
        complexity += 0.05f; // Daha ince ayar
        if (complexity > 1.0f)
            complexity = 1.0f;
        std::cout << "Complexity: " << complexity << std::endl;
        break;
    case 'X':
        complexity -= 0.05f; // Daha ince ayar
        if (complexity < 0.0f)
            complexity = 0.0f;
        std::cout << "Complexity: " << complexity << std::endl;
        break;
    case 'h':
        std::cout << "\n=== PSYCHEDELIC JULIA FRACTAL EXPLORER CONTROLS ===" << std::endl;
        std::cout << "ESC       - Exit" << std::endl;
        std::cout << "SPACE     - Reset view (zoom, offset, julia param)" << std::endl;
        std::cout << "R         - Toggle auto rotation of Julia parameter" << std::endl;
        std::cout << "+/-       - Adjust rotation speed" << std::endl;
        std::cout << "C/Shift+C - Change color palette" << std::endl;
        std::cout << "X/Shift+X - Adjust complexity of distortions and animations" << std::endl;
        std::cout << "Mouse     - Pan (drag) and Zoom (wheel)" << std::endl;
        std::cout << "H         - Show this help" << std::endl;
        std::cout << "=====================================================\n"
                  << std::endl;
        break;
    }
    glutPostRedisplay();
}

void update(int value)
{
    time_value += 0.016f; // Yaklaşık 60 FPS

    if (autoRotate)
    {
        // Dinamik dönüş hızı, zamanla değişen psychedelic bir etki için
        float currentRotationSpeed = rotationSpeed * (1.0 + sin(time_value * 0.5) * 0.5);
        float angle = currentRotationSpeed;
        float tempX = juliaX * cos(angle) - juliaY * sin(angle);
        float tempY = juliaX * sin(angle) + juliaY * cos(angle);
        juliaX = tempX;
        juliaY = tempY;
    }

    // Ek dinamik efektler (opsiyonel, denenebilir)
    // zoom = 2.5f + sin(time_value * 0.1) * 1.5f; // Zoomda dalgalanma
    // offsetX = sin(time_value * 0.08) * 0.5f; // Offset'te yatay hareket
    // offsetY = cos(time_value * 0.12) * 0.5f; // Offset'te dikey hareket
    // complexity = 0.5f + sin(time_value * 0.2) * 0.5f; // Karmaşıklıkta dalgalanma

    glutPostRedisplay();
    glutTimerFunc(16, update, 0); // 16 ms sonra tekrar çağır (yaklaşık 60 FPS)
}

int main(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowSize(WIDTH, HEIGHT);
    glutCreateWindow("🌈 Psychedelic Julia Fractal Explorer 🌌");

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

    // Başlangıç mesajı
    std::cout << "\n🌈 PSYCHEDELIC JULIA FRACTAL EXPLORER 🌌" << std::endl;
    std::cout << "Press 'H' for help and controls" << std::endl;
    std::cout << "Prepare for a visual journey!\n"
              << std::endl;

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