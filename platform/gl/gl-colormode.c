/* Support for different color modes. */

#include "gl-app.h"

int colormode = COLMODE_NORMAL;

static GLuint fragment_shader_yellow_mono, fragment_shader_yellow_multi;
static GLuint shader_program_yellow_mono, shader_program_yellow_multi;
static int shaders_inited = 0;

const struct color_scheme color_scheme[COLMODE_COUNT] =
{
	/* COLMODE_NORMAL */
	{
		NULL,
		{ 0.3f, 0.3f, 0.3f, 1.0f },	/* Canvas background */
		{ 1.0f, 1.0f, 1.0f, 1.0f },	/* Label background */
		{ 0.0f, 0.0f, 0.0f, 1.0f },	/* Label text */
		{ 0.6f, 0.6f, 0.6f, 1.0f },	/* Scrollbar inactive */
		{ 0.8f, 0.8f, 0.8f, 1.0f },	/* Scrollbar active */
		{ 0.9f, 0.9f, 0.9f, 1.0f },	/* Outline highlight current page */
		{ 1.0f, 1.0f, 1.0f, 1.0f },	/* Outline background */
		{ 0.0f, 0.0f, 0.0f, 1.0f },	/* Outline text */
		{ 1.0f, 0.0f, 0.0f, 0.4f },	/* Search hit highlighting */
		{ 1.0f, 1.0f, 1.0f, 1.0f },	/* Input field background */
		{ 0.0f, 0.0f, 0.0f, 1.0f },	/* Input field text */
		{ 0.6f, 0.6f, 1.0f, 1.0f },	/* Input field selected text */
		{ 0.9f, 0.9f, 0.9f, 1.0f },	/* Info background */
		{ 0.0f, 0.0f, 0.0f, 1.0f },	/* Info text */
		{ 0.9f, 0.9f, 0.9f, 1.0f },	/* Help background */
		{ 0.0f, 0.0f, 0.0f, 1.0f }	/* Help text */
	},
	/* COLMODE_YELLOW_MONOCHROME */
	{
		&shader_program_yellow_mono,
		{ 0.0f, 0.0f, 0.0f, 1.0f },	/* Canvas background */
		{ 0.0f, 0.0f, 0.0f, 1.0f },	/* Label background */
		{ 1.0f, 1.0f, 0.0f, 1.0f },	/* Label text */
		{ 0.2f, 0.2f, 0.2f, 1.0f },	/* Scrollbar inactive */
		{ 0.4f, 0.4f, 0.4f, 1.0f },	/* Scrollbar active */
		{ 0.25f, 0.25f, 0.25f, 1.0f },  /* Outline highlight current page */
		{ 0.0f, 0.0f, 0.0f, 1.0f },	/* Outline background */
		{ 1.0f, 1.0f, 0.0f, 1.0f },	/* Outline text */
		{ 0.3f, 0.0f, 0.0f, 0.3f },	/* Search hit highlighting */
		{ 0.0f, 0.0f, 0.0f, 1.0f },	/* Input field background */
		{ 1.0f, 1.0f, 0.0f, 1.0f },	/* Input field text */
		{ 0.1f, 0.2f, 0.8f, 1.0f },	/* Input field selected text */
		{ 0.2f, 0.2f, 0.2f, 1.0f },	/* Info background */
		{ 1.0f, 1.0f, 0.0f, 1.0f },	/* Info text */
		{ 0.2f, 0.2f, 0.2f, 1.0f },	/* Help background */
		{ 1.0f, 1.0f, 0.0f, 1.0f }	/* Help text */
	},
	/* COLMODE_YELLOW_MULTI */
	{
		&shader_program_yellow_multi,
		{ 0.0f, 0.0f, 0.0f, 1.0f },	/* Canvas background */
		{ 0.0f, 0.0f, 0.0f, 1.0f },	/* Label background */
		{ 1.0f, 1.0f, 0.0f, 1.0f },	/* Label text */
		{ 0.2f, 0.2f, 0.2f, 1.0f },	/* Scrollbar inactive */
		{ 0.4f, 0.4f, 0.4f, 1.0f },	/* Scrollbar active */
		{ 0.25f, 0.25f, 0.25f, 1.0f },  /* Outline highlight current page */
		{ 0.0f, 0.0f, 0.0f, 1.0f },	/* Outline background */
		{ 1.0f, 1.0f, 0.0f, 1.0f },	/* Outline text */
		{ 0.3f, 0.0f, 0.0f, 0.3f },	/* Search hit highlighting */
		{ 0.0f, 0.0f, 0.0f, 1.0f },	/* Input field background */
		{ 1.0f, 1.0f, 0.0f, 1.0f },	/* Input field text */
		{ 0.1f, 0.2f, 0.8f, 1.0f },	/* Input field selected text */
		{ 0.2f, 0.2f, 0.2f, 1.0f },	/* Info background */
		{ 1.0f, 1.0f, 0.0f, 1.0f },	/* Info text */
		{ 0.2f, 0.2f, 0.2f, 1.0f },	/* Help background */
		{ 1.0f, 1.0f, 0.0f, 1.0f }	/* Help text */
	}
};

