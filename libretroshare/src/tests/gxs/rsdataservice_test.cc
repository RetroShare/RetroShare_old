
#include "support.h"
#include "data_support.h"
#include "rsdataservice_test.h"
#include "gxs/rsdataservice.h"

#define DATA_BASE_NAME "msg_grp_Store"

INITTEST();
RsGeneralDataService* dStore = NULL;

void setUp();
void tearDown();

int main()
{

    std::cerr << "RsDataService Tests" << std::endl;

    test_groupStoreAndRetrieve(); REPORT("test_groupStoreAndRetrieve");
    test_messageStoresAndRetrieve(); REPORT("test_messageStoresAndRetrieve");

    FINALREPORT("RsDataService Tests");

    return TESTRESULT();

}



/*!
 * All memory is disposed off, good for looking
 * for memory leaks
 */
void test_groupStoreAndRetrieve(){

    setUp();

    int nGrp = rand()%32;
    std::map<RsNxsGrp*, RsGxsGrpMetaData*> grps;
    RsNxsGrp* grp;
    RsGxsGrpMetaData* grpMeta;
    for(int i = 0; i < nGrp; i++){
        std::pair<RsNxsGrp*, RsGxsGrpMetaData*> p;
       grp = new RsNxsGrp(RS_SERVICE_TYPE_PLUGIN_SIMPLE_FORUM);
       grpMeta = new RsGxsGrpMetaData();
       p.first = grp;
       p.second = grpMeta;
       init_item(*grp);
       init_item(grpMeta);
       grpMeta->mGroupId = grp->grpId;
       grps.insert(p);

       grpMeta = NULL;
       grp = NULL;
   }

    dStore->storeGroup(grps);

    std::map<std::string, RsNxsGrp*> gR;
    std::map<std::string, RsGxsGrpMetaData*> grpMetaR;
    dStore->retrieveNxsGrps(gR, false);
    dStore->retrieveGxsGrpMetaData(grpMetaR);

    std::map<RsNxsGrp*, RsGxsGrpMetaData*>::iterator mit = grps.begin();

    bool grpMatch = true, grpMetaMatch = true;

    for(; mit != grps.end(); mit++)
    {
        const std::string grpId = mit->first->grpId;

        // check if it exists
        if(gR.find(grpId) == gR.end()) {
            grpMatch = false;
            break;
        }

        RsNxsGrp *l = mit->first,
        *r = gR[grpId];

        // assign transaction number
        // to right to as tn is not stored
        // in db
        r->transactionNumber = l->transactionNumber;

        // then do a comparison
        if(!( *l == *r)) {
            grpMatch = false;
            break;
        }

        // now do a comparison of grp meta types

        if(grpMetaR.find(grpId) == grpMetaR.end())
        {
            grpMetaMatch = false;
            break;
        }

        RsGxsGrpMetaData *l_Meta = mit->second,
        *r_Meta = grpMetaR[grpId];

        if(!(*l_Meta == *r_Meta))
        {
            grpMetaMatch = false;
            break;
        }

        /* release resources */
        delete l_Meta;
        delete r_Meta;
        delete l;
        delete r;

        remove(grpId.c_str());
    }

    grpMetaR.clear();

    CHECK(grpMatch);
    tearDown();
}

/*!
 * Test for both selective and
 * bulk msg retrieval
 */
