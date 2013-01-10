// <edit>


#ifndef LL_LLFLOATERHEX_H
#define LL_LLFLOATERHEX_H

#include "llfloater.h"
#include "llhexeditor.h"
#include "llinventory.h"
#include "llviewertexture.h"
#include "llassettype.h"

class LLFloaterHex
: public LLFloater
{
public:
	LLFloaterHex(LLUUID item_id, BOOL vfs=false, LLAssetType::EType asset_type = LLAssetType::AT_NONE);
	static void show(LLUUID item_id, BOOL vfs=false, LLAssetType::EType asset_type = LLAssetType::AT_NONE);
	BOOL postBuild(void);
	void close(bool app_quitting);
	static void imageCallback(BOOL success, 
					LLViewerFetchedTexture *src_vi,
					LLImageRaw* src, 
					LLImageRaw* aux_src, 
					S32 discard_level,
					BOOL final,
					void* userdata);
	static void assetCallback(LLVFS *vfs,
				   const LLUUID& asset_uuid,
				   LLAssetType::EType type,
				   void* user_data, S32 status, LLExtStat ext_status);
	static void onClickSave(void* user_data);
	static void onClickUpload(void* user_data);
	static void onSaveComplete(const LLUUID& asset_uuid, void* user_data, S32 status, LLExtStat ext_status);
	static void onCommitColumnCount(LLUICtrl *control, void *user_data);
	void handleSizing();
	void readVFile();
	LLInventoryItem* mItem;
	LLUUID mAssetId;
	LLAssetType::EType mAssetType;
	BOOL mVFS;
	LLHexEditor* mEditor;
	static std::list<LLFloaterHex*> sInstances;
private:
	virtual ~LLFloaterHex();
protected:
	static S32 sUploadAmount;
};

#endif
// </edit>
