#include "bsp/Node.h"

#include "bsp/Sector.h"
#include "graphics/Mesh.h"
#include "graphics/MeshVBO.h"
#include "graphics/Frustum.h"
#include "engine.h"
#include "object/ObjectMesh.h"

constexpr const int TRIANGLES_PER_NODE = 1024;

Node::Node() : left(NULL), right(NULL), object(NULL) {
	min = vec3(1000000, 1000000, 1000000);
	max = vec3(-1000000, -1000000, -1000000);
	center = vec3(0, 0, 0);
	radius = 1000000;
}

Node::~Node() {
	if (left) delete left;
	if (right) delete right;
	if (object) {
		delete object->mesh;
		delete object;
	}
}

void Node::create(Mesh* mesh) {
	min = mesh->getMin();
	max = mesh->getMax();
	center = mesh->getCenter();
	radius = mesh->getRadius();

	int num_vertex = 0;
	for (int i = 0; i < mesh->getNumSurfaces(); i++) num_vertex += mesh->getNumVertex(i);
	if (num_vertex / 3 > TRIANGLES_PER_NODE) {
		vec4 plane;
		vec3 size = max - min;
		if (size.x > size.y) {	// find clip plane
			if (size.x > size.z) plane = vec4(1, 0, 0, 1);
			else plane = vec4(0, 0, 1, 1);
		}
		else {
			if (size.y > size.z) plane = vec4(0, 1, 0, 1);
			else plane = vec4(0, 0, 1, 1);
		}
		vec3 center(0, 0, 0);
		int num_vertex = 0;	// find center of node
		for (int i = 0; i < mesh->getNumSurfaces(); i++) {
			Mesh::Vertex* vertex = mesh->getVertex(i);
			for (int j = 0; j < mesh->getNumVertex(i); j++) {
				center += vertex[j].xyz;
				num_vertex++;
			}
		}
		center /= (float)num_vertex;
		plane.w = -(plane * vec4(center, 1));
		Mesh* left_mesh = new Mesh();
		Mesh* right_mesh = new Mesh();
		for (int i = 0; i < mesh->getNumSurfaces(); i++) {
			int num_vertex = mesh->getNumVertex(i);
			Mesh::Vertex* vertex = mesh->getVertex(i);
			int left_mesh_num_vertex = 0;
			Mesh::Vertex* left_mesh_vertex = new Mesh::Vertex[num_vertex * 2];
			int right_mesh_num_vertex = 0;
			Mesh::Vertex* right_mesh_vertex = new Mesh::Vertex[num_vertex * 2];
			for (int j = 0; j < num_vertex; j += 3) {
				/*int pos = 0;
				if(plane * vec4(vertex[j + 0].xyz,1) > 0.0) pos++;
				if(plane * vec4(vertex[j + 1].xyz,1) > 0.0) pos++;
				if(plane * vec4(vertex[j + 2].xyz,1) > 0.0) pos++;
				if(pos > 1) {
					left_mesh_vertex[left_mesh_num_vertex++] = vertex[j + 0];
					left_mesh_vertex[left_mesh_num_vertex++] = vertex[j + 1];
					left_mesh_vertex[left_mesh_num_vertex++] = vertex[j + 2];
				} else {
					right_mesh_vertex[right_mesh_num_vertex++] = vertex[j + 0];
					right_mesh_vertex[right_mesh_num_vertex++] = vertex[j + 1];
					right_mesh_vertex[right_mesh_num_vertex++] = vertex[j + 2];
				}*/
				int num_left = 0;
				int num_right = 0;
				Mesh::Vertex* left = &left_mesh_vertex[left_mesh_num_vertex];
				Mesh::Vertex* right = &right_mesh_vertex[right_mesh_num_vertex];
				float cur_dot = plane * vec4(vertex[j + 0].xyz, 1);
				for (int cur = 0; cur < 3; cur++) {
					int next = (cur + 1) % 3;
					float next_dot = plane * vec4(vertex[j + next].xyz, 1);
					if (cur_dot <= 0.0) left[num_left++] = vertex[j + cur];
					if (cur_dot > 0.0) right[num_right++] = vertex[j + cur];
					if ((cur_dot <= 0.0) != (next_dot <= 0.0)) {
						float k = -cur_dot / (next_dot - cur_dot);
						Mesh::Vertex v;
						v.xyz = vertex[j + cur].xyz * (1.0f - k) + vertex[j + next].xyz * k;
						v.normal = vertex[j + cur].normal * (1.0f - k) + vertex[j + next].normal * k;
						v.tangent = vertex[j + cur].tangent * (1.0f - k) + vertex[j + next].tangent * k;
						v.binormal = vertex[j + cur].binormal * (1.0f - k) + vertex[j + next].binormal * k;
						v.texcoord = vertex[j + cur].texcoord * (1.0f - k) + vertex[j + next].texcoord * k;
						left[num_left++] = v;
						right[num_right++] = v;
					}
					cur_dot = next_dot;
				}

				if (num_left == 3) left_mesh_num_vertex += 3;
				else if (num_left == 4) {
					left[4] = left[0];
					left[5] = left[2];
					left_mesh_num_vertex += 6;
				}

				if (num_right == 3) right_mesh_num_vertex += 3;
				else if (num_right == 4) {
					right[4] = right[0];
					right[5] = right[2];
					right_mesh_num_vertex += 6;
				}
			}
			if (left_mesh_num_vertex > 0) left_mesh->addSurface(mesh->getSurfaceName(i), left_mesh_vertex, left_mesh_num_vertex);
			if (right_mesh_num_vertex > 0) right_mesh->addSurface(mesh->getSurfaceName(i), right_mesh_vertex, right_mesh_num_vertex);
			delete[] right_mesh_vertex;
			delete[] left_mesh_vertex;
		}
		int left_mesh_num_vertex = 0;
		for (int i = 0; i < left_mesh->getNumSurfaces(); i++) left_mesh_num_vertex += left_mesh->getNumVertex(i);
		int right_mesh_num_vertex = 0;
		for (int i = 0; i < right_mesh->getNumSurfaces(); i++) right_mesh_num_vertex += right_mesh->getNumVertex(i);
		if (left_mesh_num_vertex > 0 && right_mesh_num_vertex > 0) {
			left_mesh->calculate_bounds();
			left = new Node();
			left->create(left_mesh);
			right_mesh->calculate_bounds();
			right = new Node();
			right->create(right_mesh);
		}
		else {
			mesh->create_shadow_volumes();
			mesh->create_triangle_strips();
			object = new ObjectMesh(new MeshVBO(mesh));
			delete right_mesh;
			delete left_mesh;
		}
	}
	else {
		mesh->create_shadow_volumes();
		mesh->create_triangle_strips();
		object = new ObjectMesh(new MeshVBO(mesh));
	}
	delete mesh;
}


