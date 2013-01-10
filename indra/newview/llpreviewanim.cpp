/** 
 * @file llpreviewanim.cpp
 * @brief LLPreviewAnim class implementation
 *
 * $LicenseInfo:firstyear=2004&license=viewergpl$
 * 
 * Copyright (c) 2004-2009, Linden Research, Inc.
 * 
 * Second Life Viewer Source Code
 * The source code in this file ("Source Code") is provided by Linden Lab
 * to you under the terms of the GNU General Public License, version 2.0
 * ("GPL"), unless you have obtained a separate licensing agreement
 * ("Other License"), formally executed by you and Linden Lab.  Terms of
 * the GPL can be found in doc/GPL-license.txt in this distribution, or
 * online at http://secondlifegrid.net/programs/open_source/licensing/gplv2
 * 
 * There are special exceptions to the terms and conditions of the GPL as
 * it is applied to this Source Code. View the full text of the exception
 * in the file doc/FLOSS-exception.txt in this software distribution, or
 * online at
 * http://secondlifegrid.net/programs/open_source/licensing/flossexception
 * 
 * By copying, modifying or distributing this software, you acknowledge
 * that you have read and understood your obligations described above,
 * and agree to abide by those obligations.
 * 
 * ALL LINDEN LAB SOURCE CODE IS PROVIDED "AS IS." LINDEN LAB MAKES NO
 * WARRANTIES, EXPRESS, IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY,
 * COMPLETENESS OR PERFORMANCE.
 * $/LicenseInfo$
 */

#include "llviewerprecompiledheaders.h"

#include "llpreviewanim.h"
#include "llbutton.h"
#include "llfloaterinventory.h"
#include "llresmgr.h"
#include "llinventory.h"
#include "llvoavatarself.h"
#include "llagent.h"          // gAgent
#include "llkeyframemotion.h"
#include "statemachine/aifilepicker.h"
#include "lllineeditor.h"
#include "lluictrlfactory.h"
#include "lluictrlfactory.h"
// <edit>
#include "llviewerwindow.h" // for alert
#include "llappviewer.h" // gStaticVFS
#include "llviewermenufile.h"
#include "llassetstorage.h"
#include "tsbvhexporter.h"
void cmdline_printchat(std::string message);
// </edit>

extern LLAgent gAgent;

LLPreviewAnim::LLPreviewAnim(const std::string& name, const LLRect& rect, const std::string& title, const LLUUID& item_uuid, const e_activation_type& _activate, const LLUUID& object_uuid )	:
	LLPreview( name, rect, title, item_uuid, object_uuid)
{
	mIsCopyable = false;
	setTitle(title);
	LLUICtrlFactory::getInstance()->buildFloater(this,"floater_preview_animation.xml");
	if (!getHost())
	{
		LLRect curRect = getRect();
		translate(rect.mLeft - curRect.mLeft, rect.mTop - curRect.mTop);
	}
	activate(_activate);
}

// static
void LLPreviewAnim::endAnimCallback( void *userdata )
{
	LLHandle<LLFloater>* handlep = ((LLHandle<LLFloater>*)userdata);
	LLFloater* self = handlep->get();
	delete handlep; // done with the handle
	if (self)
	{
		self->childSetValue("Anim play btn", FALSE);
		self->childSetValue("Anim audition btn", FALSE);
	}
}

// virtual
BOOL LLPreviewAnim::postBuild()
{
	const LLInventoryItem* item = getItem();

	// preload the animation
	if(item)
	{
		gAgentAvatarp->createMotion(item->getAssetUUID());
		//<edit>
		gAssetStorage->getAssetData(item->getAssetUUID(), LLAssetType::AT_ANIMATION, downloadCompleteCallback, (void *)(new LLHandle<LLFloater>(this->getHandle())), TRUE);
		//< /edit>
		childSetText("desc", item->getDescription());
	
		const LLPermissions& perm = item->getPermissions();
		mIsCopyable = (perm.getCreator() == gAgent.getID());
	}

	childSetAction("Anim play btn",playAnim,this);
	childSetAction("Anim audition btn",auditionAnim,this);
	// <edit>
	childSetAction("Anim copy uuid btn", copyAnimID, this);
	childSetAction("Anim remake btn",dupliAnim,this);
	getChild<LLUICtrl>("Export_animatn")->setCommitCallback(boost::bind(&LLPreviewAnim::onBtnExport_animatn, this));
	getChild<LLUICtrl>("Export_bvh")->setCommitCallback(boost::bind(&LLPreviewAnim::onBtnExport_bvh, this));
	// < /edit>
	childSetCommitCallback("desc", LLPreview::onText, this);
	childSetPrevalidate("desc", &LLLineEditor::prevalidatePrintableNotPipe);
	
	return LLPreview::postBuild();
}
void LLPreviewAnim::activate(e_activation_type type)
{
	switch ( type ) 
	{
		case PLAY:
		{
			playAnim( (void *) this );
			break;
		}
		case AUDITION:
		{
			auditionAnim( (void *) this );
			break;
		}
		default:
		{
		//do nothing
		}
	}
}

