#include "hw/compat.h"
#include "srmesh.h"
#include "shared.h"
#include <iostream>
#include "main/types_io.h"
#include "main/types_ops.h"
#include <fstream.h>
#include <iostream.h>

namespace reaper 
{
namespace gfx
{
namespace mesh
{
	using std::cout;
	using std::cin;

	SRMesh::SRMesh(void)
	{
		vertices.clear();
		faces.clear();
		active_vertices.clear();
		active_faces.clear();
	}

int SRMesh::write_file(char* file_name, bool replace)
{

	int vertice_count = vertices.size();
	int face_count = faces.size();

	fstream outFile;

	if(replace)
		outFile.open(file_name, ios::out | ios::binary);
	else
		outFile.open(file_name, ios::out | ios::binary | ios::noreplace);

	if(!outFile)return -1;

	outFile.write( (char*)&vertice_count, sizeof(int));
	outFile.write( (char*)&face_count, sizeof(int));

	// Write vertices
	for(int vi = 0; vi < vertices.size(); vi++)
		outFile.write( (char*)& vertices[vi], sizeof (PMVertex));

	// Write faces
	for(int fi = 0; fi < faces.size(); fi++)
		outFile.write( (char*)& faces[fi], sizeof (PMFace));

	outFile.close();

	return 0;
}

void SRMesh::read_file(char* file_name)
{
	int vi, fi;

	PMVertex* pmv = new PMVertex;
	PMFace* pmf = new PMFace;
	int vertice_count, face_count;

	fstream readFile(file_name, ios::in | ios::binary);

	readFile.read( (char*) &vertice_count, sizeof(int));
	readFile.read( (char*) &face_count, sizeof(int));

	// Read vertices
	for(vi = 0; vi < vertice_count; vi++){
		readFile.read ((char*)pmv, sizeof (PMVertex));
		vertices.push_back( (*pmv) );
	}

	// Read faces
	for(fi = 0; fi < face_count; fi++){
		readFile.read((char*)pmf, sizeof (PMFace));
		faces.push_back( (*pmf) );
	}

	readFile.close();

	// Setup active elements
	for(vi = 0; vi < vertice_count; vi++)if( vertices[vi].is_active)
		add_active_vertex(vi);

	for(fi = 0; fi < face_count; fi++)if ( faces[fi].is_active)
		add_active_face(fi);
	
}

void SRMesh::load(char* file_name)
{
	// Reset the mesh
	vertices.clear();
	faces.clear();
	active_vertices.clear();
	active_faces.clear();

	// Read the stored mesh
	read_file(file_name);
}

void SRMesh::save(char* file_name)
{
	write_file(file_name, true);
}


	void SRMesh::render(void)
	{
		render(NULL);
	}


