#include <GL/glew.h>
#include <GL/gl.h>

#include "render.h"
#include "mesh.h"

Light g_light;
Matrix *modelview_matrix;
Matrix *projection_matrix;

static void upload_mesh_to_gpu(Mesh *mesh, Shader *shader)
{
	/* TODO: So much error-checking */

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

	/* Triangles */
	glGenBuffers(1, &mesh->ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, (size_t) mesh->num_indices * 
			sizeof(GLuint), mesh->index, GL_STATIC_DRAW);

	return;
}

void entity_upload_to_gpu(Shader *shader, Entity *ent)
{
	switch(ent->type)
	{
	case TYPE_MESH:
		upload_mesh_to_gpu(ent->data.mesh.mesh, shader);
		break;
	default:
		break;
	}

	return;
}

void lights_upload_to_gpu(Shader *shader)
{
	glUniform3fv(shader->location[SHADER_UNI_LIGHT_DIR], 1, g_light.dir);
	glUniform3fv(shader->location[SHADER_UNI_LIGHT_AMBIENT], 1, g_light.ambient);
	glUniform3fv(shader->location[SHADER_UNI_LIGHT_DIFFUSE], 1, g_light.diffuse);
	glUniform3fv(shader->location[SHADER_UNI_LIGHT_SPECULAR], 1, g_light.specular);
	glUniform1f(shader->location[SHADER_UNI_LIGHT_SHININESS], g_light.shininess);
}

void entity_render(Shader *shader, Entity *ent)
{
	glmPushMatrix(&modelview_matrix);

	glmMultQuaternion(modelview_matrix, ent->data.common.orientation);
	glmTranslate(modelview_matrix, ent->data.common.position.x, 
			ent->data.common.position.y, ent->data.common.position.z);

	glmUniformMatrix(shader->location[SHADER_UNI_P_MATRIX],
			projection_matrix);
	glmUniformMatrix(shader->location[SHADER_UNI_MV_MATRIX], 
			modelview_matrix);

	glDrawRangeElements(GL_TRIANGLES, 0, ent->data.mesh.mesh->num_vertices - 1, 
			ent->data.mesh.mesh->num_indices, GL_UNSIGNED_INT, NULL);

	glmPopMatrix(&modelview_matrix);
}
