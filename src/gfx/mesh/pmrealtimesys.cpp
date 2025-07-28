#include "hw/compat.h"
#include "pmrealtimesys.h"
#include "shared.h"
#include "glhelpers.h"

#include "main/types_ops.h"
#include "world/query_obj.h"
#include "world/geom_helper.h"
#include "misc/font.h"
#include <iostream>

namespace reaper 
{
namespace gfx
{
namespace mesh
{


	using std::cout;
	using std::cin;

	void PMRealTimeSys::render(void)
	{
		Timer tim;
		static int i;
		static double qrta;
		static int qrt[100];// = {0, 0, 0, 0, 0, 0, 0 , 0, 0, 0};
		static int qrti = 0;
		static int qrt_max = 0;
		

		if(qrefine_cnt > qrt_max) qrt_max = qrefine_cnt;

		qrt[qrti] = qrefine_cnt;
		qrti++;
		if(qrti == 100)qrti = 0;

		qrta = 0.0;
		for(i = 0; i < 100; i++)qrta += (float) qrt[i];
		qrta = qrta / 100.0;

		static float afps = 0.0;
		afps += frame_time;
		static int cnt = 0;
		cnt++;
		float camang = 3.14*reaper::world::angle(cam.front, Point(1.0, 0.0, 0.0)) / 180.0;
		if(dot(cam.front, Point(0.0, 1.0, 0.0)) < 0)camang = -camang;


		//cout << "camang: " << (camang*180/3.14) << "\n";
		float len = 50.0;
		float da = (3.14*cam.horiz_fov/180.0) / 2.0;
		float x0 = len * cos(camang + da);
		float y0 = len * sin(camang + da);
		float x1 = len * cos(camang - da);
		float y1 = len * sin(camang - da);


		tim.start();

		mesh.render();

		tim.stop();		

		glDisable(GL_DEPTH_TEST);
		glDisable(GL_LIGHTING);
		glBlendFunc(GL_ONE_MINUS_DST_ALPHA,GL_DST_ALPHA);
		glEnable (GL_BLEND);

		glBegin(GL_LINES);

			float dx = cos(cam.horiz_fov) * 10;
			float dy = sin(cam.horiz_fov) * 10;


			glColor4f(1.0, 1.0, 1.0, 0.2);
			glVertex3f(cam.pos.x + x0, cam.pos.y + y0, cam.pos.z);
			glVertex3f(cam.pos.x, cam.pos.y, cam.pos.z);
			glVertex3f(cam.pos.x + x1, cam.pos.y + y1, cam.pos.z);
			glVertex3f(cam.pos.x, cam.pos.y, cam.pos.z);

		glEnd();

					
		glColor3f(1.0, 1.0, 1.0);
		char dt[256];
		sprintf(dt, "face count: %d", mesh.active_faces.size());
		output(20.0, 20.0, dt);
		sprintf(dt, "fps: %f", 1.0/(frame_time / (float) render_cnt));
		output(20.0, 30.0, dt);
		sprintf(dt, "qrefine cnt: %d ", (int) qrta);
		output(20.0, 40.0, dt);
		sprintf(dt, "qrefine max: %d ", (int) qrt_max);
		output(20.0, 50.0, dt);
		sprintf(dt, "recursion: %d ", (int) stack_depth);
		output(20.0, 60.0, dt);

		glEnable(GL_LIGHTING);
		glEnable(GL_DEPTH_TEST);


	}

	void PMRealTimeSys::set_cam(Point origin, Point dir, float fov)
	{
		cam.pos = origin;
		cam.front = norm(dir);
		cam.horiz_fov = fov;
		cam_fov_cos = cos( (cam.horiz_fov*3.14/180.0) / 2.0);
	}


	void PMRealTimeSys::vsplit(int vi, lint::iterator* li, lint::iterator& tracer)
	{

		split_cnt++;
	
		if(li == NULL)mesh.split(vi);
		else {
			mesh.split_children_at(vi, *li);

		}
	}

