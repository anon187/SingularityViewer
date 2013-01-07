/**
 * @file awavefront.cpp
 * @brief A system which allows saving in-world objects to Wavefront .OBJ files for offline texturizing/shading.
 * @author Apelsin
 * 
 * $LicenseInfo:firstyear=2011&license=WTFPLV2$
 *  
 */
#include "llviewerprecompiledheaders.h"
#include "awavefront.h"
#include "llviewercontrol.h"
#include "llvoavatardefines.h"
#include "llvolume.h"
#include "llvolumemgr.h"
#include "llvovolume.h"
#include "llface.h"
// system library includes
#include <fstream>
#include <sstream>

Wavefront::Wavefront(vert_t v, tri_t t)
{
	name = "";
	vertices = v;
	triangles = t;
}
Wavefront::Wavefront(LLVolumeFace *fayse, LLXform *transform, bool include_normals, LLXform *transform_normals)
	:name("")
{
	v4adapt verts = v4adapt(fayse->mPositions);
	v4adapt norms = v4adapt(fayse->mNormals);
	for (S32 i = 0; i < fayse->mNumVertices; i++)
	{
		LLVector3 v = verts[i];
		vertices.push_back(std::pair<LLVector3, LLVector2>(v, fayse->mTexCoords[i]));
	}
	if(transform)
		Transform(vertices, transform);
	if(include_normals)
	{
		for (S32 i = 0; i < fayse->mNumVertices; i++)
			normals.push_back(norms[i]);
		if(transform_normals)
			Transform(normals, transform_normals);
	}
	for (S32 i = 0; i < fayse->mNumIndices/3; i++) 
	{
		tri t;
		t.v0 = fayse->mIndices[i*3+0];
		t.v1 = fayse->mIndices[i*3+1];
		t.v2 = fayse->mIndices[i*3+2];
		triangles.push_back(t);
	}
}
/*Wavefront::Wavefront(LLPolyMesh *mesh, LLXform *transform)
{
	name = "";
	vertices = vert_t();
	triangles = tri_t();
	for (U32 i = 0; i < mesh->getNumVertices(); i++)
	{
		LLVector3 v = mesh->getCoords()[i];
		vertices.push_back(std::pair<LLVector3, LLVector2>(v, mesh->getDetailTexCoords()[i]));
	}
	if(transform)
		Transform(vertices, transform);
	for (int i = 0; i < mesh->getNumFaces(); i++) 
	{	//for each triangle
		tri t;
		t.v0 = mesh->getFaces()[i][0];
		t.v1 = mesh->getFaces()[i][1];
		t.v2 = mesh->getFaces()[i][2];
		triangles.push_back(t);
	}
}*/
Wavefront::Wavefront(LLFace *fayse, LLPolyMesh *mesh, LLXform *transform, bool include_normals, LLXform *transform_normals)
	:name("")
{
	LLStrider<LLVector3> getVerts = LLStrider<LLVector3>();
	LLStrider<LLVector3> getNorms= LLStrider<LLVector3>();
	LLStrider<LLVector2> getCoord = LLStrider<LLVector2>();
	LLStrider<U16> getIndices = LLStrider<U16>();
	//LLFacePool *pool = fayse->getPool();
	//if(is_avatar)
	//LLDrawPoolAvatar *avatarPoolp = (LLDrawPoolAvatar *)fayse->getPool();
	LLVertexBuffer *vb = fayse->getVertexBuffer();
	if(!vb)
		return;
	fayse->getGeometry(getVerts, getNorms, getCoord, getIndices);
	U16 vcount = mesh?mesh->getNumVertices():vb->getNumVerts(); //vertices
	U32 pcount = mesh?mesh->getNumFaces():(vb->getNumIndices()/3); //indices
	U16 start = fayse->getGeomStart(); //vertices
	U16 end = start + vcount - 1; //vertices
	U16 offset = fayse->getIndicesStart(); //indices
	for(int i = start; i <= end; i++)
		vertices.push_back(std::make_pair(getVerts[i], getCoord[i]));
	if(transform)
		Transform(vertices, transform);
	if(include_normals)
	{
		for(int i = start; i <= end; i++)
			normals.push_back(getNorms[i]);
		if(transform_normals)
			Transform(normals, transform_normals);
	}
	for(U32 i = 0; i < pcount; i++)
	{
		tri triangle;
		triangle.v0 = getIndices[i * 3  + offset] + start;
		triangle.v1 = getIndices[i * 3 + 1 + offset] + start;
		triangle.v2 = getIndices[i * 3 + 2 + offset] + start;
		triangles.push_back(triangle);
	}
}
void Wavefront::Transform(vert_t &v, LLXform *x) //recursive
{
	LLMatrix4 m = LLMatrix4();
	x->getLocalMat4(m);
	for(vert_t::iterator iterv = v.begin();
		iterv != v.end(); iterv++)
	{
		iterv->first = iterv->first * m;
	}
	if(x->getParent())
		Transform(v, x->getParent());
	return;
}
void Wavefront::Transform(vec3_t &v, LLXform *x) //recursive
{
	LLMatrix4 m = LLMatrix4();
	x->getLocalMat4(m);
	for(vec3_t::iterator iterv = v.begin();
		iterv != v.end(); iterv++)
	{
		*iterv = *iterv * m;
	}
	if(x->getParent())
		Transform(v, x->getParent());
	return;
}