	void SRMesh::render(PMVertex* pmv)
	{


		glBegin(GL_TRIANGLES);  //tells OpenGL that we're going to start drawing triangles

		lint::iterator li;
		int x, y, z, i, j;
		Point p[3];

		float colorBronzeSpec[4] = { 1.0, 1.0, 0.4, 1.0 };
		glMaterialfv(GL_FRONT, GL_SPECULAR, colorBronzeSpec);
		glMaterialf(GL_FRONT, GL_SHININESS, 70.0);


		for(li = active_faces.begin(); li != active_faces.end(); li++){


			for(int j = 0; j < 3; j++)
				p[j] = vertices[get_active_vertex_relative(faces[*li].vertices[j])].point;
		
			glColor3f(faces[*li].color.x, faces[*li].color.y, faces[*li].color.z);

			float color[4];
			for(int ci = 0; ci < 3; ci++)color[ci] = faces[*li].color[ci]*0.4;
			color[4] = 1.0;
			glMaterialfv(GL_FRONT, GL_DIFFUSE, color);
			glMaterialfv(GL_FRONT, GL_AMBIENT, color);
			glMaterialfv(GL_FRONT, GL_SPECULAR, color);

			Point n = cross( p[1] - p[0], p[2] - p[0]);
			n = n/length(n);

			glNormal3f(n.x, n.y, n.z);
			//glNormal3f(faces[*li].normal.x, faces[*li].normal.y, faces[*li].normal.z);


			for(int j = 0; j < 3; j++)
				glVertex3f(p[j].x, p[j].y, p[j].z);

				//glFlush();
		}

		// Draw collpsing...

		glDisable(GL_LIGHTING);

		if(pmv != NULL){

			for(i = 0; i < 2; i++)if((*pmv).flr[i] != -1){

				if(i == 0)glColor3f(0.0, 1.0, 0.0);
				else glColor3f(1.0, 0.0, 0.0);

				for(j = 0; j < 3; j++){
					x = vertices[get_active_vertex_relative(faces[(*pmv).flr[i]].vertices[j])].point.x;
					y = vertices[get_active_vertex_relative(faces[(*pmv).flr[i]].vertices[j])].point.y;
					z = vertices[get_active_vertex_relative(faces[(*pmv).flr[i]].vertices[j])].point.z;
					glVertex3f(x, y, z);
				}
				
				glColor3f(1.0, 0.0, 1.0);
				Point p = vertices[(*pmv).vu].point;
				glVertex3f(p.x, p.y+2.7, p.z);
				glVertex3f(p.x-1.9, p.y-1.9, p.z);
				glVertex3f(p.x+1.9, p.y-1.9, p.z);
				
			}

			for(i = 0; i < 4; i++)if((*pmv).fn[i] != -1){

				glColor3f(0.0, 1.0, 1.0);

				for(j = 0; j < 3; j++){
					x = vertices[get_active_vertex_relative(faces[(*pmv).fn[i]].vertices[j])].point.x;
					y = vertices[get_active_vertex_relative(faces[(*pmv).fn[i]].vertices[j])].point.y;
					z = vertices[get_active_vertex_relative(faces[(*pmv).fn[i]].vertices[j])].point.z;
					glVertex3f(x, y, z);
				}

				x = vertices[get_active_vertex_relative(faces[(*pmv).fn[i]].vertices[(*pmv).fni[i]])].point.x;
				y = vertices[get_active_vertex_relative(faces[(*pmv).fn[i]].vertices[(*pmv).fni[i]])].point.y;
				z = vertices[get_active_vertex_relative(faces[(*pmv).fn[i]].vertices[(*pmv).fni[i]])].point.z;
				glColor3f(0.0, 0.0, 1.0);
				glVertex3f(x,     y+2.7, z);
				glVertex3f(x-1.9, y-1.9, z);
				glVertex3f(x+1.9, y-1.9, z);

				
			}
			
		}

		glEnd();

		//glDisable(GL_DEPTH);
		//glEnable(GL_DEPTH);

	}

	bool SRMesh::vertex_spawned(int vid)
	{
		int fl = vertices[vid].flr[0];
		int fr = vertices[vid].flr[1];

		if( (fl == -1) && (fr == -1) ) throw MeshException("SRMesh::vertex_spawned: fl and fr both nonexisting.");

		if(fl != -1)return faces[fl].is_active;
		else return faces[fr].is_active;
	}

	void SRMesh::add_active_vertex(int vind)
	{
		if(vind == -1)return;
		if(vind > vertices.size()){
			throw MeshException("SRMesh::add_active_vertex invalid index");
		}
		vertices[vind].active = active_vertices.insert(active_vertices.end(), vind);
		vertices[vind].is_active = true;
	}

	void SRMesh::add_active_vertex_at(int vind, std::list<int>::iterator dst)
	{
		if(vind == -1)return;
		if(vind > vertices.size()){
			throw MeshException("SRMesh::add_active_vertex invalid index");
		}
		vertices[vind].active = active_vertices.insert(dst, vind);
		vertices[vind].is_active = true;
	}

	void SRMesh::add_active_face(int find)
	{
		if(find == -1)return;
		faces[find].active = active_faces.insert(active_faces.end(), find);
		faces[find].is_active = true;
	}

	void SRMesh::remove_active_vertex(int vind)
	{
		if(!vertices[vind].is_active)throw MeshException("cannot remove inactive");
		if(vind == -1)return;
		active_vertices.erase(vertices[vind].active);
		vertices[vind].is_active = false;
	}

	void SRMesh::remove_active_face(int find)
	{
		if(find == -1)return;
		active_faces.erase(faces[find].active);
		faces[find].is_active = false;
	}

	int SRMesh::get_active_vertex_relative(int i) const
	{
		if(i == -1)throw MeshException("get_active_vertex_relative: vertex lacks parent.");
		if(vertices[i].is_active)return i;
		return get_active_vertex_relative(vertices[i].parent);
	}

	int SRMesh::get_face_vertex(int fid, int vid) const
	{
		return get_active_vertex_relative(faces[fid].vertices[vid]);
	}

