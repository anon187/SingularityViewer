#include "llfloater.h"
 
class LLFloaterFoo : public LLFloater
{
public:
    LLFloaterFoo();

    virtual ~LLFloaterFoo();

    // by convention, this shows the floater and does instance management
    static void show(void*);
 
private:
    // when a line editor loses keyboard focus, it is committed.
    // commit callbacks are named onCommitWidgetName by convention.
    static void onCommitBaz(LLUICtrl* ctrl, void *userdata);
 
    // by convention, button callbacks are named onClickButtonLabel
    static void onClickHippo(void* userdata);
 
    // no pointers to widgets here - they are referenced by name

    //assuming we just need one, which is typical
    static LLFloaterFoo* sInstance;

};