#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <glib.h>
#include <GL/gl.h>

#include "model.h"
#include "view.h"
#include "program.h"
#include "util.h"

// Shader structure:
struct shader {
    const char *filename;
	GLuint		 id;
};

// Location definitions:
enum loc_type {
	UNIFORM,
	ATTRIBUTE,
};

struct loc {
	const char	*name;
	enum loc_type	 type;
	GLint		 id;
};

static struct loc loc_bkgd[] = {
	[LOC_BKGD_VERTEX]  = { "vertex",	ATTRIBUTE },
	[LOC_BKGD_TEXTURE] = { "texture",	ATTRIBUTE },
};

static struct loc loc_cube[] = {
	[LOC_CUBE_VIEW]   = { "view_matrix",	UNIFORM   },
	[LOC_CUBE_MODEL]  = { "model_matrix",	UNIFORM   },
	[LOC_CUBE_VERTEX] = { "vertex",		ATTRIBUTE },
	[LOC_CUBE_VCOLOR] = { "vcolor",		ATTRIBUTE },
	[LOC_CUBE_NORMAL] = { "normal",		ATTRIBUTE },
};

// Programs:
enum {
	BKGD,
	CUBE,
};

// Program structure:
static struct program {
	struct {
		struct shader vert;
		struct shader frag;
	} shader;
	struct loc *loc;
	size_t nloc;
	GLuint id;
}
programs[] = {
	[BKGD] = {
		.shader.vert = {.filename = "shaders/bkgd/vertex.glsl"},
		.shader.frag = {.filename = "shaders/bkgd/fragment.glsl"},
		.loc         = loc_bkgd,
		.nloc        = NELEM(loc_bkgd),
	},
	[CUBE] = {
            .shader.vert = {.filename = "shaders/cube/vertex.glsl"},
            .shader.frag = {.filename = "shaders/cube/fragment.glsl"},
		.loc         = loc_cube,
		.nloc        = NELEM(loc_cube),
	},
};

static void
check_compile (GLuint shader)
{
	GLint length;

	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);

	if (length <= 1)
		return;

	GLchar *log = calloc(length, sizeof(GLchar));
	glGetShaderInfoLog(shader, length, NULL, log);
	fprintf(stderr, "glCompileShader failed:\n%s\n", log);
	free(log);
}

static void
check_link (GLuint program)
{
	GLint status, length;

	glGetProgramiv(program, GL_LINK_STATUS, &status);
	if (status != GL_FALSE)
		return;

	glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
	GLchar *log = calloc(length, sizeof(GLchar));
	glGetProgramInfoLog(program, length, NULL, log);
	fprintf(stderr, "glLinkProgram failed: %s\n", log);
	free(log);
}

static void
create_shader (struct shader *shader, GLenum type)
{
    GLchar *buf = NULL;
    gsize bytes_read;
    gboolean success = g_file_get_contents(shader->filename, &buf, &bytes_read, NULL);
    g_assert(success);
	GLint len = (int)bytes_read;

    const GLchar *const_buf = buf;
	shader->id = glCreateShader(type);
	glShaderSource(shader->id, 1, &const_buf, &len);
	glCompileShader(shader->id);

	check_compile(shader->id);
    free(buf);
}

static void
program_init (struct program *p)
{
	struct shader *vert = &p->shader.vert;
	struct shader *frag = &p->shader.frag;

	create_shader(vert, GL_VERTEX_SHADER);
	create_shader(frag, GL_FRAGMENT_SHADER);

	p->id = glCreateProgram();

	glAttachShader(p->id, vert->id);
	glAttachShader(p->id, frag->id);

	glLinkProgram(p->id);
	check_link(p->id);

	glDetachShader(p->id, vert->id);
	glDetachShader(p->id, frag->id);

	glDeleteShader(vert->id);
	glDeleteShader(frag->id);

	FOREACH_NELEM (p->loc, p->nloc, l) {
		switch (l->type)
		{
		case UNIFORM:
			l->id = glGetUniformLocation(p->id, l->name);
			break;

		case ATTRIBUTE:
			l->id = glGetAttribLocation(p->id, l->name);
			break;
		}
	}
}

void
programs_init (void)
{
	FOREACH (programs, p)
		program_init(p);
}

void
program_cube_use (void)
{
	glUseProgram(programs[CUBE].id);

	glUniformMatrix4fv(loc_cube[LOC_CUBE_VIEW ].id, 1, GL_FALSE, view_matrix());
	glUniformMatrix4fv(loc_cube[LOC_CUBE_MODEL].id, 1, GL_FALSE, model_matrix());
}

void
program_bkgd_use (void)
{
	glUseProgram(programs[BKGD].id);

	glUniform1i(glGetUniformLocation(programs[BKGD].id, "tex"), 0);
}

GLint
program_bkgd_loc (const enum LocBkgd index)
{
	return loc_bkgd[index].id;
}

GLint
program_cube_loc (const enum LocCube index)
{
	return loc_cube[index].id;
}
