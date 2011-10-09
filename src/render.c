#include <GL/glew.h>
#include <GL/gl.h>

#include "render.h"
#include "mesh.h"

GLuint orbit_vbo;

void mesh_upload_to_gpu(void *data, Shader *shader)
{
	Mesh *mesh = (Mesh *) data;
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

void renderable_upload_to_gpu(Renderable *obj, Shader *shader)
{
	glGenVertexArrays(1, &obj->vao);
	glBindVertexArray(obj->vao);

	obj->upload_to_gpu(obj->data, shader);
	obj->memloc = MEMLOC_GPU;

	glBindVertexArray(0); /* So we don't accidentally overwrite the VAO */

	return;
}

void light_upload_to_gpu(void *data, Shader *shader)
{
	Vec3 newpos;
	GLfloat pos3f[3];
	Light *light = (Light *) data;

	newpos = glmTransformVector(glmViewMatrix, light->position);
	pos3f[0] = newpos.x; pos3f[1] = newpos.y; pos3f[2] = newpos.z;

	glUniform3fv(shader->location[SHADER_UNI_LIGHT_POS], 1, pos3f);
	glUniform3fv(shader->location[SHADER_UNI_LIGHT_AMBIENT], 1, light->ambient);
	glUniform3fv(shader->location[SHADER_UNI_LIGHT_DIFFUSE], 1, light->diffuse);
	glUniform3fv(shader->location[SHADER_UNI_LIGHT_SPECULAR], 1, light->specular);
}

void mesh_render(void *self, Shader *shader)
{
	Mesh *mesh = (Mesh *) self;

	shader = shader;
	glDrawRangeElements(GL_TRIANGLES, 0, mesh->num_vertices - 1,
			mesh->num_indices, GL_UNSIGNED_INT, NULL);
}

void render_entity_list(Entity *ent, Shader *shader)
{
	while(ent)
	{
		entity_render(ent, shader);
		ent = ent->next;
	}
}

void entity_render(Entity *ent, Shader *shader)
{
	glmPushMatrix(&glmModelMatrix);

	glmLoadIdentity(glmModelMatrix);
	glmMultQuaternion(glmModelMatrix, ent->orientation);
	glmTranslateVector(glmModelMatrix, ent->position);
	glmScaleUniform(glmModelMatrix, ent->scale);

	glmUniformMatrix(shader->location[SHADER_UNI_P_MATRIX], glmProjectionMatrix);
	glmUniformMatrix(shader->location[SHADER_UNI_V_MATRIX], glmViewMatrix);
	glmUniformMatrix(shader->location[SHADER_UNI_M_MATRIX], glmModelMatrix);

	glBindVertexArray(ent->renderable->vao);

	ent->renderable->render(ent->renderable->data, shader);

	glBindVertexArray(0);

	glmPopMatrix(&glmModelMatrix);
}
