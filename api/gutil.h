struct COLOR {
    GLfloat r;
    GLfloat g;
    GLfloat b;
    GLfloat a;
};

extern void HLStoRGB( double H, double L, double S, COLOR& c);

extern float frand();

extern void drawSphere(GLfloat* pos, GLfloat rad);
extern void drawCylinder(bool vertical, GLfloat* pos, GLfloat len, GLfloat rad);

#define TEXT_LEFT       0
#define TEXT_CENTER     1
#define TEXT_RIGHT      2
extern void draw_text_line(
    GLfloat* pos, GLfloat height, GLfloat width, char *text,
    int justify=TEXT_LEFT
);
extern void draw_text(
    GLfloat* pos, GLfloat height, GLfloat width, GLfloat spacing, char *text
);
extern GLfloat text_width(char* text);
extern void draw_text_panel(
    GLfloat* _pos, GLfloat* size, GLfloat margin, COLOR color,
    GLfloat char_height, GLfloat line_width, GLfloat line_spacing,
    char* text);

extern void mode_shaded(GLfloat*);
extern void mode_unshaded();
extern void mode_lines();

// draw a progress bar as an opaque cylinder within a translucent cylinder
//
class PROGRESS {
    GLfloat pos[3];
    GLfloat color[4], inner_color[4];
    GLfloat len, rad, inner_rad;
public:
    PROGRESS(GLfloat* pos, GLfloat len, GLfloat diam, GLfloat inner, GLfloat* c, GLfloat* ic);
    void draw(float);
};

// draw a graph as a ribbon
//
class GRAPH_2D {
    float pos[3], size[3];
    float color[4], tick_color[4];
    float *data, dmax;
    int len;
    void draw_x(int);
    void draw_y(int);
public:
    GRAPH_2D(float* pos, float* size, float* color, float* tick_color);
    void draw(float* data, int len);
    void add_tick(float x, float yfrac);
};

extern int read_ppm(char* name, int& w, int& h, unsigned char** arrayp);

extern void init_texture(char* filename);
extern void draw_texture(float* pos, float* size);