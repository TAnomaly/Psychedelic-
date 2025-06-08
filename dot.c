#include <GL/glut.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <GL/glew.h> // GLEW kullanımı için gerekli

// --- Tanımlar ---
#define WINDOW_WIDTH 1920
#define WINDOW_HEIGHT 1080
#define DOT_SIZE 3.5f         // Temel nokta boyutu
#define TEXT_DOT_SIZE 5.0f    // Metin noktası boyutu
#define GRID_SIZE 80          // Düşen noktaların grid yoğunluğu
#define TEXT_RESOLUTION 0.02f // Metin pürüzsüzlüğü (küçük değerler daha yoğun nokta)

// --- Shader Kaynak Kodları ---
// Vertex Shader: Her noktaya özel renk, zaman ve metin bayrağını gönderir.
const char *vertexShaderSource =
    "#version 120\n"
    "attribute vec4 customColor;\n" // Noktanın temel rengi
    "attribute float isTextDot;\n"  // Bu noktanın metin noktası olup olmadığını belirler
    "uniform float time;\n"         // Animasyon zamanı

    "varying vec4 vColor;\n"      // Fragment shader'a iletilecek renk
    "varying float vIsTextDot;\n" // Fragment shader'a iletilecek metin bayrağı

    "void main() {\n"
    "    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\n"
    "    vColor = customColor;\n"
    "    vIsTextDot = isTextDot;\n"
    "    \n"
    "    // Metin noktaları için hafif pozisyon kaydırması (dalgalanma efekti)
    "    if (isTextDot > 0.5) {\n"
    "        float wave = sin(time * 8.0 + gl_Vertex.x * 0.02) * 0.5;\n"
    "        gl_Position.y += wave;\n"
    "    }\n"
    "}\n";

// Fragment Shader: Psychedelic renk değişimleri ve parlamalar.
const char *fragmentShaderSource =
    "#version 120\n"
    "uniform float time;\n"
    "varying vec4 vColor;\n"      // Vertex shader'dan gelen renk
    "varying float vIsTextDot;\n" // Vertex shader'dan gelen metin bayrağı

    "uniform float globalGlowFactor;\n"  // Global parlatma faktörü
    "uniform float textGlowIntensity;\n" // Metin parlatma yoğunluğu

    "void main() {\n"
    "    vec4 finalColor = vColor;\n"
    "    float dist = length(gl_PointCoord - vec2(0.5));\n"
    "    float alpha = 1.0 - smoothstep(0.45, 0.5, dist);\n" // Noktaların kenarlarını yumuşatır ve yuvarlak yapar

    "    // Genel parlatma etkisi (tüm noktalar için)
    "    float generalGlow = sin(time * 2.0) * 0.3 + 0.7;\n"
    "    finalColor.rgb *= generalGlow * globalGlowFactor;\n"

    "    // Metin noktaları için psychedelic renk geçişleri ve ekstra parlatma
    "    if (vIsTextDot > 0.5) {\n"
    "        // Zamanla ve ekran konumuna göre değişen renk geçişleri
    "        float r = sin(time * 3.0 + gl_FragCoord.x * 0.005) * 0.5 + 0.5;\n"
    "        float g = sin(time * 4.0 + gl_FragCoord.y * 0.005 + 2.0) * 0.5 + 0.5;\n"
    "        float b = sin(time * 5.0 + gl_FragCoord.x * 0.003 + 4.0) * 0.5 + 0.5;\n"
    "        \n"
    "        // Metin rengini karıştır
    "        finalColor.rgb = mix(finalColor.rgb, vec3(r, g, b), 0.7);\n" // Orijinal yeşil ile yeni renkleri karıştır

    "        // Metin parlaması
    "        float textSpecificGlow = sin(time * 6.0 + gl_FragCoord.x * 0.01 + gl_FragCoord.y * 0.008) * 0.5 + 0.5;\n"
    "        finalColor.rgb += finalColor.rgb * textSpecificGlow * textGlowIntensity;\n"
    "        finalColor.a = 0.8 + textSpecificGlow * 0.2;\n" // Metin opaklığını da dinamikleştir
    "    }\n"

    "    gl_FragColor = vec4(finalColor.rgb, finalColor.a * alpha);\n"
    "}\n";

// --- Yapılar ve Global Değişkenler ---
typedef struct
{
    float x, y;
} Point;