	void PMRealTimeSys::backtrace_ecol(int vi, lint::iterator& li)
	{

		if(mesh.vertices[vi].is_active)throw MeshException("backtrace_ecol");

		while( ((*li == mesh.vertices[vi].vt) || (*li == mesh.vertices[vi].vu)) && li != mesh.active_vertices.end()) li++; 

		mesh.col_parent_at(vi, li);

	}

	
	void PMRealTimeSys::ecol(int vi, lint::iterator& li)
	{
		bool bgn = false;

		if(li == mesh.active_vertices.begin()){
			mesh.col(vi);
			bgn = true;
		} else {
			li--;
			if( (*li == mesh.vertices[vi].vt) || (*li == mesh.vertices[vi].vu) )
				if(li == mesh.active_vertices.begin()) bgn = true;
				else li--;
				mesh.col(vi);
			li++;
		}

		if(bgn)li = mesh.active_vertices.begin();

		// Reconsider vl/vr 
		int vl = (mesh.vertices[vi].flr[0] == -1)?-1:mesh.get_active_vertex_relative(mesh.faces[mesh.vertices[vi].flr[0]].vertices[2]);
		int vr = (mesh.vertices[vi].flr[1] == -1)?-1:mesh.get_active_vertex_relative(mesh.faces[mesh.vertices[vi].flr[1]].vertices[1]);

		if(vl != -1){relocate_vertex(vl, li);}
		if(vr != -1){relocate_vertex(vr, li);}
	}

	void PMRealTimeSys::relocate_vertex(int vi, lint::iterator& li)
	{
		if(!mesh.vertices[vi].is_active)throw MeshException("relocate_vertex::cannot relocate inactive vertex");

		if(li == mesh.vertices[vi].active)return;

		mesh.remove_active_vertex(vi);
		lint::iterator tli = li;
		tli++;
		mesh.add_active_vertex_at(vi, tli);
	}
		
	void PMRealTimeSys::fix_restore(lint::iterator& li, int vi)
	{
		while( (li != mesh.active_vertices.end()) && ((*li == vi) || (*li == mesh.vertices[vi].vu) || (*li == mesh.vertices[vi].vu)) )li++;
	}

	void PMRealTimeSys::force_vsplit(int vi, lint::iterator& li)
	{

		lint::iterator restore = li;
		lint::iterator tracer = li;

		restore++;

		int fwd = 0;
		
		vint vstack;
		vint back_trace;

		vstack.push_back(vi);

		while(!vstack.empty()){

			if(vstack.size() > stack_depth) stack_depth = vstack.size();

			int vind = vstack.back();
			PMVertex& v = mesh.vertices[vind];

			//cout << "vstack.size: " << vstack.size() << "\n";
			//cin.get();
			// already taken care of
			if(v.has_children() && mesh.vertex_spawned(vind)) {
				//cout << " " << vind << " already taken care of \n";
				vstack.pop_back();
			// inactive, split parent
			} else if(!v.is_active) {
				//cout << " " << vind << " inactive, split parent (" << v.parent << ")\n";
				vstack.push_back(v.parent);
			// geometry ok - split
			} else if(vsplit_legal(v)){
				//back_trace.push_back(vind);
				//fwd++;
				if(*restore == vind)restore++;
				//cout << " " << vind << " geometry ok - split to " << v.vu << "/" << v.vt << " open: " << v.flr[0] << "/" << v.flr[1] << "\n";
				//cout << "$" << vind << mesh.vertices[19327].is_active << "\t";
				lint::iterator tli = li;
				if(*li == vind){
					//cout << " match ";
					tli++;
					if(li == mesh.active_vertices.begin()){
						vstack.pop_back();
						vsplit(vind, &restore, tracer);
						li = mesh.active_vertices.begin();
					} else {
						li--;
						vstack.pop_back();
						vsplit(vind, &restore, tracer);
						li++;
					}
				} else {
					//cout << " nocheck ";
					tli++;
					vstack.pop_back();
					vsplit(vind, &restore, tracer);

				}
				
			// geometry NOT ok - split to create necessary faces
			} else {
				//cout << " " << vind << " geometry NOT ok - split to create necessary faces: ";
				for(int i = 0; i < 4; i++) 
					if((v.fn[i] != -1) && !mesh.faces[v.fn[i]].is_active){
						int tmp = mesh.vertices[mesh.faces[v.fn[i]].vertices[0]].parent;
						vstack.push_back(tmp);
						//cout << tmp << "(" << v.fn[i] << ") ";
					}
				//cout << "\n";


			}

		//	cin.get();

			// Need to clear the stack at some point or it may grow
			// inefficiently deep
			if(vstack.size() > max_stack_depth){

				vstack.clear();
				
				//  for(int i = back_trace.size() - 1; i >= 0; i--){
				//	if(mesh.vertices[back_trace[i]].is_active)throw MeshException("backtracing..");
				//	backtrace_ecol(back_trace[i],  li);

				restore--;
				li = restore;
			}
			
			
				
		}
	}

