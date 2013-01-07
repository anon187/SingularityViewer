/**
 * @file awavefront.h
 * @brief A system which allows saving in-world objects to Wavefront .OBJ files for offline texturizing/shading.
 * @author Apelsin
 * 
 * $LicenseInfo:firstyear=2011&license=WTFPLV2$
 *  
 */

#include "llviewerprecompiledheaders.h"
#include "llvoavatar.h"
#include "llvolume.h"
#include "llface.h"
#include "m4math.h"
#include "xform.h"
// system library includes
#include <iostream>

class Wavefront
{
public:
	struct tri
	{
		S32 v0;
		S32 v1;
		S32 v2;
	};
	typedef std::vector< std::pair< LLVector3, LLVector2 > > vert_t;
	typedef std::vector< LLVector3 > vec3_t;
	typedef std::vector< tri > tri_t;
	vert_t vertices;
	vec3_t normals; //null unless otherwise specified!
	tri_t triangles; //because almost all surfaces in SL are triangles
	std::string name;
	Wavefront(vert_t v, tri_t t);
	Wavefront(LLVolumeFace *fayse, LLXform *transform = NULL, bool include_normals = true, LLXform *transform_normals = NULL);
	//Wavefront(LLPolyMesh *mesh, LLXform *transform = NULL);
	Wavefront(LLFace *fayse, LLPolyMesh *mesh = NULL, LLXform *transform = NULL, bool include_normals = true, LLXform *transform_normals = NULL);
	static void Transform(vert_t &v, LLXform *x); //helper function
	static void Transform(vec3_t &v, LLXform *x); //helper function
};
class WavefrontSaver
{
public:
	static std::string eyeLname;
	static std::string eyeRname;
	static bool swapYZ;
	typedef Wavefront::vert_t vert_t;
	typedef Wavefront::vec3_t vec3_t;
	typedef Wavefront::tri_t tri_t;
	typedef std::vector<Wavefront*> wave_t;
	wave_t obj_v;
	LLVector3 offset;
	F32 scale;
	WavefrontSaver();
	void Add(Wavefront *obj);
	void Add(LLVolume *vol, LLXform *transform = NULL, bool include_normals = true, LLXform *transform_normals = NULL);
	void Add(LLViewerObject *some_vo);
	void Add(LLVOAvatar *av_vo);
	BOOL saveFile(LLFILE *fp, int index = 0);
};
class v4adapt
{
private:
	LLStrider<LLVector4a> mV4aStrider;
public:
	v4adapt(LLVector4a* vp);
	inline LLVector3 operator[] (const unsigned int i)
	{
		return LLVector3((F32*)&mV4aStrider[i]);
	}
};