#define MAX_POINTS 6000 // Metin nokta sayısını artırdık
Point text_dots[MAX_POINTS];
int text_dot_count = 0;

float dots[GRID_SIZE][GRID_SIZE][3];   // x, y, opacity
float colors[GRID_SIZE][GRID_SIZE][3]; // r, g, b

float time_value = 0.0f;
int dots_initialized = 0;
GLuint shaderProgram;

// Shader uniform ve attribute lokasyonları
GLint timeLocation;
GLint customColorLocation;
GLint isTextDotLocationAttr; // isTextDot artık bir attribute
GLint globalGlowFactorLocation;
GLint textGlowIntensityLocation;

// Kontrol değişkenleri
float currentGlobalGlowFactor = 1.0f;
float currentTextGlowIntensity = 1.5f;

// --- Yardımcı Fonksiyonlar ---
// Shader derleme hatası kontrolü
void checkShaderError(GLuint shader)
{
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        fprintf(stderr, "Shader compilation error: %s\n", infoLog);
    }
}

// Shader program bağlama hatası kontrolü
void checkProgramError(GLuint program)
{
    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success)
    {
        char infoLog[512];
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        fprintf(stderr, "Shader program linking error: %s\n", infoLog);
    }
}

// Shader'ları başlatır ve uniform/attribute lokasyonlarını alır
void initShaders()
{
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);

    glCompileShader(vertexShader);
    checkShaderError(vertexShader);

    glCompileShader(fragmentShader);
    checkShaderError(fragmentShader);

    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);

    // Attribute'ları bağla
    glBindAttribLocation(shaderProgram, 0, "customColor");
    glBindAttribLocation(shaderProgram, 1, "isTextDot"); // isTextDot için yeni slot

    glLinkProgram(shaderProgram);
    checkProgramError(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Uniform lokasyonlarını al
    timeLocation = glGetUniformLocation(shaderProgram, "time");
    globalGlowFactorLocation = glGetUniformLocation(shaderProgram, "globalGlowFactor");
    textGlowIntensityLocation = glGetUniformLocation(shaderProgram, "textGlowIntensity");

    // Attribute lokasyonlarını al
    customColorLocation = glGetAttribLocation(shaderProgram, "customColor");
    isTextDotLocationAttr = glGetAttribLocation(shaderProgram, "isTextDot");

    if (customColorLocation == -1 || isTextDotLocationAttr == -1)
    {
        fprintf(stderr, "Warning: Shader attributes not found or not active.\n");
    }
}

// Metin noktası ekler
void addTextDot(float x, float y)
{
    if (text_dot_count < MAX_POINTS)
    {
        text_dots[text_dot_count].x = x;
        text_dots[text_dot_count].y = y;
        text_dot_count++;
    }
}

// Harf çizim fonksiyonları (metin çözünürlüğü ve nokta yoğunluğu artırıldı)
void addLetterW(float x, float y, float size)
{
    float step = TEXT_RESOLUTION;
    for (float i = 0; i <= 1; i += step)
    {
        addTextDot(x + i * size * 0.2f, y + size - i * size);
        addTextDot(x + i * size * 0.4f + size * 0.2f, y + i * size);
        addTextDot(x + i * size * 0.4f + size * 0.6f, y + size - i * size);
        addTextDot(x + i * size * 0.2f + size * 0.8f, y + i * size);
    }
}

void addLetterA(float x, float y, float size)
{
    float step = TEXT_RESOLUTION;
    for (float i = 0; i <= 1; i += step)
    {
        addTextDot(x + i * size * 0.5f, y + i * size);
        addTextDot(x + size - i * size * 0.5f, y + i * size);
        if (i <= 0.5f)
        {
            addTextDot(x + size * 0.25f + i * size * 0.5f, y + size * 0.5f);
        }
    }
}

void addLetterK(float x, float y, float size)
{
    float step = TEXT_RESOLUTION;
    for (float i = 0; i <= 1; i += step)
    {
        addTextDot(x, y + i * size);
        addTextDot(x + i * size * 0.8f, y + size * 0.5f + (0.5f - i) * size);
        addTextDot(x + i * size * 0.8f, y + size * 0.5f - (0.5f - i) * size);
    }
}