	void PMRealTimeSys::refine(void)
	{
		// for each {v | v in ActiveVertices}
		// if v.vt and qrefine(v)
		//		force_vsplit(v)
		// else if v.parent and ecol_legal(v.parent) and not qrefine(v.parent)
		//		ecol(v.parent)  ((and reconsider some vertices)) <- WHICH???
		//qtab.clear();

		static bool cond;
		cond = false;//render_cnt > 100;
		qrefine_cnt = split_cnt = 0;
		static int cnt, scnt, ecnt;
		cnt = scnt = ecnt = 0;

		//static int VMAX = 100000;

		//for(int i = 0; i < VMAX; i++)access[i] = 0;

		ref_timer.start();

		lint::iterator li, tmp, lj;



		li = mesh.active_vertices.begin();

		int smax = 0;

		//cout << mesh.active_vertices.size() << "/";

		while(li !=  mesh.active_vertices.end()){

			static int fcnt;
			fcnt = mesh.active_faces.size();


			// cout << mesh.active_vertices.size() << "(" << *li << ")\t";

			if(mesh.active_vertices.size() > smax) smax = mesh.active_vertices.size();
			cnt++;

			

			//if(cond)cout << "\nActive vertices: " << mesh.active_vertices.size();
			//if(cond)cout << "refine active vertices: ";
			//for(lj = mesh.active_vertices.begin(); lj != mesh.active_vertices.end(); lj++)cout << *lj << " ";
			//cout << "\n";

			if(false){
				lj = mesh.active_vertices.begin();
				for(int i = 0; i < 17; i++){
					cout << *lj << ", ";
					if(lj != mesh.active_vertices.end())lj++;
				}
				//cout << "\nrefine considering vertex: " << *li << "\t";
			}

			


			PMVertex& v = mesh.vertices[*li];

			//cout << "p:" << v.has_parent() << " ";

			//if(v.has_parent())cout << "q:" << !qrefine(mesh.vertices[v.parent]) << " ";

			if(  v.has_children() && qrefine(*li)){
				//cout << "vsplit..." << v.vt << " / " << v.vu;

				scnt++;
				int backup = *li;
				force_vsplit(*li, li);	
				if(li != mesh.active_vertices.end())li++;
				if(cond)cin.get();
			} else if((fcnt > lower_bound) && v.has_parent() && ecol_legal(mesh.vertices[v.parent]) && !qrefine(v.parent)) {
				//if(true)cout << "ecol..." << mesh.vertices[v.parent].vu << " / " << mesh.vertices[v.parent].vt;
				//cout << "col ";
				ecnt++;
				//if(access[*li] == 1)access[*li] = 2;;
				ecol(v.parent, li);
				if(cond)cin.get();
			} else {
				//if(true)cout << "take no action...";  
				if(li != mesh.active_vertices.end())li++;
			}
		 	//cout << "done\n";
			//cout << "\n";


		
		}

		//cin.get();
		/*
		int mac = 0;
		for(int i = 0; i < VMAX; i++)if(access[i] == 2){
			mac++;
			//cout << "\naccess[" << i << "] accessed: " << access[i] << "\n";
			//cin.get();
		}
		*/
		//cout << cnt << "/" << mac << "/" << ecnt << "/" << split_cnt << "\t";
		ref_timer.stop();

	}

	inline bool PMRealTimeSys::qrefine(int vi)
	{
		/*
		int hval = qtab.lookup(vi);
		if(hval != -1){
			return hval;
		}
		*/

		PMVertex& v = mesh.vertices[vi];
		qrefine_cnt++;

		if(outside_view_frustum(v))	return false;

		if(oriented_away(v)) return false;

		if(screen_space_error(v) < tau) return false;
		
		return true;
	}

