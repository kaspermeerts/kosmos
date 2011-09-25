#include <GL/glew.h>
#include <GL/gl.h>

#include "render.h"
#include "mesh.h"

GLuint orbit_vbo;

static void upload_mesh_to_gpu(Mesh *mesh, Shader *shader)
{
	/* Vertices */
	glGenBuffers(1, &mesh->vertex_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, mesh->vertex_vbo);
	glBufferData(GL_ARRAY_BUFFER, (size_t) mesh->num_vertices * sizeof(Vertex),
			mesh->vertex, GL_STATIC_DRAW);
	glEnableVertexAttribArray(shader->location[SHADER_ATT_POSITION]);
	glVertexAttribPointer(shader->location[SHADER_ATT_POSITION], 3, GL_FLOAT,
			GL_FALSE, sizeof(Vertex), 0);
	
	/* Normals */
	glGenBuffers(1, &mesh->normal_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, mesh->normal_vbo);
	glBufferData(GL_ARRAY_BUFFER, (size_t) mesh->num_normals * sizeof(Normal),
			mesh->normal, GL_STATIC_DRAW);
	glEnableVertexAttribArray(shader->location[SHADER_ATT_NORMAL]);
	glVertexAttribPointer(shader->location[SHADER_ATT_NORMAL], 3, GL_FLOAT,
			GL_FALSE, sizeof(Normal), 0);

	/* Indices */
	glGenBuffers(1, &mesh->ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, (size_t) mesh->num_indices * 
			sizeof(GLuint), mesh->index, GL_STATIC_DRAW);

	return;
}

static void upload_orbit_to_gpu(int n, Vertex *path, Shader *shader)
{
	glGenBuffers(1, &orbit_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, orbit_vbo);
	glBufferData(GL_ARRAY_BUFFER, (size_t) n * sizeof(Vec3), path,
			GL_STATIC_DRAW);
	glEnableVertexAttribArray(shader->location[SHADER_ATT_POSITION]);
	glVertexAttribPointer(shader->location[SHADER_ATT_POSITION], 3, GL_FLOAT,
		GL_FALSE, sizeof(Vec3), 0);
}

void entity_upload_to_gpu(Shader *shader, Entity *ent)
{
	glGenVertexArrays(1, &ent->vao);
	glBindVertexArray(ent->vao);

	switch(ent->type)
	{
	case ENTITY_MESH:
		upload_mesh_to_gpu(ent->mesh, shader);
		break;
	case ENTITY_ORBIT:
		upload_orbit_to_gpu(ent->num_samples, ent->sample, shader);
		break;
	default:
		break;
	}

	glBindVertexArray(0); /* So we don't accidentally overwrite the VAO */

	return;
}

void light_upload_to_gpu(Shader *shader, Light *light)
{
	Vec3 newpos;
	GLfloat pos3f[3];

	newpos = glmTransformVector(glmViewMatrix, light->position);
	pos3f[0] = newpos.x; pos3f[1] = newpos.y; pos3f[2] = newpos.z;

	glUniform3fv(shader->location[SHADER_UNI_LIGHT_POS], 1, pos3f);
	glUniform3fv(shader->location[SHADER_UNI_LIGHT_AMBIENT], 1, light->ambient);
	glUniform3fv(shader->location[SHADER_UNI_LIGHT_DIFFUSE], 1, light->diffuse);
	glUniform3fv(shader->location[SHADER_UNI_LIGHT_SPECULAR], 1, light->specular);
	glUniform1f(shader->location[SHADER_UNI_LIGHT_SHININESS], light->shininess);
}

static void render_mesh(Mesh *mesh)
{
	glDrawRangeElements(GL_TRIANGLES, 0, mesh->num_vertices - 1, 
			mesh->num_indices, GL_UNSIGNED_INT, NULL);
}

static void render_orbit(int n)
{
	glDrawArrays(GL_LINE_LOOP, 0, n);
}

void entity_render(Shader *shader, Entity *ent)
{
	glmPushMatrix(&glmModelMatrix);

	glmMultQuaternion(glmModelMatrix, ent->orientation);
	glmTranslateVector(glmModelMatrix, ent->position);

	glmUniformMatrix(shader->location[SHADER_UNI_P_MATRIX], glmProjectionMatrix);
	glmUniformMatrix(shader->location[SHADER_UNI_V_MATRIX], glmViewMatrix);
	glmUniformMatrix(shader->location[SHADER_UNI_M_MATRIX], glmModelMatrix);

	glBindVertexArray(ent->vao);

	switch(ent->type)
	{
	case ENTITY_MESH:
		render_mesh(ent->mesh);
		break;
	case ENTITY_ORBIT:
		glBindBuffer(GL_ARRAY_BUFFER, orbit_vbo);
		render_orbit(ent->num_samples);
		break;
	default:
		break;
	};

	glBindVertexArray(0);

	glmPopMatrix(&glmModelMatrix);
}