/*
  OpenGL Shader for yellow-on-black color mode. The color of each pixel is
  first converted to grey-scale, and then mapped inversed to yellow. The result
  is that tradition black letters on white background appears as yellow on
  black.
*/
static const char fragment_shader_src_yellow_mono[] =
  "uniform sampler2D tex;\n"
  "void main (void)\n"
  "{\n"
  "  vec4 color = texture2D(tex,gl_TexCoord[0].st);\n"
  "  float bright = 0.2125*color.r + 0.7154*color.g + 0.0721*color.b;\n"
  "  float inv = color.a*(1.0 - bright);\n"
  "  gl_FragColor = vec4(inv, inv, 0.0, color.a);\n"
  "}\n";

/*
  A variation of the yellow-on-black shader that tries to preserve other
  colours. It first transforms into HSL space. Then it inverts the L axis
  to transform light to dark and vice versa. Finally it tints yellow those
  colours that are of low saturation or with L close to max or min.
*/
static const char fragment_shader_src_yellow_multi[] =
  "vec3 hsl2rgb(in float h, in float s, in float l) {\n"
  "	float c = (1.0 - abs(2.0*l-1.0)) * s;\n"
  "	float x = c*(1.0 - abs(mod(h, 2.0) - 1.0));\n"
  "	vec3 rgb;\n"
  "	if (h <= 1.0)\n"
  "		rgb = vec3(c, x, 0.0);\n"
  "	else if (h <= 2.0)\n"
  "		rgb = vec3(x, c, 0.0);\n"
  "	else if (h <= 3.0)\n"
  "		rgb = vec3(0.0, c, x);\n"
  "	else if (h <= 4.0)\n"
  "		rgb = vec3(0.0, x, c);\n"
  "	else if (h <= 5.0)\n"
  "		rgb = vec3(x, 0.0, c);\n"
  "	else\n"
  "		rgb = vec3(c, 0.0, x);\n"
  "	float m = l - 0.5*c;\n"
  "	return rgb + m;\n"
  "}\n"
  "\n"
  "vec3 rgb2hsl(in float r, in float g, in float b) {\n"
  "	float M = max(max(r, g), b);\n"
  "	float m = min(min(r, g), b);\n"
  "	float c = M - m;\n"
  "	float crecip = 1.0/c;\n"
  "	float h;\n"
  "	if (c == 0.0)\n"
  "		h = 0.0;\n"
  "	else if (M == r)\n"
  "		h = mod((g - b)*crecip, 6.0);\n"
  "	else if (M == g)\n"
  "		h = (b - r)*crecip + 2.0;\n"
  "	else\n"
  "		h = (r - g)*crecip + 4.0;\n"
  "	float l = 0.5*(M + m);\n"
  "	float s;\n"
  "	if (c == 0.0)\n"
  "		s = 0.0;\n"
  "	else\n"
  "		s = c / (1.0 - abs(2.0*l - 1.0));\n"
  "	return vec3(h, s, l);\n"
  "}\n"
  "\n"
  "uniform sampler2D tex;\n"
  "void main (void)\n"
  "{\n"
  "  vec4 color = texture2D(tex, gl_TexCoord[0].st);\n"
  "  vec3 hsl = rgb2hsl(color.r, color.g, color.b);\n"
  "  float new_l = 1.0 - hsl[2];\n"
  "  float tint = pow(hsl[1]*(1.0 - abs(1.0 - 2.0*new_l)), 1.0/4.0);\n"
  "  vec3 rgb = hsl2rgb(hsl[0], hsl[1], color.a*new_l);"
  "  gl_FragColor = vec4(rgb.r, rgb.g, tint*rgb.b, color.a);\n"
  "}\n";

