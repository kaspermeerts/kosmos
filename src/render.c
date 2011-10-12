#include <GL/glew.h>
#include <GL/gl.h>

#include "render.h"
#include "mesh.h"

GLuint orbit_vbo;

void mesh_upload_to_gpu(Renderable *obj)
{
	Mesh *mesh = (Mesh *) obj->data;
	Shader *shader = obj->shader;
	/* Vertices */
	glGenBuffers(1, &mesh->vbo);
	glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
	glBufferData(GL_ARRAY_BUFFER, (size_t) mesh->num_vertices * sizeof(Vertex3N),
			mesh->vertex, GL_STATIC_DRAW);
	glEnableVertexAttribArray(shader->location[SHADER_ATT_POSITION]);
	glVertexAttribPointer(shader->location[SHADER_ATT_POSITION], 3, GL_FLOAT,
			GL_FALSE, sizeof(Vertex3N), (void *) offsetof(Vertex3N, x));
	glEnableVertexAttribArray(shader->location[SHADER_ATT_NORMAL]);
	glVertexAttribPointer(shader->location[SHADER_ATT_NORMAL], 3, GL_FLOAT,
			GL_FALSE, sizeof(Vertex3N), (void *) offsetof(Vertex3N, nx));

	/* Indices */
	glGenBuffers(1, &mesh->ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, (size_t) mesh->num_indices *
			sizeof(GLuint), mesh->index, GL_STATIC_DRAW);

	return;
}

void point_upload_to_gpu(Renderable *obj)
{
	Shader *shader = obj->shader;
	GLuint vbo;
	GLfloat origin[] = {0, 0, 0};

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(origin), origin, GL_STATIC_DRAW);
	glEnableVertexAttribArray(shader->location[SHADER_ATT_POSITION]);
	glVertexAttribPointer(shader->location[SHADER_ATT_POSITION], 3, GL_FLOAT,
			GL_FALSE, sizeof(origin), 0);
}

void renderable_upload_to_gpu(Renderable *obj)
{
	glGenVertexArrays(1, &obj->vao);
	glBindVertexArray(obj->vao);

	obj->upload_to_gpu(obj);
	obj->memloc = MEMLOC_GPU;

	glBindVertexArray(0); /* So we don't accidentally overwrite the VAO */

	return;
}

void light_upload_to_gpu(void *data, Shader *shader)
{
	Vec3 newpos;
	GLfloat pos3f[3];
	Light *light = (Light *) data;
	GLint pos_loc, ambi_loc, diff_loc, spec_loc;

	newpos = glmTransformVector(glmViewMatrix, light->position);
	pos3f[0] = newpos.x; pos3f[1] = newpos.y; pos3f[2] = newpos.z;
	pos_loc = glGetUniformLocation(shader->program, "light_pos");
	ambi_loc = glGetUniformLocation(shader->program, "light_ambient");
	diff_loc = glGetUniformLocation(shader->program, "light_diffuse");
	spec_loc = glGetUniformLocation(shader->program, "light_specular");
	glUniform3fv(pos_loc, 1, pos3f);
	glUniform3fv(ambi_loc, 1, light->ambient);
	glUniform3fv(diff_loc, 1, light->diffuse);
	glUniform3fv(spec_loc, 1, light->specular);
}

void mesh_render(Renderable *obj)
{
	Mesh *mesh = (Mesh *) obj->data;

	glDrawRangeElements(GL_TRIANGLES, 0, mesh->num_vertices - 1,
			mesh->num_indices, GL_UNSIGNED_INT, NULL);
}

void point_render(Renderable *obj)
{
	obj = NULL;
	glDrawArrays(GL_POINTS, 0, 1);
}

static void entity_render(Entity *ent)
{
	Shader *shader = ent->renderable->shader;
	glmPushMatrix(&glmModelMatrix);

	glmLoadIdentity(glmModelMatrix);
	glmTranslateVector(glmModelMatrix, ent->position);
	glmMultQuaternion(glmModelMatrix, ent->orientation);
	glmScaleUniform(glmModelMatrix, ent->radius);

	glmUniformMatrix(shader->location[SHADER_UNI_P_MATRIX], glmProjectionMatrix);
	glmUniformMatrix(shader->location[SHADER_UNI_V_MATRIX], glmViewMatrix);
	glmUniformMatrix(shader->location[SHADER_UNI_M_MATRIX], glmModelMatrix);

	glBindVertexArray(ent->renderable->vao);

	ent->renderable->render(ent->renderable);

	glBindVertexArray(0);

	glmPopMatrix(&glmModelMatrix);
}

void render_entity_list(Entity *ent)
{
	while(ent)
	{
		if (ent->prev == NULL || ent->prev->renderable->shader != ent->renderable->shader)
			glUseProgram(ent->renderable->shader->program);
		entity_render(ent);
		ent = ent->next;
	}
}
