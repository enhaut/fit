/*!
 * @file
 * @brief This file contains implementation of gpu
 *
 * @author Tomáš Milet, imilet@fit.vutbr.cz
 * @author Samuel Dobroň, xdobro23@stud.fit.vutbr.cz
 */

#include <student/gpu.hpp>
#include "fwd.hpp"

uint32_t getVertexID(VertexArray const &vao, uint32_t vertexID)
{
    if (!vao.indexBuffer)
        return vertexID;

    if (vao.indexType == IndexType::UINT32)
        return ((uint32_t *)vao.indexBuffer)[vertexID];
    else if (vao.indexType == IndexType::UINT16)
        return (uint32_t)(((uint16_t *)vao.indexBuffer)[vertexID]);
    else
        return (uint32_t)(((uint8_t *)vao.indexBuffer)[vertexID]);
}

void setVertexAttributes(GPUContext &ctx, InVertex &inV)
{
    for (uint32_t i = 0; i < maxAttributes; i++)
    {
        if (!ctx.vao.vertexAttrib[i].bufferData || ctx.vao.vertexAttrib[i].type == AttributeType::EMPTY)
            continue;

        uint64_t offset = ctx.vao.vertexAttrib[i].offset + ctx.vao.vertexAttrib[i].stride * inV.gl_VertexID;
        auto *bufferData = (uint8_t *)ctx.vao.vertexAttrib[i].bufferData;
        auto attribute_value = bufferData + offset;

        auto vertexAttributeType = ctx.vao.vertexAttrib[i].type;
        if (vertexAttributeType == AttributeType::FLOAT)
            inV.attributes[i].v1 = *(float *)attribute_value;
        else if (vertexAttributeType == AttributeType::VEC2)
            inV.attributes[i].v2 = *(glm::vec2 *)attribute_value;
        else if (vertexAttributeType == AttributeType::VEC3)
            inV.attributes[i].v3 = *(glm::vec3 *)attribute_value;
        else
            inV.attributes[i].v4 = *(glm::vec4 *)attribute_value;
    }
}
typedef struct Triangle {
    OutVertex points[3];
}Triangle_t;


#include <iostream>

using namespace std;
void loadTriangle(Triangle_t &triangle, GPUContext &ctx, uint32_t ID)
{
    for (auto & point : triangle.points)
    {
        InVertex inVertex;
        inVertex.gl_VertexID = getVertexID(ctx.vao, ID);
        setVertexAttributes(ctx, inVertex);
        ctx.prg.vertexShader(point, inVertex, ctx.prg.uniforms);
    }
}

#define DIVISION(i) (point.gl_Position[i] / point.gl_Position[3])
void perspectiveDivision(Triangle_t &triangle)
{
    for (auto & point : triangle.points)
    {
        //glm::vec4 position;
        point.gl_Position[0] = DIVISION(0);
        point.gl_Position[1] = DIVISION(1);
        point.gl_Position[2] = DIVISION(2);
        //point.gl_Position[3] = 0;

        //point.gl_Position = position;
    }
}

#define POINT_VIEWPORT_TRANSFORMATION(i) ()
void viewportTransformation(Triangle_t &triangle, GPUContext &ctx)
{
    for (auto & point : triangle.points)
    {
        point.gl_Position[0] = (point.gl_Position[0]/ctx.frame.width * 0.5f + 0.5f) * ctx.frame.width;
        point.gl_Position[1] = (point.gl_Position[1]/ctx.frame.width * 0.5f + 0.5f) * ctx.frame.height;
    }
}

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))
void rasterize(GPUContext &ctx, Triangle_t &triangle)
{
    int minX, minY, maxX, maxY;
    for (auto & point : triangle.points)
    {
        minX = MIN(minX, point.gl_Position[0]);
        minY = MIN(minY, point.gl_Position[1]);
        maxX = MAX(maxX, point.gl_Position[0]);
        maxY = MAX(maxY, point.gl_Position[1]);
    }

    for (int x = minX; x < maxX; x++)
    {
        for (int y = minY; y < maxY; y++)
        {
            InFragment inFragment;
            OutFragment outFragment;
            ctx.prg.fragmentShader(outFragment, inFragment, ctx.prg.uniforms);
        }
    }
}


//! [drawTrianglesImpl]
void drawTrianglesImpl(GPUContext &ctx, uint32_t nofVertices){
  for (uint32_t i = 0; i < nofVertices; i++)
  {
      InVertex inVertex;
      inVertex.gl_VertexID = getVertexID(ctx.vao, i);
      setVertexAttributes(ctx, inVertex);

      OutVertex outVertex;
      ctx.prg.vertexShader(outVertex, inVertex, ctx.prg.uniforms);
  }
/*
  for (uint32_t i = 0; i < nofVertices/3; i += 3)
  {
      Triangle_t triangle;
      loadTriangle(triangle, ctx, i);
      perspectiveDivision(triangle);

      viewportTransformation(triangle, ctx);
  }*/
}
//! [drawTrianglesImpl]

/**
 * @brief This function reads color from texture.
 *
 * @param texture texture
 * @param uv uv coordinates
 *
 * @return color 4 floats
 */
glm::vec4 read_texture(Texture const&texture,glm::vec2 uv){
  if(!texture.data)return glm::vec4(0.f);
  auto uv1 = glm::fract(uv);
  auto uv2 = uv1*glm::vec2(texture.width-1,texture.height-1)+0.5f;
  auto pix = glm::uvec2(uv2);
  //auto t   = glm::fract(uv2);
  glm::vec4 color = glm::vec4(0.f,0.f,0.f,1.f);
  for(uint32_t c=0;c<texture.channels;++c)
    color[c] = texture.data[(pix.y*texture.width+pix.x)*texture.channels+c]/255.f;
  return color;
}

/**
 * @brief This function clears framebuffer.
 *
 * @param ctx GPUContext
 * @param r red channel
 * @param g green channel
 * @param b blue channel
 * @param a alpha channel
 */
void clear(GPUContext&ctx,float r,float g,float b,float a){
  auto&frame = ctx.frame;
  auto const nofPixels = frame.width * frame.height;
  for(size_t i=0;i<nofPixels;++i){
    frame.depth[i] = 10e10f;
    frame.color[i*4+0] = static_cast<uint8_t>(glm::min(r*255.f,255.f));
    frame.color[i*4+1] = static_cast<uint8_t>(glm::min(g*255.f,255.f));
    frame.color[i*4+2] = static_cast<uint8_t>(glm::min(b*255.f,255.f));
    frame.color[i*4+3] = static_cast<uint8_t>(glm::min(a*255.f,255.f));
  }
}