	inline bool PMRealTimeSys::outside_view_frustum(PMVertex& v)
	{
		static Vector p; 
		static float dotp;
		static float dotpn;
		static float ret;

		p = v.point - cam.pos;

		dotp = (cam.front.x*p.x + cam.front.y*p.y + cam.front.z*p.z);
		if(dotp < 0.0)return true;

		dotpn = dotp / (length(p));		
		ret = dotpn < cam_fov_cos;

		return ret;
	}

	inline bool PMRealTimeSys::oriented_away(PMVertex& v){

		return false;
		
	}


	inline float PMRealTimeSys::screen_space_error(PMVertex& v)
	{
		Point u = (v.point - cam.pos);

		float len = length(u);
		return v.flr_area/(len*len);

	}

	inline bool PMRealTimeSys::ecol_legal(PMVertex& v)
	{
		
		//return (faces_aligned(v.flr[0], v.fn[0]));
		//cout << "ecol legal in action...";

		//if(v.parent == -1)return false;

		// Sibling active?
		bool b0a = mesh.vertices[v.vt].is_active;
		bool b0b = mesh.vertices[v.vu].is_active;

		bool b1a = (v.fn[0] == -1) || ( mesh.faces[v.fn[0]].is_active );
		bool b1b = (v.fn[1] == -1) || ( mesh.faces[v.fn[1]].is_active );
		bool b1c = (v.fn[2] == -1) || ( mesh.faces[v.fn[2]].is_active );
		bool b1d = (v.fn[3] == -1) || ( mesh.faces[v.fn[3]].is_active );
		bool b1 = b1a && b1b && b1c && b1d;

		// Matching neighbours?		
		bool b0c = (v.flr[0] == -1) || (mesh.faces[v.flr[0]].nf[1] == v.fn[0]);
		bool b0d = (v.flr[0] == -1) || (mesh.faces[v.flr[0]].nf[0] == v.fn[1]);
		bool b0e = (v.flr[1] == -1) || (mesh.faces[v.flr[1]].nf[2] == v.fn[2]);
		bool b0f = (v.flr[1] == -1) || (mesh.faces[v.flr[1]].nf[0] == v.fn[3]);

		
		bool b0 = b0a && b0b && b0c && b0d && b0e && b0f;

		//cout << "\n" << b0a << b0b << b0c << b0d << b0e << b0f << "\n";
		//cout << b0 << b1 << "\n";

		return b0 && b1;
	}


	inline bool PMRealTimeSys::vsplit_legal(PMVertex& v)
	{
		bool b0 = ((v.fn[0] == -1) || (mesh.faces[v.fn[0]]).is_active) &&
				  ((v.fn[1] == -1) || (mesh.faces[v.fn[1]]).is_active) &&
				  ((v.fn[2] == -1) || (mesh.faces[v.fn[2]]).is_active) &&
				  ((v.fn[3] == -1) || (mesh.faces[v.fn[3]]).is_active);

		return b0;

	}


	void PMRealTimeSys::report(bool verbose)
	{
		cout << "\n";
		cout << "---------------------------\n";
		cout << "| PMRealTimeSys reporting |" << "\n";
		cout << "---------------------------" << "\n";
		cout << "Vertex count:\t " << mesh.vertices.size() << "\n";
		cout << "Faces count:\t" << mesh.faces.size() << "\n";
		cout << "\n";
		cout << "Active Vertex count:\t " << mesh.active_vertices.size() << "\n";
		cout << "Active Faces count:\t" << mesh.active_faces.size() << "\n";
		cout << "\n";

		if(verbose){
			cout << "#\tactive\tvt\tvu\tparent\tx\ty\tz\tfl\tfr\n";
			for(int i = 0; i < mesh.vertices.size(); i++){
				cout << i << "\t" << mesh.vertices[i].is_active << "\t" << mesh.vertices[i].vt << "\t" << mesh.vertices[i].vu << "\t" << mesh.vertices[i].parent << "\t";
				cout << mesh.vertices[i].point.x << "\t";
				cout << mesh.vertices[i].point.y << "\t";
				cout << mesh.vertices[i].point.z << "\t";
				cout << mesh.vertices[i].flr[0] << "\t";
				cout << mesh.vertices[i].flr[1] << "\n";
			}
		}


		
	}


}
}
}
