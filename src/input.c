#include <stdbool.h>
#include <allegro5/allegro.h>

#include "mathlib.h"
#include "camera.h"
#include "input.h"

static bool wireframe = false;

bool handle_input(ALLEGRO_EVENT_QUEUE *ev_queue, Camera *cam)
{
	ALLEGRO_EVENT ev;
	ALLEGRO_MOUSE_STATE state;

	while (al_get_next_event(ev_queue, &ev))
	{
		switch (ev.type)
		{
		case ALLEGRO_EVENT_DISPLAY_CLOSE:
			return false;
			break;
		case ALLEGRO_EVENT_MOUSE_BUTTON_UP:
			//al_show_mouse_cursor(dpy);
			//al_ungrab_mouse();
			break;
		case ALLEGRO_EVENT_MOUSE_BUTTON_DOWN:
			//al_hide_mouse_cursor(dpy);
			//al_grab_mouse(dpy);
			break;
		case ALLEGRO_EVENT_MOUSE_AXES:
			if (ev.mouse.dz != 0)
				cam_dolly(cam, ev.mouse.dz);
			al_get_mouse_state(&state);

			/* The y coordinate needs to be inverted because OpenGL has
			 * the origin in the lower-left and Allegro the upper-left */
			if (state.buttons & 1)
				cam_orbit(cam, ev.mouse.dx, -ev.mouse.dy);
			else if (state.buttons & 2)
				cam_rotate(cam, ev.mouse.dx, -ev.mouse.dy);
			break;
		case ALLEGRO_EVENT_KEY_CHAR:
			if (ev.keyboard.unichar == 'w')
			{
				wireframe = !wireframe;
				if (wireframe)
				{
					glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
					glDisable(GL_CULL_FACE);
				}
				else
				{
					glPolygonMode(GL_FRONT, GL_FILL);
					glEnable(GL_CULL_FACE);
				}
			} else if (ev.keyboard.unichar == 'c')
			{
				Vec3 upv = quat_transform(cam->orientation, (Vec3){0, 1, 0});
				cam_lookat(cam, cam->position, cam->target, upv);
			}
			break;
		case ALLEGRO_EVENT_DISPLAY_RESIZE:
			cam->width = ev.display.width;
			cam->height = ev.display.height;
			glmLoadIdentity(glmProjectionMatrix);
			cam_projection_matrix(cam, glmProjectionMatrix);
			glViewport(cam->left, cam->bottom, cam->width, cam->height);
		default:
			break;
		}
	}

	return true;
}
