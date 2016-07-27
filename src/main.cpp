
#include "base.h"
#include "nes.h"

#if !defined(BASE_PLATFORM_WINDOWS)
#error "SimpleNES requires BASE_PLATFORM_WINDOWS."
#endif

#include "time.h"
#include "gl/gl.h"
#include "glut/glut.h"

using namespace base;
using namespace nes;

#define FAMICOM_VIDEO_WIDTH         (256)
#define FAMICOM_VIDEO_HEIGHT        (240)
#define FAMICOM_SYSTEM_SCALE        (3)

const uint8 g_keymap[] = 
{
    'M', 'N', 'V', 'B', VK_UP, VK_DOWN, VK_LEFT, VK_RIGHT
};

famicom nes_system;
controller gamepad;
uint32 texture = -1;
bool has_valid_rom = false;
uint8 frame_data[FAMICOM_VIDEO_WIDTH * FAMICOM_VIDEO_HEIGHT * 3] = {0};

void _prepare_frame_texture()
{
    if (has_valid_rom)
    {
        nes_system.read_frame_buffer(frame_data + 8 * FAMICOM_VIDEO_WIDTH * 3);
    }
    else
    {
        // If the rom failed to load then render animated static.
        for (uint32 pixel = 0; pixel < FAMICOM_VIDEO_WIDTH * FAMICOM_VIDEO_HEIGHT * 3;)
        {
            uint8 pixel_value = rand() % 255;
            frame_data[pixel++] = pixel_value;
            frame_data[pixel++] = pixel_value;
            frame_data[pixel++] = pixel_value;
        }
    }

    if (-1 == texture)
    {
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, FAMICOM_VIDEO_WIDTH, FAMICOM_VIDEO_HEIGHT, 0,
                     GL_RGB, GL_UNSIGNED_BYTE, frame_data);
    }
    else
    {
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, FAMICOM_VIDEO_WIDTH, FAMICOM_VIDEO_HEIGHT, 
                        GL_RGB, GL_UNSIGNED_BYTE, frame_data);
    }
}

void update_input()
{
    for (uint8 i = 0; i < 8; i++)
    {
        if (GetAsyncKeyState(g_keymap[i]))
        {
            gamepad.set_button(i, true);
        }
        else
        {
            gamepad.set_button(i, false);
        }
    }
}

void render_scene()
{
    if (has_valid_rom)
    {
        update_input();
        nes_system.tick();
    }

    _prepare_frame_texture();

    glClearColor(1, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();

    if (-1 != texture)
    {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, texture);
    }

    glBegin(GL_QUADS);
    glTexCoord2f(0, 1);
	glVertex3f(-1.0f,-1.0f, 0.0f);
    glTexCoord2f(0, 0);
	glVertex3f(-1.0f, 1.0f, 0.0);
    glTexCoord2f(1, 0);
	glVertex3f( 1.0f, 1.0f, 0.0);
    glTexCoord2f(1, 1);
    glVertex3f( 1.0f,-1.0f, 0.0);
	glEnd();

    glutSwapBuffers();
}

void main(int argc, char **argv)
{
    if (argc < 2)
    {
        base_msg("No game rom detected!");
        base_msg("syntax: nes <rom filename>");
    }
    else
    {
        has_valid_rom = base_succeeded(nes_system.insert_rom(argv[1]));
    }

    nes_system.attach_controller(0, &gamepad);

    glutInit(&argc, argv);
    glutInitWindowSize(
        FAMICOM_VIDEO_WIDTH * FAMICOM_SYSTEM_SCALE, 
        FAMICOM_VIDEO_HEIGHT * FAMICOM_SYSTEM_SCALE);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
    glutCreateWindow("SimpleNES Emulator");
    glutDisplayFunc(&render_scene);
    glutIdleFunc(&render_scene);
    glutMainLoop();

    nes_system.eject_rom();
}