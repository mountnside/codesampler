#include "projtex.h"

/*
this class handles all the setting up of the textures as well as the texture matrix
operations that will project the light map onto our cube. Again, we are using the fixed function pipeline
to achieve this solution
*/

TexProj::TexProj()
{
	texWidth = 64;
	texHeight = 64;
}

TexProj::~TexProj()
{
}

void TexProj::loadTexture(const char *name)
{
	AUX_RGBImageRec *Image;

	Image = auxDIBImageLoad(name);

	glGenTextures(1, &texObject);
	BindProjectiveTexture(false);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	gluBuild2DMipmaps(GL_TEXTURE_2D,GL_RGB,Image->sizeX,Image->sizeY,GL_RGB,GL_UNSIGNED_BYTE,Image->data);

	if(Image)
	{
		if(Image->data)
			free(Image->data);
		free(Image);
	}
}

/*
void TexProj::loadLightMapTexture()
{
	GLfloat eyePlaneS[] =  {1.0f, 0.0f, 0.0f, 0.0f};
	GLfloat eyePlaneT[] =  {0.0f, 1.0f, 0.0f, 0.0f};
	GLfloat eyePlaneR[] =  {0.0f, 0.0f, 1.0f, 0.0f};
	GLfloat eyePlaneQ[] =  {0.0f, 0.0f, 0.0f, 1.0f};
	GLfloat borderColor[4] = {0.1f, 0.1f, 0.1f, 1.0f};
	GLubyte *data;
	GLubyte *spotTextureIterator;

	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);
	glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);

	//set up texture generation mode and set the corresponding planes
	glTexGeni(GL_S,GL_TEXTURE_GEN_MODE,GL_EYE_LINEAR);
	glTexGenfv(GL_S,GL_EYE_PLANE,eyePlaneS);
	glTexGeni(GL_T,GL_TEXTURE_GEN_MODE,GL_EYE_LINEAR);
	glTexGenfv(GL_T,GL_EYE_PLANE,eyePlaneT);
	glTexGeni(GL_R,GL_TEXTURE_GEN_MODE,GL_EYE_LINEAR);
	glTexGenfv(GL_R,GL_EYE_PLANE,eyePlaneR);
	glTexGeni(GL_Q,GL_TEXTURE_GEN_MODE,GL_EYE_LINEAR);
	glTexGenfv(GL_Q,GL_EYE_PLANE,eyePlaneQ);

	glEnable(GL_TEXTURE_GEN_S);
    glEnable(GL_TEXTURE_GEN_T);
    glEnable(GL_TEXTURE_GEN_R);
    glEnable(GL_TEXTURE_GEN_Q);

	//create the light map 
	data = static_cast<GLubyte *>(malloc(texWidth * texHeight * 4 * sizeof(GLubyte)));
	spotTextureIterator = data;

	for (int j = 0; j < texHeight; ++j) {
      float dy = (texHeight * 0.5f - j + 0.5f) / (texHeight * 0.5f);

      for (int i = 0; i < texWidth; ++i) {
        float dx = (texWidth * 0.5f - i + 0.5f) / (texWidth * 0.5f);
        float r = cos(PI / 2.0f * sqrt(dx * dx + dy * dy));
        float c;

        r = (r < 0) ? 0 : r * r;
        c = 0xff * (r + borderColor[0]);
        spotTextureIterator[0] = (c <= 0xff) ? c : 0xff;
        c = 0xff * (r + borderColor[1]);
        spotTextureIterator[1] = (c <= 0xff) ? c : 0xff;
        c = 0xff * (r + borderColor[2]);
        spotTextureIterator[2] = (c <= 0xff) ? c : 0xff;
        c = 0xff * (r + borderColor[3]);
        spotTextureIterator[3] = (c <= 0xff) ? c : 0xff;
        spotTextureIterator += 4;
      }
    }

	//now we can generate the texture
	glGenTextures(1,&projTexture);

	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameterfv(GL_TEXTURE_2D,GL_TEXTURE_BORDER_COLOR,borderColor);
	gluBuild2DMipmaps(GL_TEXTURE_2D,4,texWidth,texHeight,GL_RGBA,GL_UNSIGNED_BYTE,data);
}
*/

void TexProj::loadLightMapTexture(const char *name)
{
	GLfloat eyePlaneS[] =  {1.0f, 0.0f, 0.0f, 0.0f};
	GLfloat eyePlaneT[] =  {0.0f, 1.0f, 0.0f, 0.0f};
	GLfloat eyePlaneR[] =  {0.0f, 0.0f, 1.0f, 0.0f};
	GLfloat eyePlaneQ[] =  {0.0f, 0.0f, 0.0f, 1.0f};
    GLfloat borderColor[4] = {0.6f, 0.6f, 0.6f, 1.0f};

	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);
	glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);

	//set up texture generation mode and set the corresponding planes
	glTexGeni(GL_S,GL_TEXTURE_GEN_MODE,GL_EYE_LINEAR);
	glTexGenfv(GL_S,GL_EYE_PLANE,eyePlaneS);
	glTexGeni(GL_T,GL_TEXTURE_GEN_MODE,GL_EYE_LINEAR);
	glTexGenfv(GL_T,GL_EYE_PLANE,eyePlaneT);
	glTexGeni(GL_R,GL_TEXTURE_GEN_MODE,GL_EYE_LINEAR);
	glTexGenfv(GL_R,GL_EYE_PLANE,eyePlaneR);
	glTexGeni(GL_Q,GL_TEXTURE_GEN_MODE,GL_EYE_LINEAR);
	glTexGenfv(GL_Q,GL_EYE_PLANE,eyePlaneQ);

	glEnable(GL_TEXTURE_GEN_S);
    glEnable(GL_TEXTURE_GEN_T);
    glEnable(GL_TEXTURE_GEN_R);
    glEnable(GL_TEXTURE_GEN_Q);

    AUX_RGBImageRec *Image;
    Image = auxDIBImageLoad(name);
    glGenTextures(1,&projTexture);

	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameterfv(GL_TEXTURE_2D,GL_TEXTURE_BORDER_COLOR,borderColor);

    gluBuild2DMipmaps(GL_TEXTURE_2D,GL_RGB,Image->sizeX,Image->sizeY,GL_RGB,GL_UNSIGNED_BYTE,Image->data);

    if(Image)
    {
        if(Image->data)
            free(Image->data);
        free(Image);
    }
}

void TexProj::textureProjection(Matrix4x4f &mv)
{
	Matrix4x4f &inverseMV = Matrix4x4f::invertMatrix(mv);

	//here is where we do the transformations on the texture matrix for the lightmap
	//the basic equation here is M = Bias * Scale * ModelView for light map * Inverse Modelview 
	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();
	glTranslatef(0.5f,0.5f,0.0f); //Bias
	glScalef(0.5f,0.5f,1.0f);		//Scale
	glFrustum(-0.035,0.035,-0.035,0.035,0.1,1.9); //MV for light map
	glTranslatef(0.0f,0.0f,-1.0f);
	glMultMatrixf(inverseMV.getMatrix());	//Inverse ModelView
	glMatrixMode(GL_MODELVIEW);
}

void TexProj::BindProjectiveTexture(bool exp)
{
	if(exp)
		glBindTexture(GL_TEXTURE_2D, projTexture);
	else
		glBindTexture(GL_TEXTURE_2D, texObject);
}