void addLetterE(float x, float y, float size)
{
    float step = TEXT_RESOLUTION;
    for (float i = 0; i <= 1; i += step)
    {
        addTextDot(x, y + i * size);
        addTextDot(x + i * size * 0.8f, y + size);
        addTextDot(x + i * size * 0.6f, y + size * 0.5f);
        addTextDot(x + i * size * 0.8f, y);
    }
}

void addLetterU(float x, float y, float size)
{
    float step = TEXT_RESOLUTION;
    for (float i = 0; i <= 1; i += step)
    {
        addTextDot(x, y + size - i * size * 0.8f);
        addTextDot(x + i * size * 0.8f, y);
        addTextDot(x + size * 0.8f, y + size - i * size * 0.8f);
    }
}

void addLetterP(float x, float y, float size)
{
    float step = TEXT_RESOLUTION;
    for (float i = 0; i <= 1; i += step)
    {
        addTextDot(x, y + i * size);
        if (i <= 0.6f)
        {
            addTextDot(x + i * size * 0.8f, y + size);
            addTextDot(x + size * 0.8f, y + size - i * size * 0.6f);
            addTextDot(x + size * 0.8f - i * size * 0.8f, y + size * 0.4f);
        }
    }
}

void addLetterN(float x, float y, float size)
{
    float step = TEXT_RESOLUTION;
    for (float i = 0; i <= 1; i += step)
    {
        addTextDot(x, y + i * size);
        addTextDot(x + i * size * 0.8f, y + size - i * size);
        addTextDot(x + size * 0.8f, y + i * size);
    }
}

void addLetterO(float x, float y, float size)
{
    float step = TEXT_RESOLUTION;
    for (float i = 0; i <= 1; i += step)
    {
        float angle = i * 2 * 3.14159f;
        addTextDot(x + size * 0.4f + cos(angle) * size * 0.4f,
                   y + size * 0.5f + sin(angle) * size * 0.4f);
    }
}

void addLetterComma(float x, float y, float size)
{
    float step = TEXT_RESOLUTION * 0.5f;
    for (float i = 0; i <= 1; i += step)
    {
        addTextDot(x + size * 0.1f, y + size * 0.8f - i * size * 0.2f);
        addTextDot(x + size * 0.1f + i * size * 0.1f, y + size * 0.6f - i * size * 0.1f);
    }
}

void addLetterPeriod(float x, float y, float size)
{
    addTextDot(x, y + size * 0.1f);
}

// "WAKE UP, NEO..." metnini oluşturur
void initTextDots()
{
    text_dot_count = 0;
    float base_x = WINDOW_WIDTH / 2.0f - 400.0f;
    float base_y = WINDOW_HEIGHT / 2.0f + 100.0f;
    float letter_size = 100.0f;         // Harf boyutunu biraz küçülttük
    float spacing = letter_size * 1.0f; // Boşluğu biraz azalttık

    // First line: "WAKE"
    addLetterW(base_x, base_y, letter_size);
    addLetterA(base_x + spacing * 1.1f, base_y, letter_size);
    addLetterK(base_x + spacing * 2.2f, base_y, letter_size);
    addLetterE(base_x + spacing * 3.3f, base_y, letter_size);

    // Second line: "UP,"
    base_x = WINDOW_WIDTH / 2.0f - 100.0f;
    base_y -= letter_size * 1.3f; // Satır aralığını ayarladık
    addLetterU(base_x, base_y, letter_size);
    addLetterP(base_x + spacing * 1.1f, base_y, letter_size);
    addLetterComma(base_x + spacing * 2.0f, base_y, letter_size * 0.5f); // Virgül boyutu

    // Third line: "NEO..."
    base_x = WINDOW_WIDTH / 2.0f - 150.0f;
    base_y -= letter_size * 1.3f;
    addLetterN(base_x, base_y, letter_size);
    addLetterE(base_x + spacing * 1.1f, base_y, letter_size);
    addLetterO(base_x + spacing * 2.2f, base_y, letter_size);

    // Üç nokta
    addLetterPeriod(base_x + spacing * 3.3f, base_y, letter_size * 0.3f);
    addLetterPeriod(base_x + spacing * 3.5f, base_y, letter_size * 0.3f);
    addLetterPeriod(base_x + spacing * 3.7f, base_y, letter_size * 0.3f);
}

