#include "UI.h"

#include "Window.h"
#include "Shader.h"

#include "Map/Block.h"
#include "Map/Map.h"

typedef struct
{
  GLuint VAOCrosshairs;
  GLuint VBOCrosshairs;

  GLuint VAOBlockWireframe;
  GLuint VBOBlockWireframe;
} UI;

static UI* ui; //To distinguish the two, the variable name must differ from its type!

static void UIFramebufferSizeChangeCallback(void* thisObject, int32_t newWidth, int32_t newHeight)
{
  UIUpdateAspectRatio((float)newWidth / newHeight);

  thisObject; //A reference to resolve "C4100".
}

void UIInit(float aspectRatio)
{
  ui = (UI*)OwnMalloc(sizeof(UI), false);

  if(ui == NULL)
  {
    LogError("Variable \"ui\" in function \"%s\" (error output line: %d) from file \"%s\" must not be \"NULL\".", true, __func__, __LINE__, __FILE__);

    return;
  }

  RegisterFramebufferSizeChangeCallback(ui, UIFramebufferSizeChangeCallback);

  //Crosshairs-buffer:
  float center = 0.0f;
  float size = 0.03f;

  float verticesCrosshairs[] = 
  {
    (center - size) / aspectRatio, center, 0.0f,
    (center + size) / aspectRatio, center, 0.0f,

    center, (center - size), 0.0f,
    center, (center + size), 0.0f
  };

  ui->VAOCrosshairs = OpenGLCreateVAO();
  ui->VBOCrosshairs = OpenGLCreateVBO(verticesCrosshairs, sizeof(verticesCrosshairs));
  OpenGL_VBOLayout(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
  //Never OpenGL: OpenGL_VBOLayout(ui->VAOCrosshairs, ui->VBOCrosshairs, 0, 0, 3, GL_FLOAT, GL_FALSE, 0, 3 * sizeof(float));

  //Block wireframe buffer:
  float x = 0.0f, y = 0.0f, z = 0.0f;
  float bSize = BLOCK_SIZE;

  float offset = 0.001f * BLOCK_SIZE;
  float verticesWireframe[] = 
  {
    x - offset, y - offset, z - offset,
    x + bSize + offset, y - offset, z - offset,

    x + bSize + offset, y - offset, z - offset,
    x + bSize + offset, y + bSize + offset, z - offset,

    x + bSize + offset, y + bSize + offset, z - offset,
    x - offset, y + bSize + offset, z - offset,

    x - offset, y + bSize + offset, z - offset,
    x - offset, y - offset, z - offset,

    x - offset, y - offset, z + bSize + offset,
    x + bSize + offset, y - offset, z + bSize + offset,

    x + bSize + offset, y - offset, z + bSize + offset,
    x + bSize + offset, y + bSize + offset, z + bSize + offset,

    x + bSize + offset, y + bSize + offset, z + bSize + offset,
    x - offset, y + bSize + offset, z + bSize + offset,

    x - offset, y + bSize + offset, z + bSize + offset,
    x - offset, y - offset, z + bSize + offset,

    x - offset, y - offset, z + bSize + offset,
    x - offset, y - offset, z - offset,

    x + bSize + offset, y - offset, z + bSize + offset,
    x + bSize + offset, y - offset, z - offset,

    x + bSize + offset, y + bSize + offset, z + bSize + offset,
    x + bSize + offset, y + bSize + offset, z - offset,

    x - offset, y + bSize + offset, z + bSize + offset,
    x - offset, y + bSize + offset, z - offset
  };

  ui->VAOBlockWireframe = OpenGLCreateVAO();
  ui->VBOBlockWireframe = OpenGLCreateVBO(verticesWireframe, sizeof(verticesWireframe));
  OpenGL_VBOLayout(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
  //Never OpenGL: OpenGL_VBOLayout(ui->VAOBlockWireframe, ui->VBOBlockWireframe, 0, 0, 3, GL_FLOAT, GL_FALSE, 0, 3 * sizeof(float));
}

void UIUpdateAspectRatio(float newRatio)
{
  glDeleteVertexArrays(1, &ui->VAOCrosshairs);
  glDeleteBuffers(1, &ui->VBOCrosshairs);

  float center = 0.0f;
  float size = 0.03f;

  float vertices[] = 
  {
    (center - size) / newRatio, center, 0.0f,
    (center + size) / newRatio, center, 0.0f,

    center, (center - size), 0.0f,
    center, (center + size), 0.0f
  };

  ui->VAOCrosshairs = OpenGLCreateVAO();
  ui->VBOCrosshairs = OpenGLCreateVBO(vertices, sizeof(vertices));
  OpenGL_VBOLayout(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
  //Never OpenGL: OpenGL_VBOLayout(ui->VAOCrosshairs, ui->VBOCrosshairs, 0, 0, 3, GL_FLOAT, GL_FALSE, 0, 3 * sizeof(float));
}

void UIRenderCrosshairs()
{
  glBindVertexArray(ui->VAOCrosshairs);

  glUseProgram(SHADER_LINE);
  mat4 ident;
  glm_mat4_identity(ident);
  ShaderSetMat4(SHADER_LINE, "MVPMatrix", ident);

  glDisable(GL_DEPTH_TEST);
  glLineWidth(1.0f);
  glDrawArrays(GL_LINES, 0, 4);
}

void UIRenderBlockWireframe(Player* p, Camera* cam)
{
  glBindVertexArray(ui->VAOBlockWireframe);

  float x = p->blockPointedAt[0] * BLOCK_SIZE;
  float y = p->blockPointedAt[1] * BLOCK_SIZE;
  float z = p->blockPointedAt[2] * BLOCK_SIZE;

  mat4 model;
  glm_mat4_identity(model);
  glm_translate(model, (vec3){x, y, z});

  uint8_t block = MapGetBlock(p->blockPointedAt[0], p->blockPointedAt[1], p->blockPointedAt[2]);

  //Diminish wireframe:
  if(BlockIsPlant(block))
  {
    glm_scale(model, (vec3){0.5f, 0.5f, 0.5f});
    glm_translate(model, (vec3){BLOCK_SIZE / 2.0f, 0.0f, BLOCK_SIZE / 2.0f});
  }

  mat4 MVP;
  glm_mat4_mul(cam->viewProjMatrix, model, MVP);

  glUseProgram(SHADER_LINE);
  ShaderSetMat4(SHADER_LINE, "MVPMatrix", MVP);

  glLineWidth(1.0f);
  glDrawArrays(GL_LINES, 0, 24);
}

void UIFree()
{
  if(ui == NULL)
  {
    LogWarning("For \"UIFree()\" incurred only half the work as the variable \"ui\" was already \"NULL\".", true);

    return;
  }

  glDeleteVertexArrays(2, (GLuint[2])
  {
    ui->VAOCrosshairs,
    ui->VAOBlockWireframe
  });

  glDeleteBuffers(2, (GLuint[2])
  {
    ui->VBOCrosshairs,
    ui->VBOBlockWireframe
  });

  free(ui);
  ui = NULL;
}