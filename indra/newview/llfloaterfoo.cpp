#include "llviewerprecompiledheaders.h" // must be first include

#include "llfloaterfoo.h"

#include "lluictrlfactory.h" // builds floaters from XML

// Statics
LLFloaterFoo* LLFloaterFoo::sInstance = NULL;

LLFloaterFoo::LLFloaterFoo()
:   LLFloater(std::string("floater_foo"))
{
    LLUICtrlFactory::getInstance()->buildFloater(this, "floater_foo.xml");

    childSetCommitCallback("baz_edit", onCommitBaz, this);

    childSetAction("hippo_btn", onClickHippo, this);
    setDefaultBtn("hippo_btn");
}

// static
void LLFloaterFoo::show(void*)
{
    if (!sInstance)
	sInstance = new LLFloaterFoo();

    sInstance->open();
}

LLFloaterFoo::~LLFloaterFoo()
{
    sInstance=NULL;
}

// static
void LLFloaterFoo::onCommitBaz(LLUICtrl* ctrl, void* userdata)
{
    LLFloaterFoo* self = (LLFloaterFoo*)userdata;
    std::string text = self->childGetText("baz_edit");
    llinfos << "baz contains: " << text << llendl;
}

// static
void LLFloaterFoo::onClickHippo(void* userdata)
{
    LLFloaterFoo* self = (LLFloaterFoo*)userdata;
    llinfos << "Hippo! from " << self->getName() << llendl;
}