// Düşen noktaları ilk durumuna getirir ve renklerini atar
void initDots()
{
    for (int i = 0; i < GRID_SIZE; i++)
    {
        for (int j = 0; j < GRID_SIZE; j++)
        {
            dots[i][j][0] = i * (WINDOW_WIDTH / (float)GRID_SIZE);
            dots[i][j][1] = WINDOW_HEIGHT + (rand() % 1000); // Ekran dışından başlasın
            dots[i][j][2] = 0.0f;                            // Başlangıç opaklığı

            // Temel renk (yeşil tonları)
            float green = 0.6f + (float)(rand() % 40) / 100.0f; // Daha geniş yeşil ton yelpazesi
            colors[i][j][0] = green * 0.2f;
            colors[i][j][1] = green;
            colors[i][j][2] = green * 0.2f;
        }
    }
    dots_initialized = 1;
    initTextDots(); // Metin noktalarını da tekrar oluştur
}

// Tek bir noktayı çizer
void drawDot(float x, float y, float r, float g, float b, float a, float size, float is_text_dot_flag)
{
    glPointSize(size);

    // customColor ve isTextDot attribute'larını ayarla
    glVertexAttrib4f(customColorLocation, r, g, b, a);
    glVertexAttrib1f(isTextDotLocationAttr, is_text_dot_flag);

    glBegin(GL_POINTS);
    glVertex2f(x, y);
    glEnd();
}

// --- OpenGL Geri Çağırım Fonksiyonları ---
// Ekranı çizen fonksiyon
void display()
{
    glClear(GL_COLOR_BUFFER_BIT); // Arka planı temizle

    glEnable(GL_BLEND);                // Karıştırma (transparency) etkinleştir
    glBlendFunc(GL_SRC_ALPHA, GL_ONE); // Additive blending (parlayan efekt için ideal)

    glUseProgram(shaderProgram); // Shader programını kullan

    // Shader uniform değişkenlerini ayarla
    glUniform1f(timeLocation, time_value);
    glUniform1f(globalGlowFactorLocation, currentGlobalGlowFactor);
    glUniform1f(textGlowIntensityLocation, currentTextGlowIntensity);

    glEnable(GL_POINT_SMOOTH);                        // Noktaları yuvarlak hale getir (anti-aliasing)
    glEnableVertexAttribArray(customColorLocation);   // customColor attribute'unu etkinleştir
    glEnableVertexAttribArray(isTextDotLocationAttr); // isTextDot attribute'unu etkinleştir

    // Düşen noktaları çiz
    for (int i = 0; i < GRID_SIZE; i++)
    {
        for (int j = 0; j < GRID_SIZE; j++)
        {
            // Nokta ekranın altına düşerse, yukarıdan tekrar başlat
            if (dots[i][j][1] < -DOT_SIZE)
            {
                dots[i][j][1] = WINDOW_HEIGHT + (rand() % 200); // Farklı yüksekliklerden başlat
                dots[i][j][2] = 0.0f;                           // Opaklığı sıfırla

                // Her sıfırlamada renkleri hafifçe değiştir
                float green = 0.6f + (float)(rand() % 40) / 100.0f;
                colors[i][j][0] = green * 0.2f;
                colors[i][j][1] = green;
                colors[i][j][2] = green * 0.2f;
            }

            // Noktanın düşüş hızı (rastgelelik eklendi)
            float speed = 1.0f + (float)(rand() % 5); // Biraz daha yavaş düşüş başlangıcı
            dots[i][j][1] -= speed;
            // Opaklığı zamanla artır, böylece nokta belirginleşerek düşer
            dots[i][j][2] = fmin(1.0f, dots[i][j][2] + 0.008f); // Opaklık artış hızı

            // Düşen noktalar için isTextDot bayrağı 0.0
            drawDot(dots[i][j][0], dots[i][j][1],
                    colors[i][j][0], colors[i][j][1], colors[i][j][2],
                    dots[i][j][2] * 0.6f, DOT_SIZE, 0.0f); // Alfa değerini opaklık ile kontrol et
        }
    }

    // Metin noktalarını çiz (psychedelic glow efekti ile)
    for (int i = 0; i < text_dot_count; i++)
    {
        // Metin noktaları için isTextDot bayrağı 1.0
        drawDot(text_dots[i].x, text_dots[i].y,
                0.3f, 1.0f, 0.3f, // Metin için ana renk (parlak yeşil)
                1.0f, TEXT_DOT_SIZE, 1.0f);
    }

    glDisableVertexAttribArray(customColorLocation);   // Attribute'u devre dışı bırak
    glDisableVertexAttribArray(isTextDotLocationAttr); // Attribute'u devre dışı bırak
    glUseProgram(0);                                   // Shader programını kapat
    glDisable(GL_BLEND);
    glDisable(GL_POINT_SMOOTH);

    glutSwapBuffers(); // Çift tamponlama: çizilen resmi ekranda göster
}

