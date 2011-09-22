#include <GL/glew.h>
#include <GL/gl.h>

#include "render.h"
#include "mesh.h"

Light g_light;
Matrix *modelview_matrix;
Matrix *projection_matrix;

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

void entity_upload_to_gpu(Shader *shader, Entity *ent)
{
	glGenVertexArrays(1, &ent->vao);
	glBindVertexArray(ent->vao);

	switch(ent->type)
	{
	case ENTITY_MESH:
		upload_mesh_to_gpu(ent->mesh, shader);
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

	newpos = glmTransformVector(modelview_matrix, light->position);
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

void entity_render(Shader *shader, Entity *ent)
{
	glmPushMatrix(&modelview_matrix);

	glmMultQuaternion(modelview_matrix, ent->orientation);
	glmTranslateVector(modelview_matrix, ent->position);

	glmUniformMatrix(shader->location[SHADER_UNI_P_MATRIX], projection_matrix);
	glmUniformMatrix(shader->location[SHADER_UNI_MV_MATRIX], modelview_matrix);

	glBindVertexArray(ent->vao);

	switch(ent->type)
	{
	case ENTITY_MESH:
		render_mesh(ent->mesh);
		break;
	default:
		break;
	};

	glBindVertexArray(0);

	glmPopMatrix(&modelview_matrix);
}
