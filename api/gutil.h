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