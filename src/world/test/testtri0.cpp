

#include "hw/compat.h"

#include "hw/debug.h"
#include "hw/gl.h"
#include "hw/gfx.h"
#include "hw/event.h"
#include "misc/font.h"
#include "hw/hwtime.h"
#include "world/world.h"
#include "object/base.h"

#include "misc/test_main.h"

using namespace reaper;


void draw_it(world::WorldRef w, world::Cylinder cyl)
{
	static int flip = 0;
//	static Cylinder cyl(Point2D(10, 10), 5);
	static GLUquadricObj* gl_cyl = gluNewQuadric();
//	hw::event::EventProxy ep = hw::event::EventSystem::get_ref(0);

	::glEnable(GL_DEPTH_TEST);

	glPushMatrix();
	glRotatef(-60, 1, 0, 0);
	world::tri_iterator c, e = w->end_tri();
	glTranslatef(-10, 0, -10);
	glColor3f(0, 1, 0);
	glBegin(GL_TRIANGLES);
	for (c = w->begin_tri(); c != e; c++) {
		glVertex3fv(c->a.get());
		glVertex3fv(c->b.get());
		glVertex3fv(c->c.get());
	}
	glEnd();

	glTranslatef(0, -0.01, 0);
	glColor3f(1, 0, 0);
	glBegin(GL_TRIANGLES);
	for (c = w->find_tri(cyl); c != e; c++) {
		glVertex3fv(c->a.get());
		glVertex3fv(c->b.get());
		glVertex3fv(c->c.get());
	}

	glEnd();

	glColor4f(0, 0, 0.5, 0.5);
	glTranslatef(cyl.p.x, 10, cyl.p.y);
	glRotatef(90, 1, 0, 0);
	gluCylinder(gl_cyl, cyl.r, cyl.r, 20, 50, 50);
	glPopMatrix();

	glColor3f(1,1,1);
	char buf[100];
	snprintf(buf, 100, "x: %f  y: %f %d", cyl.p.x, cyl.p.y, flip);
	flip ^= 1;
	::glDisable(GL_DEPTH_TEST);
	gfx::font::glPutStr(0, 0, buf);
}

int test_main()
{
	hw::gfx::Gfx gx;
	debug::ExitFail ef(1);
	hw::event::EventSystem es(gx);
	hw::event::EventProxy ep = hw::event::EventSystem::get_ref(0);
	world::WorldRef w = world::World::create("test_level");

	float width = 0.5;
	for (float i = 0; i < 20; i+=width) {
		for (float j = 0; j < 20; j+=width) {
			w->add_object(new world::Triangle(
						Point(i, 1, j),
						Point(i+width, 1, j),
						Point(i+width, 1, j+width)));
		}
	}

	glViewport(0,0,gx.current_mode().width, gx.current_mode().height);		// hela fönstret
	glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
	gluPerspective(70,1,1,1000);

	glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        gluLookAt(0,-20,0,0,0,0,0,0,1);

	glClearColor(0,0,0,0);
	::glEnable(GL_BLEND);
	::glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	world::Cylinder cyl(Point2D(10, 10), 5);
	float step = 0.2;
	hw::time::TimeSpan start = hw::time::get_time();
	while (true) {
		if (ep.key(hw::event::id::Escape))
			break;
		if (ep.key(hw::event::id::Right))
			cyl.p.x+=step;
		if (ep.key(hw::event::id::Left))
			cyl.p.x-=step;
		if (ep.key(hw::event::id::Up))
			cyl.p.y+=step;
		if (ep.key(hw::event::id::Down))
			cyl.p.y-=step;
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		draw_it(w, cyl);
		gx.update_screen();
		hw::time::msleep(10);
		if (exit_now())
			break;
	}

	ef.disable();
	return 0;
}