std::string WavefrontSaver::eyeLname = "eyeBallLeftMesh";
std::string WavefrontSaver::eyeRname = "eyeBallRightMesh";
bool WavefrontSaver::swapYZ = false;

WavefrontSaver::WavefrontSaver()
	:scale(1.0)
{
	//By the time this constructor is called, it should be safe to do this...
	eyeLname = LLVOAvatarDefines::LLVOAvatarDictionary::getInstance()->
		getMesh(LLVOAvatarDefines::MESH_ID_EYEBALL_LEFT)->mName;
	eyeRname = LLVOAvatarDefines::LLVOAvatarDictionary::getInstance()->
		getMesh(LLVOAvatarDefines::MESH_ID_EYEBALL_RIGHT)->mName;
	swapYZ = gSavedSettings.getBOOL("OBJExportSwapYZ");

}
void WavefrontSaver::Add(Wavefront *obj)
{
	if(obj)
		obj_v.push_back(obj);
}
void WavefrontSaver::Add(LLVolume *vol, LLXform *transform, bool include_normals, LLXform *transform_normals)
{
	int faces = vol->getNumVolumeFaces();
	for(int i = 0; i < faces; i++) //each face will be treated as a separate Wavefront object
	{
		LLVolumeFace fayse = vol->getVolumeFace(i);
		Add(new Wavefront(&fayse, transform, include_normals, transform_normals));
	}
}
void WavefrontSaver::Add(LLViewerObject *some_vo)
{
	LLVolume *vol = some_vo->getVolume();
	LLXform * v_form = new LLXform();
	v_form->setScale(some_vo->getScale());
	v_form->setPosition(some_vo->getRenderPosition());
	v_form->setRotation(some_vo->getRenderRotation());
	LLXform *normfix = new LLXform();
	normfix->setRotation(v_form->getRotation()); //Should work...
	Add(vol, v_form, true, normfix);
}
void WavefrontSaver::Add(LLVOAvatar *av_vo) //adds attachments, too!
{
	offset = -av_vo->getRenderPosition();
	std::vector<LLViewerJoint *> vjv = av_vo->mMeshLOD;
	for(std::vector<LLViewerJoint *>::iterator itervj = vjv.begin();
		itervj != vjv.end(); itervj++)
	{
		LLViewerJoint *vj = *itervj;
		if(!vj)
			continue;
		if(vj->mMeshParts.size() == 0)
			continue;
		LLViewerJointMesh *vjm = vj->mMeshParts[0]; //highest LOD
		if(!vjm)
			continue;
		vjm->updateJointGeometry();
		LLFace *face = vjm->mFace;
		if(!face)
			continue;
		//Beware: this is a hack because LLFace has multiple LODs
		//'pm' supplies the right number of vertices and triangles!
		LLPolyMesh *pm = vjm->getMesh();
		if(!pm)
			continue;
		LLXform *normfix = new LLXform();
		normfix->setRotation(pm->getRotation());
		//Special case for balls...I mean eyeballs!
		if(vj->getName() == eyeLname || vj->getName() == eyeRname) 
		{
			LLXform *lol = new LLXform();
			lol->setPosition(-offset);
			Add(new Wavefront(face, pm, lol, true, normfix));
		}
		else
			Add(new Wavefront(face, pm, NULL, true, normfix));
	}
	//LLDynamicArray<LLViewerObject*> av_arr = LLDynamicArray<LLViewerObject*>();
	//av_vo->addThisAndAllChildren(av_arr);
	//for(LLDynamicArray<LLViewerObject*>::iterator iter = av_arr.begin();
	//	iter != av_arr.end(); iter++)
	for(LLVOAvatar::attachment_map_t::iterator iter = av_vo->mAttachmentPoints.begin();
		iter != av_vo->mAttachmentPoints.end(); iter++)
	{
		LLViewerJointAttachment *ja = iter->second;
		if(!ja)
			continue;
		for(LLViewerJointAttachment::attachedobjs_vec_t::iterator itero = ja->mAttachedObjects.begin();
			itero != ja->mAttachedObjects.end(); itero++)
		{
			LLViewerObject *o = *itero;
			if(!o)
				continue;
			LLDynamicArray<LLViewerObject*> prims = LLDynamicArray<LLViewerObject*>();
			o->addThisAndAllChildren(prims);
			for(LLDynamicArray<LLViewerObject*>::iterator iterc = prims.begin();
				iterc != prims.end(); iterc++)
			{
				LLViewerObject *c = *iterc;
				if(!c)
					continue;
				LLVolume *vol = c->getVolume();
				if(!vol)
					continue;
				LLXform * v_form = new LLXform();
				v_form->setScale(c->getScale());
				v_form->setPosition(c->getRenderPosition());
				v_form->setRotation(c->getRenderRotation());
				LLXform *normfix = new LLXform();
				normfix->setRotation(v_form->getRotation());
				if(c->isHUDAttachment()) 
					v_form->addPosition(-offset);
				//Normals of HUD elements are funky
				//TO-DO: fix 'em!
				Add(vol, v_form, true, normfix);
			}
		}
	}
}