	void SRMesh::split_children_at(int vind, lint::iterator& dst)
	{
		PMVertex& pmv = vertices[vind];

		// Activate children
		add_active_vertex_at(pmv.vt, dst);
		add_active_vertex_at(pmv.vu, dst);

		// Activate fl/fr
		add_active_face(pmv.flr[0]);
		add_active_face(pmv.flr[1]);

		// Deactivate v
		remove_active_vertex(vind);

		// Fix neighbouring faces
		for(int i = 0; i < 4; i++)if(pmv.fn[i] != -1)
			faces[pmv.fn[i]].nf[pmv.fni[i]] = pmv.flr[i<2?0:1];
	}

	void SRMesh::split(int vind)
	{
		PMVertex& pmv = vertices[vind];

		// Activate children
		add_active_vertex(pmv.vt);
		add_active_vertex(pmv.vu);

		// Activate fl/fr
		add_active_face(pmv.flr[0]);
		add_active_face(pmv.flr[1]);

		// Deactivate v
		remove_active_vertex(vind);

		// Fix neighbouring faces
		for(int i = 0; i < 4; i++)if(pmv.fn[i] != -1)
			faces[pmv.fn[i]].nf[pmv.fni[i]] = pmv.flr[i<2?0:1];

	}

	void SRMesh::col_parent_at(int vind, lint::iterator& dst)
	{
		PMVertex& pmv = vertices[vind];

		//Deactivate children
		remove_active_vertex(pmv.vt);
		remove_active_vertex(pmv.vu);

		//Deactivate fl/fr
		remove_active_face(pmv.flr[0]);
		remove_active_face(pmv.flr[1]);

		//Activate v
		add_active_vertex_at(vind, dst);

		//Fix neighbouring faces
		if(pmv.fn[0] != -1)faces[pmv.fn[0]].nf[pmv.fni[0]] = pmv.fn[1];
		if(pmv.fn[1] != -1)faces[pmv.fn[1]].nf[pmv.fni[1]] = pmv.fn[0];
		if(pmv.fn[2] != -1)faces[pmv.fn[2]].nf[pmv.fni[2]] = pmv.fn[3];
		if(pmv.fn[3] != -1)faces[pmv.fn[3]].nf[pmv.fni[3]] = pmv.fn[2];

	}


	void SRMesh::col(int vind)
	{
		PMVertex& pmv = vertices[vind];

		//Deactivate children
		remove_active_vertex(pmv.vt);
		remove_active_vertex(pmv.vu);

		//Deactivate fl/fr
		remove_active_face(pmv.flr[0]);
		remove_active_face(pmv.flr[1]);

		//Activate v
		add_active_vertex(vind);

		//Fix neighbouring faces
		if(pmv.fn[0] != -1)faces[pmv.fn[0]].nf[pmv.fni[0]] = pmv.fn[1];
		if(pmv.fn[1] != -1)faces[pmv.fn[1]].nf[pmv.fni[1]] = pmv.fn[0];
		if(pmv.fn[2] != -1)faces[pmv.fn[2]].nf[pmv.fni[2]] = pmv.fn[3];
		if(pmv.fn[3] != -1)faces[pmv.fn[3]].nf[pmv.fni[3]] = pmv.fn[2];

	}

	void SRMesh::switch_face_neighbour(int src, int old_dest, int new_dest, char* caller)
	{
		if(src == -1)return;

		if(src == new_dest)
			throw MeshException("SRMesh::switch_face_neighbour: Cannot be my own neighbour!\n");
	
		int i;

		for(i = 0; i < 3; i++){
			if( (faces[src].nf[i] == new_dest) && (new_dest != -1)){
				throw MeshException("SRMesh::switch_face_neighbour: new_dest already exist");
			}
		}

		//cout << "\nSWITCH:\n\n\nsrc: " << src << "\told_dest: " << old_dest << "\tnew_dest: " << new_dest << "\n";	
		//int chk = 0;
		bool chk = false;

		for(i = 0; i < 3; i++){
		//	cout << "facing: " << i << " is " << faces[src].nf[i] << ", ";
			if(faces[src].nf[i] == old_dest){
				faces[src].nf[i] = new_dest;
				//chk++;
				chk = true;
			}
		}

		if(chk)return;

		cout << "\nSWITCH:\n\n\nsrc: " << src << "\told_dest: " << old_dest << "\tnew_dest: " << new_dest << "\n";	

		faces[src].report(src);

		cout << "\n" << caller << "\n";
		throw MeshException("SRMesh::switch_face_neigbour: no matching neigbour.");
	}


}
}
}