// Animasyon güncelleme fonksiyonu
void update(int value)
{
    if (!dots_initialized)
    {
        initDots(); // İlk çalıştırmada noktaları başlat
    }

    time_value += 0.016f;         // Zamanı güncelle (yaklaşık 60 FPS için)
    glutPostRedisplay();          // Ekranın yeniden çizilmesini iste
    glutTimerFunc(16, update, 0); // 16 ms sonra tekrar çağır (yaklaşık 60 FPS)
}

// Pencere boyutlandırma fonksiyonu
void reshape(int w, int h)
{
    glViewport(0, 0, w, h);      // Görüntü alanını ayarla
    glMatrixMode(GL_PROJECTION); // Projeksiyon matris moduna geç
    glLoadIdentity();            // Matrisi sıfırla
    // 2D ortografik projeksiyon (0,0 sol alt, WINDOW_WIDTH,WINDOW_HEIGHT sağ üst)
    gluOrtho2D(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT);
    glMatrixMode(GL_MODELVIEW); // Model görünüm matris moduna geri dön
}

// Klavye etkileşimleri
void keyboard(unsigned char key, int x, int y)
{
    if (key == 'q' || key == 'Q')
    {
        exit(0); // Programdan çık
    }
    else if (key == 'r' || key == 'R')
    {
        initDots();        // Noktaları ve metni sıfırla
        time_value = 0.0f; // Animasyon zamanını sıfırla
        printf("Animation reset.\n");
    }
    else if (key == '+') // Global parlatmayı artır
    {
        currentGlobalGlowFactor += 0.1f;
        if (currentGlobalGlowFactor > 2.0f)
            currentGlobalGlowFactor = 2.0f;
        printf("Global Glow Factor: %.2f\n", currentGlobalGlowFactor);
    }
    else if (key == '-') // Global parlatmayı azalt
    {
        currentGlobalGlowFactor -= 0.1f;
        if (currentGlobalGlowFactor < 0.1f)
            currentGlobalGlowFactor = 0.1f;
        printf("Global Glow Factor: %.2f\n", currentGlobalGlowFactor);
    }
    else if (key == 't') // Metin parlatma yoğunluğunu artır
    {
        currentTextGlowIntensity += 0.2f;
        if (currentTextGlowIntensity > 5.0f)
            currentTextGlowIntensity = 5.0f;
        printf("Text Glow Intensity: %.2f\n", currentTextGlowIntensity);
    }
    else if (key == 'g') // Metin parlatma yoğunluğunu azalt
    {
        currentTextGlowIntensity -= 0.2f;
        if (currentTextGlowIntensity < 0.1f)
            currentTextGlowIntensity = 0.1f;
        printf("Text Glow Intensity: %.2f\n", currentTextGlowIntensity);
    }
}

// Ana fonksiyon
int main(int argc, char **argv)
{
    glutInit(&argc, argv);                                    // GLUT'u başlat
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_ALPHA); // Çift tamponlama, RGB renk, Alfa kanalı
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);          // Pencere boyutunu ayarla
    glutCreateWindow("Psychedelic Matrix: Wake up, Neo...");  // Pencere başlığını ayarla

    // GLEW'i başlat (shader'ları kullanabilmek için gerekli)
    GLenum err = glewInit();
    if (GLEW_OK != err)
    {
        fprintf(stderr, "GLEW Error: %s\n", glewGetErrorString(err));
        return 1;
    }

    initShaders();                        // Shader'ları başlat
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Arka plan rengini siyah yap

    glutDisplayFunc(display);    // Ekran çizim fonksiyonunu ayarla
    glutReshapeFunc(reshape);    // Pencere boyutlandırma fonksiyonunu ayarla
    glutKeyboardFunc(keyboard);  // Klavye fonksiyonunu ayarla
    glutTimerFunc(0, update, 0); // Animasyon döngüsünü başlat

    glutMainLoop(); // GLUT olay döngüsünü başlat
    return 0;
}