// static
void LLPreviewAnim::playAnim( void *userdata )
{
	LLPreviewAnim* self = (LLPreviewAnim*) userdata;
	const LLInventoryItem *item = self->getItem();

	if(item)
	{
		LLUUID itemID=item->getAssetUUID();

		LLButton* btn = self->getChild<LLButton>("Anim play btn");
		if (btn)
		{
			btn->toggleState();
		}
		
		if (self->childGetValue("Anim play btn").asBoolean() ) 
		{
			self->mPauseRequest = NULL;
			gAgent.sendAnimationRequest(itemID, ANIM_REQUEST_START);
			LLMotion* motion = gAgentAvatarp->findMotion(itemID);
			if (motion)
			{
				motion->setDeactivateCallback(&endAnimCallback, (void *)(new LLHandle<LLFloater>(self->getHandle())));
			}
		}
		else
		{
			gAgentAvatarp->stopMotion(itemID);
			gAgent.sendAnimationRequest(itemID, ANIM_REQUEST_STOP);
		}
	}
}

// static
void LLPreviewAnim::auditionAnim( void *userdata )
{
	LLPreviewAnim* self = (LLPreviewAnim*) userdata;
	const LLInventoryItem *item = self->getItem();

	if(item)
	{
		LLUUID itemID=item->getAssetUUID();

		LLButton* btn = self->getChild<LLButton>("Anim audition btn");
		if (btn)
		{
			btn->toggleState();
		}
		
		if (self->childGetValue("Anim audition btn").asBoolean() ) 
		{
			self->mPauseRequest = NULL;
			gAgentAvatarp->startMotion(item->getAssetUUID());
			LLMotion* motion = gAgentAvatarp->findMotion(itemID);
			
			if (motion)
			{
				motion->setDeactivateCallback(&endAnimCallback, (void *)(new LLHandle<LLFloater>(self->getHandle())));
			}
		}
		else
		{
			gAgentAvatarp->stopMotion(itemID);
			gAgent.sendAnimationRequest(itemID, ANIM_REQUEST_STOP);
		}
	}
}

//<edit>
void LLPreviewAnim::downloadCompleteCallback(LLVFS *vfs, const LLUUID& uuid, LLAssetType::EType type, void *userdata, S32 result, LLExtStat extstat)
{
	LLHandle<LLFloater>* handlep = ((LLHandle<LLFloater>*)userdata);
	LLPreviewAnim* self = (LLPreviewAnim*)handlep->get();
	delete handlep; // done with the handle
	if (self)
	{
		if(result == LL_ERR_NOERR) {
			self->childSetEnabled("Anim remake btn", TRUE);
			self->childSetEnabled("Export_animatn", TRUE);
			self->childSetEnabled("Export_bvh", TRUE);
			self->mAnimBufferSize = vfs->getSize(uuid, type);
			self->mAnimBuffer = new U8[self->mAnimBufferSize];
			vfs->getData(uuid, type, self->mAnimBuffer, 0, self->mAnimBufferSize);
		}
	}
}