void Node::load(FILE* file) {
	int header;
	fread(&header, sizeof(int), 1, file);
	if (header == 0x7fffffff) {
		fread(&min, sizeof(vec3), 1, file);
		fread(&max, sizeof(vec3), 1, file);
		fread(&center, sizeof(vec3), 1, file);
		fread(&radius, sizeof(float), 1, file);
		left = new Node();
		left->load(file);
		right = new Node();
		right->load(file);
	}
	else {
		fseek(file, -sizeof(int), SEEK_CUR);
		Mesh* mesh = new Mesh();
		mesh->load(file);
		min = mesh->getMin();
		max = mesh->getMax();
		center = mesh->getCenter();
		radius = mesh->getRadius();
		object = new ObjectMesh(new MeshVBO(mesh));
		delete mesh;
	}
}


void Node::save(FILE* file) 
{
	if (left && right) 
	{
		int header = 0x7fffffff;
		fwrite(&header, sizeof(int), 1, file);
		fwrite(&min, sizeof(vec3), 1, file);
		fwrite(&max, sizeof(vec3), 1, file);
		fwrite(&center, sizeof(vec3), 1, file);
		fwrite(&radius, sizeof(float), 1, file);
		left->save(file);
		right->save(file);
	}
	else {
		object->mesh->save(file);
	}
}


void Node::bindMaterial(const char* name, Material* material)
{
	if (left) left->bindMaterial(name, material);
	if (right) right->bindMaterial(name, material);
	if (object) object->bindMaterial(name, material);
}


void Node::render(Frustum* frustum) {
	if (left && right) {
		int check_left = frustum->inside(left->center, left->radius);
		int check_right = frustum->inside(right->center, right->radius);
		if (check_left && check_right) {
			if ((left->center - Engine::camera).length() < (right->center - Engine::camera).length()) {
				left->render(frustum);
				right->render(frustum);
			}
			else {
				right->render(frustum);
				left->render(frustum);
			}
			return;
		}
		if (check_left) left->render(frustum);
		else if (check_right) right->render(frustum);
		return;
	}

	if (object && object->frame != Engine::frame) {
		Sector* s = Bsp::visible_sectors[Bsp::num_visible_sectors - 1];
		s->visible_objects[s->num_visible_objects++] = object;
		Engine::num_triangles += object->render(Object::RENDER_OPACITY);
	}
}