int init_shaders(void)
{
	char buf[1024];
	const char *shader_src_ptr;
	GLint param;

	if (shaders_inited)
		return 1;

	if (glewInit() != GLEW_OK)
	{
		fz_warn(ctx, "OpenGL shader initialisation failed, "
			"color modes will be unavailable");
		return 0;
	}
	if (!glewIsSupported("GL_VERSION_2_0"))
	{
		fz_warn(ctx, "OpenGL version 2.0 not available, "
			"color modes are not supported");
		return 0;
	}

	fragment_shader_yellow_mono = glCreateShader(GL_FRAGMENT_SHADER);
	fragment_shader_yellow_multi = glCreateShader(GL_FRAGMENT_SHADER);
	if (!fragment_shader_yellow_mono || !fragment_shader_yellow_multi)
	{
		fz_warn(ctx, "Could not create OpenGL shaders, "
			"color modes will be unavailable");
		return 0;
	}

	shader_src_ptr = fragment_shader_src_yellow_mono;
	glShaderSource(fragment_shader_yellow_mono, 1, &shader_src_ptr, NULL);
	shader_src_ptr = fragment_shader_src_yellow_multi;
	glShaderSource(fragment_shader_yellow_multi, 1, &shader_src_ptr, NULL);

	glCompileShader(fragment_shader_yellow_mono);
	glGetShaderiv(fragment_shader_yellow_mono, GL_COMPILE_STATUS, &param);
	if (!param) {
		GLsizei out_len;
		glGetShaderInfoLog(fragment_shader_yellow_mono, sizeof(buf)-1, &out_len, buf);
		fz_warn(ctx, "Error compiling shader, "
			"color modes will be unavailable: %s", buf);
		return 0;
	}
	glCompileShader(fragment_shader_yellow_multi);
	glGetShaderiv(fragment_shader_yellow_multi, GL_COMPILE_STATUS, &param);
	if (!param) {
		GLsizei out_len;
		glGetShaderInfoLog(fragment_shader_yellow_multi, sizeof(buf)-1, &out_len, buf);
		fz_warn(ctx, "Error compiling shader, "
			"color modes will be unavailable: %s", buf);
		return 0;
	}

	shader_program_yellow_mono = glCreateProgram();
	shader_program_yellow_multi = glCreateProgram();
	if (!shader_program_yellow_mono || !shader_program_yellow_multi)
	{
		fz_warn(ctx, "Could not create OpenGL shader programs, "
			"color modes will be unavailable");
		return 0;
	}
	glAttachShader(shader_program_yellow_mono, fragment_shader_yellow_mono);
	glAttachShader(shader_program_yellow_multi, fragment_shader_yellow_multi);
	glLinkProgram(shader_program_yellow_mono);
	glGetProgramiv(shader_program_yellow_mono, GL_LINK_STATUS, &param);
	if (!param)
	{
		fz_warn(ctx, "OpenGL shader program link failure, "
			"color modes will be unavailable");
		return 0;
	}
	glLinkProgram(shader_program_yellow_multi);
	glGetProgramiv(shader_program_yellow_multi, GL_LINK_STATUS, &param);
	if (!param)
	{
		fz_warn(ctx, "OpenGL shader program link failure, "
			"color modes will be unavailable");
		return 0;
	}

	shaders_inited = 1;
	return 1;				/* Success */
}

void finish_shaders(void)
{
	if (!shaders_inited)
		return;
	glUseProgram(0);
	glDetachShader(shader_program_yellow_mono, fragment_shader_yellow_mono);
	glDetachShader(shader_program_yellow_multi, fragment_shader_yellow_multi);
	glDeleteProgram(shader_program_yellow_mono);
	glDeleteProgram(shader_program_yellow_multi);
	glDeleteShader(fragment_shader_yellow_mono);
	glDeleteShader(fragment_shader_yellow_multi);
	shaders_inited = 0;
}