void test_messageStoresAndRetrieve()
{
    setUp();

    // first create a grpId
    std::string grpId0, grpId1;

    randString(SHORT_STR, grpId0);
    randString(SHORT_STR, grpId1);
    std::vector<std::string> grpV; // stores grpIds of all msgs stored and retrieved
    grpV.push_back(grpId0);
    grpV.push_back(grpId1);

    std::map<RsNxsMsg*, RsGxsMsgMetaData*> msgs;
    RsNxsMsg* msg = NULL;
    RsGxsMsgMetaData* msgMeta = NULL;
    int nMsgs = rand()%120;
    GxsMsgReq req;

    std::map<std::string, RsNxsMsg*> VergrpId0, VergrpId1;
    std::map<std::string, RsGxsMsgMetaData*> VerMetagrpId0, VerMetagrpId1;

    for(int i=0; i<nMsgs; i++)
    {
        msg = new RsNxsMsg(RS_SERVICE_TYPE_PLUGIN_SIMPLE_FORUM);
        msgMeta = new RsGxsMsgMetaData();
        init_item(*msg);
        init_item(msgMeta);
        std::pair<RsNxsMsg*, RsGxsMsgMetaData*> p(msg, msgMeta);
        int chosen = 0;
        if(rand()%50 > 24){
            chosen = 1;

        }

        const std::string& grpId = grpV[chosen];

        if(chosen)
            req[grpId].insert(msg->msgId);

        msgMeta->mMsgId = msg->msgId;
        msgMeta->mGroupId = msg->grpId = grpId;

        // store msgs in map to use for verification
        std::pair<std::string, RsNxsMsg*> vP(msg->msgId, msg);
        std::pair<std::string, RsGxsMsgMetaData*> vPmeta(msg->msgId, msgMeta);

        if(!chosen)
        {
            VergrpId0.insert(vP);
            VerMetagrpId0.insert(vPmeta);
        }
        else
        {
            VergrpId1.insert(vP);
            VerMetagrpId0.insert(vPmeta);
        }
        msg = NULL;
        msgMeta = NULL;

        msgs.insert(p);
    }

    req[grpV[0]] = std::set<std::string>(); // assign empty list for other

    dStore->storeMessage(msgs);

    // now retrieve msgs for comparison
    // first selective retrieval

    GxsMsgResult msgResult;
    GxsMsgMetaResult msgMetaResult;
    dStore->retrieveNxsMsgs(req, msgResult, false);
    dStore->retrieveGxsMsgMetaData(grpV, msgMetaResult);

    // now look at result for grpId 1
    std::vector<RsNxsMsg*>& result0 = msgResult[grpId0];
    std::vector<RsNxsMsg*>& result1 = msgResult[grpId1];
    std::vector<RsGxsMsgMetaData*>& resultMeta0 = msgMetaResult[grpId0];
    std::vector<RsGxsMsgMetaData*>& resultMeta1 = msgMetaResult[grpId1];



    bool msgGrpId0_Match = true, msgGrpId1_Match = true;
    bool msgMetaGrpId0_Match = true, msgMetaGrpId1_Match = true;

    // MSG test, selective retrieval
    for(std::vector<RsNxsMsg*>::size_type i = 0; i < result0.size(); i++)
    {
        RsNxsMsg* l = result0[i] ;

        if(VergrpId0.find(l->msgId) == VergrpId0.end())
        {
            msgGrpId0_Match = false;
            break;
        }

        RsNxsMsg* r = VergrpId0[l->msgId];
        r->transactionNumber = l->transactionNumber;

        if(!(*l == *r))
        {
            msgGrpId0_Match = false;
            break;
        }
    }

    CHECK(msgGrpId0_Match);

    // META test
    for(std::vector<RsGxsMsgMetaData*>::size_type i = 0; i < resultMeta0.size(); i++)
    {
        RsGxsMsgMetaData* l = resultMeta0[i] ;

        if(VerMetagrpId0.find(l->mMsgId) == VerMetagrpId0.end())
        {
            msgMetaGrpId0_Match = false;
            break;
        }

        RsGxsMsgMetaData* r = VerMetagrpId0[l->mMsgId];

        if(!(*l == *r))
        {
            msgMetaGrpId0_Match = false;
            break;
        }
    }

    CHECK(msgMetaGrpId0_Match);

    // MSG test, bulk retrieval
    for(std::vector<RsNxsMsg*>::size_type i = 0; i < result1.size(); i++)
    {
        RsNxsMsg* l = result1[i] ;

        if(VergrpId1.find(l->msgId) == VergrpId1.end())
        {
            msgGrpId1_Match = false;
            break;
        }

        RsNxsMsg* r = VergrpId1[l->msgId];

        r->transactionNumber = l->transactionNumber;

        if(!(*l == *r))
        {
            msgGrpId1_Match = false;
            break;
        }
    }

    CHECK(msgGrpId1_Match);

    //dStore->retrieveGxsMsgMetaData();
    std::string msgFile = grpId0 + "-msgs";
    remove(msgFile.c_str());
    msgFile = grpId1 + "-msgs";
    remove(msgFile.c_str());
    tearDown();
}



void setUp(){
    dStore = new RsDataService(".", DATA_BASE_NAME, RS_SERVICE_TYPE_PLUGIN_SIMPLE_FORUM);
}

void tearDown(){

    dStore->resetDataStore(); // reset to clean up store files except db
    delete dStore;
    dStore = NULL;
    int rc = remove(DATA_BASE_NAME);

    if(rc == 0){
        std::cerr << "Successful tear down" << std::endl;
    }
    else{
        std::cerr << "Tear down failed" << std::endl;
        perror("Error: ");
    }

}



bool operator ==(const RsGxsGrpMetaData& l, const RsGxsGrpMetaData& r)
{
    if(!(l.adminSign == r.adminSign)) return false;
    if(!(l.idSign == r.idSign)) return false;
    if(!(l.keys == r.keys)) return false;
    if(l.mGroupFlags != r.mGroupFlags) return false;
    if(l.mPublishTs != r.mPublishTs) return false;
    if(l.mAuthorId != r.mAuthorId) return false;
    if(l.mGroupName != r.mGroupName) return false;
    if(l.mGroupId != r.mGroupId) return false;
    if(l.mGroupStatus != r.mGroupStatus) return false;
    if(l.mPop != r.mPop) return false;
    if(l.mMsgCount != r.mMsgCount) return false;
    if(l.mSubscribeFlags != r.mSubscribeFlags) return false;

    return true;
}

bool operator ==(const RsGxsMsgMetaData& l, const RsGxsMsgMetaData& r)
{

    if(!(l.idSign == r.idSign)) return false;
    if(!(l.pubSign == r.pubSign)) return false;
    if(l.mGroupId != r.mGroupId) return false;
    if(l.mAuthorId != r.mAuthorId) return false;
    if(l.mParentId != r.mParentId) return false;
    if(l.mOrigMsgId != r.mOrigMsgId) return false;
    if(l.mThreadId != r.mThreadId) return false;
    if(l.mMsgId != r.mMsgId) return false;
    if(l.mMsgName != r.mMsgName) return false;
    if(l.mPublishTs != r.mPublishTs) return false;
    if(l.mMsgFlags != r.mMsgFlags) return false;

    return true;
}