void LLPreviewAnim::dupliAnim( void *userdata )
{
		LLPreviewAnim* self = (LLPreviewAnim*) userdata;
		const LLInventoryItem *item = self->getItem();
		
		if(item)
		{
			if(self->mAnimBuffer == NULL) 
			{	
				return;
			}	
			LLKeyframeMotion* motionp = NULL;

			LLAssetID			xMotionID;
			LLTransactionID		xTransactionID;

			// generate unique id for this motion
			xTransactionID.generate();
			xMotionID = xTransactionID.makeAssetID(gAgent.getSecureSessionID());
			motionp = (LLKeyframeMotion*)gAgentAvatarp->createMotion(xMotionID);
			LLDataPackerBinaryBuffer dp(self->mAnimBuffer, self->mAnimBufferSize);
			LLVOAvatar* avatar = gAgentAvatarp;
			LLMotion*   motion = avatar->findMotion(item->getAssetUUID());
			LLKeyframeMotion* tmp = (LLKeyframeMotion*)motion;
			tmp->serialize(dp);
			dp.reset();
			BOOL success = motionp && motionp->deserialize(dp);

			if (success)
			{
			motionp->setName(item->getName());
			gAgentAvatarp->startMotion(xMotionID);
			motionp = (LLKeyframeMotion*)gAgentAvatarp->findMotion(xMotionID);

			S32 file_size = motionp->getFileSize();
			U8* buffer = new U8[file_size];

			LLDataPackerBinaryBuffer dp(buffer, file_size);
			if (motionp->serialize(dp))
			{
			  LLVFile file(gVFS, motionp->getID(), LLAssetType::AT_ANIMATION, LLVFile::APPEND);
			  LLAssetStorage::LLStoreAssetCallback callback = NULL;

				S32 size = dp.getCurrentSize();
				file.setMaxSize(size);
				if (file.write((U8*)buffer, size))
				{
					std::string name = item->getName();
					std::string desc = item->getDescription();
					upload_new_resource(xTransactionID, // tid
						    LLAssetType::AT_ANIMATION,
						    name,
						    desc,
						    0,
						    LLFolderType::FT_ANIMATION,
						    LLInventoryType::IT_ANIMATION,
							PERM_NONE,PERM_NONE,PERM_NONE,
						    name,
						    callback,0,userdata);
			}
			else
			{
				llwarns << "Failure writing animation data." << llendl;
					LLNotifications::instance().add("WriteAnimationFail");
				}
			}

			delete [] buffer;
			gAgentAvatarp->removeMotion(xMotionID);
			LLKeyframeDataCache::removeKeyframeData(xMotionID);
			}

		}

}

void LLPreviewAnim::onBtnExport_animatn()
{
	std::string default_filename("untitled.animatn");
	const LLInventoryItem *item = getItem();
	if(item)
	{
		default_filename = LLDir::getScrubbedFileName(item->getName()) + ".animatn";
	}

	AIFilePicker* filepicker = AIFilePicker::create();
	filepicker->open(default_filename, FFSAVE_ANIMATN);
	filepicker->run(boost::bind(&LLPreviewAnim::onBtnExport_animatn_continued, this, filepicker));
}

void LLPreviewAnim::onBtnExport_animatn_continued(void* userdata, AIFilePicker* filepicker)
{
	LLPreviewAnim* self = (LLPreviewAnim*) userdata;
	if(!filepicker->hasFilename())
		return;
	S32 file_size;
	LLAPRFile infile(filepicker->getFilename().c_str(), LL_APR_WB, &file_size);
	apr_file_t *fp = infile.getFileHandle();
	if(fp)infile.write(self->mAnimBuffer, self->mAnimBufferSize);
		infile.close();
		std::string filename = filepicker->getFilename().c_str();
		#if LL_WINDOWS
		std::string name = filename.substr(filename.find_last_of("\\")+1);
		#else
		std::string name = filename.substr(filename.find_last_of("/")+1);
		#endif
		cmdline_printchat("Saved " + name);
}

// static
void LLPreviewAnim::onBtnExport_bvh()
{
	std::string default_filename("untitled.bvh");
	const LLInventoryItem *item = getItem();
	if(item)
	{
		default_filename = LLDir::getScrubbedFileName(item->getName()) + ".bvh";
	}

	AIFilePicker* filepicker = AIFilePicker::create();
	filepicker->open(default_filename, FFSAVE_ANIM);
	filepicker->run(boost::bind(&LLPreviewAnim::onBtnExport_bvh_continued, this, filepicker));
}

void LLPreviewAnim::onBtnExport_bvh_continued(void* userdata, AIFilePicker* filepicker)
{
	LLPreviewAnim* self = (LLPreviewAnim*) userdata;
	if(!filepicker->hasFilename())
		return;

	TSBVHExporter exporter;
		LLDataPackerBinaryBuffer dp(self->mAnimBuffer, self->mAnimBufferSize);

		if(exporter.deserialize(dp)) exporter.exportBVHFile(filepicker->getFilename().c_str());

		std::string filename = filepicker->getFilename().c_str();
		#if LL_WINDOWS
		std::string name = filename.substr(filename.find_last_of("\\")+1);
		#else
		std::string name = filename.substr(filename.find_last_of("/")+1);
		#endif
		cmdline_printchat("Saved " + name);
}