BOOL WavefrontSaver::saveFile(LLFILE *fp, int index)
{
	if (!fp)
		return FALSE;
	int num = 0;
	for(wave_t::iterator w_iter = obj_v.begin(); w_iter != obj_v.end(); w_iter++)
	{
		int count = 0;
		std::string name = (*w_iter)->name;
		vert_t vertices = (*w_iter)->vertices;
		vec3_t normals = (*w_iter)->normals;
		tri_t triangles = (*w_iter)->triangles;

		//Write Object
		if(name == "")
			name = llformat("%d", num++);
		std::string outstring = llformat("o %s\n", name.c_str());
		if (fwrite(outstring.c_str(), 1, outstring.length(), fp) != outstring.length())
			llwarns << "Short write" << llendl;		
		//Write vertices; swap axes if necessary
		double xm = swapYZ?-1.0:1.0;
		int y = swapYZ?2:1;
		int z = swapYZ?1:2;
		for (vert_t::iterator v_iter = vertices.begin(); v_iter != vertices.end(); v_iter++) {
			count++;
			LLVector3 v = v_iter->first + offset;
			std::string outstring = llformat("v %f %f %f\n",v[0] * scale * xm,
															v[y] * scale,
															v[z] * scale);
			if (fwrite(outstring.c_str(), 1, outstring.length(), fp) != outstring.length())
				llwarns << "Short write" << llendl;
		}
		for (vec3_t::iterator n_iter = normals.begin(); n_iter != normals.end(); n_iter++) {
			LLVector3 n = *n_iter;
			std::string outstring = llformat("vn %f %f %f\n",n[0] * xm,
															 n[y],
															 n[z]);
			if (fwrite(outstring.c_str(), 1, outstring.length(), fp) != outstring.length())
				llwarns << "Short write" << llendl;
		}
		for (vert_t::iterator v_iter = vertices.begin(); v_iter != vertices.end(); v_iter++) {
			std::string outstring = llformat("vt %f %f\n",	v_iter->second[0],
															v_iter->second[1]);
			if (fwrite(outstring.c_str(), 1, outstring.length(), fp) != outstring.length())
				llwarns << "Short write" << llendl;
		}
		//Write triangles
		for (tri_t::iterator t_iter = triangles.begin(); t_iter != triangles.end(); t_iter++) {
			S32 f1 = t_iter->v0 + index + 1;
			S32 f2 = t_iter->v1 + index + 1;
			S32 f3 = t_iter->v2 + index + 1;
			std::string outstring = llformat("f %d/%d/%d %d/%d/%d %d/%d/%d\n",	f1, f1, f1,
																				f2, f2, f2,
																				f3, f3, f3);
			if (fwrite(outstring.c_str(), 1, outstring.length(), fp) != outstring.length())
				llwarns << "Short write" << llendl;
		}
		index += count;
	}
	
	return TRUE;
}
v4adapt::v4adapt(LLVector4a* vp)
{
	mV4aStrider = vp;
}