//< /edit>
void LLPreviewAnim::copyAnimID(void *userdata)
{
	LLPreviewAnim* self = (LLPreviewAnim*) userdata;
	const LLInventoryItem *item = self->getItem();

	if(item)
	{
		gViewerWindow->mWindow->copyTextToClipboard(utf8str_to_wstring(item->getAssetUUID().asString()));
		//<edit>
		cmdline_printchat(item->getAssetUUID().asString());//< /edit>
		//< /edit>
	}
}
// virtual
BOOL LLPreviewAnim::canSaveAs() const
{
    //<edit>
	//return mIsCopyable;
	return TRUE;
	//< /edit>
}

// virtual
void LLPreviewAnim::saveAs()
{
	const LLInventoryItem *item = getItem();

	if(item)
	{
		// Some animations aren't hosted on the servers
		// I guess they're in this static vfs thing
		//<edit>
		//bool static_vfile = false;
		bool static_vfile = true;
		//< /edit>
		LLVFile* anim_file = new LLVFile(gStaticVFS, item->getAssetUUID(), LLAssetType::AT_ANIMATION);
		if (anim_file && anim_file->getSize())
		{
			static_vfile = true; // for method 2
			LLPreviewAnim::gotAssetForSave(gStaticVFS, item->getAssetUUID(), LLAssetType::AT_ANIMATION, this, 0, 0);
		}
		delete anim_file;
		anim_file = NULL;

		if(!static_vfile)
		{
			gAssetStorage->getAssetData(item->getAssetUUID(), LLAssetType::AT_ANIMATION, LLPreviewAnim::gotAssetForSave, this, TRUE);
		}
	}
}

// static
void LLPreviewAnim::gotAssetForSave(LLVFS *vfs,
									   const LLUUID& asset_uuid,
									   LLAssetType::EType type,
									   void* user_data, S32 status, LLExtStat ext_status)
{
	LLPreviewAnim* self = (LLPreviewAnim*) user_data;
	//const LLInventoryItem *item = self->getItem();

	LLVFile file(vfs, asset_uuid, type, LLVFile::READ);
	S32 size = file.getSize();

	char* buffer = new char[size];
	if (buffer == NULL)
	{
		llerrs << "Memory Allocation Failed" << llendl;
		return;
	}

	file.read((U8*)buffer, size);

	// Write it back out...

	AIFilePicker* filepicker = AIFilePicker::create();
	filepicker->open(LLDir::getScrubbedFileName(self->getItem()->getName()) + ".animatn", FFSAVE_ANIMATN);
	filepicker->run(boost::bind(&LLPreviewAnim::gotAssetForSave_continued, buffer, size, filepicker));
}

// static
void LLPreviewAnim::gotAssetForSave_continued(char* buffer, S32 size, AIFilePicker* filepicker)
{
	if (filepicker->hasFilename())
	{
		std::string filename = filepicker->getFilename();
		std::ofstream export_file(filename.c_str(), std::ofstream::binary);
		export_file.write(buffer, size);
		export_file.close();
	}
	delete[] buffer;
}

// virtual
LLUUID LLPreviewAnim::getItemID()
{
	const LLViewerInventoryItem* item = getItem();
	if(item)
	{
		return item->getUUID();
	}
	return LLUUID::null;
}
// </edit>

void LLPreviewAnim::onClose(bool app_quitting)
{
	const LLInventoryItem *item = getItem();

	if(item)
	{
		gAgentAvatarp->stopMotion(item->getAssetUUID());
		gAgent.sendAnimationRequest(item->getAssetUUID(), ANIM_REQUEST_STOP);
		LLMotion* motion = gAgentAvatarp->findMotion(item->getAssetUUID());
		
		if (motion)
		{
			// *TODO: minor memory leak here, user data is never deleted (Use real callbacks)
			motion->setDeactivateCallback(NULL, (void *)NULL);
		}
	}
	//<edit>
	if(mAnimBuffer) {
		delete mAnimBuffer;
		mAnimBuffer = NULL;
	}
	//< /edit>
	destroy();
}
