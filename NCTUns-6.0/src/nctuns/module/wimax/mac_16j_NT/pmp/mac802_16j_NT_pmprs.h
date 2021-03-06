/*
 * Copyright (c) from 2000 to 2009
 *
 * Network and System Laboratory
 * Department of Computer Science
 * College of Computer Science
 * National Chiao Tung University, Taiwan
 * All Rights Reserved.
 *
 * This source code file is part of the NCTUns 6.0 network simulator.
 *
 * Permission to use, copy, modify, and distribute this software and
 * its documentation is hereby granted (excluding for commercial or
 * for-profit use), provided that both the copyright notice and this
 * permission notice appear in all copies of the software, derivative
 * works, or modified versions, and any portions thereof, and that
 * both notices appear in supporting documentation, and that credit
 * is given to National Chiao Tung University, Taiwan in all publications
 * reporting on direct or indirect use of this code or its derivatives.
 *
 * National Chiao Tung University, Taiwan makes no representations
 * about the suitability of this software for any purpose. It is provided
 * "AS IS" without express or implied warranty.
 *
 * A Web site containing the latest NCTUns 6.0 network simulator software
 * and its documentations is set up at http://NSL.csie.nctu.edu.tw/nctuns.html.
 *
 * Project Chief-Technology-Officer
 *
 * Prof. Shie-Yuan Wang <shieyuan@csie.nctu.edu.tw>
 * National Chiao Tung University, Taiwan
 *
 * 09/01/2009
 */

#ifndef __NCTUNS_80216J_NT_MAC_80216J_NT_PMPRS_H__
#define __NCTUNS_80216J_NT_MAC_80216J_NT_PMPRS_H__

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <math.h>
#include <vector>

#include <ethernet.h>
#include <nctuns_api.h>
#include <packet.h>
#include <object.h>
#include <tcp.h>
#include <timer.h>
#include <hash_map>

#include "scheduler.h"
#include "msobject_NT.h"
#include "rsobject_NT.h"
#include "rs_base_NT.h"
#include "ms_base_NT.h"
#include "../common.h"
#include "../library.h"
#include "../mac_address.h"
#include "../timer_mngr.h"
#include "../ofdma_map_ie.h"
#include "../../phy_16j_NT/ofdma_80216j_NT.h"
#include "../../phy_16j_NT/ofdma_pmprs_NT.h"

#define    NTRS_Init       0x000
#define    NTRS_Scan       0x001    // Scanning & Synchronization to the DL
#define    NTRS_DLParam    0x002    // Obtain DL parameters
#define    NTRS_ULParam    0x004    // Obtain UL parameters
#define    NTRS_Ranging    0x008    // Initial ranging & automatic adjustments
#define    NTRS_Negcap     0x010    // Negotiate basic capabilities
#define    NTRS_Auth       0x020
#define    NTRS_Register   0x040    // Registration
#define    NTRS_Prov       0x080    // Establish provisioned connections
#define    NTRS_Op         0x100    // Operational

#define    MAXMS        255

namespace std
{
	using namespace __gnu_cxx;
}
extern const double frameDuration_NT[];

using namespace std;
using namespace MR_Common_NT;
using namespace MR_MacAddress_NT;
using namespace MR_Timer_NT;
using namespace MR_RSbase_NT;
using namespace MR_MSbase_NT;
using namespace MR_OFDMAMapIE_NT;

class RS_16j_NT_Scheduler;
class msObject_NT;
class rsObject_NT;


class mac802_16j_NT_PMPRS:public mac802_16RSbase {
	private:
        enum RangingStatus {
	        Init,
	        Continue,
	        Abort,
	        RngSuccess,
	        CodeSuccess,
    	    Rerange,
	        REQWaitRSP,
	        CodeWaitRSP,
        };
		char    *LinkMode;
        char    *CSTYPE;
		uint8_t myDIUC;

		int     State;
        int     frameNumber;
		int     *_frameDuCode;
		uint8_t ULsymbols;

        int     _ULAccessSymbols;
        int     _ULRelaySymbols;
        int     _DLAccessSymbols;
        int     _DLRelaySymbols;
        bool    relay_ind;     // indicate if relay zone existed
        int     RelayULallocStartTime;

        hash_map<uint32_t,int> routingTable;
        typedef pair <uint32_t,int> ipConnPair;
        list<msObject_NT *>     PolledList;

        //msObject_NT             *mslist[MAXMS];

		timerObj *timerSendRngCode;
		timerObj *timerSendRngReq;
		timerObj *timerSendUL;
		timerObj *timerSendDL;
		timerObj *timerLostDL;
		timerObj *timerLostUL;
		timerObj *timerHORetransmit;
		timerObj *timerPeriodRanging;
		timerObj *timerSendScanReq;
		timerObj *timerStartScan;
		timerObj *timerStartHO;

		int LostULinterval;
		int LostDLinterval;
		int LostRMAPinterval;
		int PeriodRangingInterval;
		int HORetransmitInterval;
		int ScanReqInterval;
		int ServerLevelPredict;
		int HOReqRetryAvailable;
		int TimingAdjust;
		double fecSNR[16];    // Index by rate id (fec)

		uint8_t RngStart;
		uint8_t RngEnd;
		uint8_t ReqStart;
		uint8_t ReqEnd;
		uint8_t RngCodeRegion;
		uint8_t UCDCfgCount;
		uint8_t DCDCfgCount;
		//uint8_t RCDCfgCount;

		uint16_t RngCount;
		uint16_t RngTime;
		uint16_t RngLastStatus;
		uint32_t RngChOffset;
		uint32_t RngSymOffset;

		bool        ScanFlag;
		uint8_t     ServingBSID[6];
		uint8_t     saved_msg[256];
		uint16_t    lastCID;
		ifmgmt      *ifsavedmm;

        list <uint16_t> mem_cidlist;

		RangingObject savedCode;
		ULMAP_IE_u savedULie;
		ULMAP_IE_u savedIHie;
		ULMAP_IE_u savedBPie;
		ULMAP_IE_u savedALie;

        // store the relay broadcast messages sends from BS
        ManagementConnection             *broadcastConnection;
		ManagementConnection             *initRangingConnection;

        vector<ManagementConnection *>   MnConnections;    // Basic, Primary Connections
		vector<DataConnection *>         DtConnections;
		vector<ConnectionReceiver *>     ReceiverList;

        /*record management & data connection relay by this RS*/

		struct DCDBurstProfile DCDProfile[16];
		struct UCDBurstProfile UCDProfile[16];
		struct PHYInfo LastSignalInfo;

		NeighborMRBSs_NT *NeighborMRBSList;
		RS_16j_NT_Scheduler *pScheduler;

		///////////////////////////////////////////////////////////////// member functions

		//void PushPDUintoDLRelayManageConnection(uint16_t cid, ifmgmt * pdu);
        Connection *getConnection(uint16_t cid);
		uint64_t computeSendTime(int numSlots, int duration, int fastime);
		uint64_t computeSendTime(const struct OFDMA_ULMAP_IE_12 &ie_12, int txopOffset, int fastime);
        uint64_t frameStartTime;

		void T1();
		void T2();
		void T3();
		void T6();
		void T8();
		void T12();
		void T18();
		void T42();
		void T62();
        void T67();
        void LostDLMAP();
		void LostULMAP();
		void HOmsgRetransmit();
		void StartPeriodRanging();
		void PacketScheduling();
		void DLPacketScheduling();
		void ULPacketScheduling();
		void StartScanNbrMRBS_NTs();
		void StartHandover();
		void WaitForRangingOpportunity();

		int procDCD(struct mgmt_msg *, int, int);
		int procUCD(struct mgmt_msg *, int, int);
		int procRCD(struct mgmt_msg *, int, int);
		int procDLMAP(struct mgmt_msg *, int, int);
		int procULMAP(struct mgmt_msg *, int, int);

        int procMS_InfoDel(struct mgmt_msg *, int, int);
        int procMOB_NBRADV(struct mgmt_msg *, int, int);
		int procRNGRSP(struct mgmt_msg *, int, int);
		int procSBCRSP(struct mgmt_msg *, int, int);
		int procMOB_SCNRSP(struct mgmt_msg *, int, int);
		int procREGRSP(struct mgmt_msg *, int, int);
		int procDSAREQ(struct mgmt_msg *, int, int);
		int procDSAACK(struct mgmt_msg *, int, int);
		int procMOB_BSHORSP(struct mgmt_msg *, int, int);

        // 16j add
        int procREPREQ(struct mgmt_msg *, int, int);
		int procRMAP(struct mgmt_msg *, int, int);
		void procRS_AccessMAP(struct mgmt_msg *, int, int);
	    int procRS_ConfigCMD(struct mgmt_msg *, int, int);
	    void procUCD_relay(struct mgmt_msg *, int, int);
	    void procDCD_relay(struct mgmt_msg *, int, int);
        void procRNGRSP_relay(struct mgmt_msg *, int, int);
        int procRS_MemberListUpdate(struct mgmt_msg *, int, int);
        void procRNGREQ(struct mgmt_msg *, int, int);
		int procDSAREQ_relay(struct mgmt_msg *, int, int);

		int SendRNGCODE();
		int SendRNGREQ();
		int SendSBCREQ();
		int SendREGREQ();
		int SendMOB_SCNREQ();
		int SendMOB_SCNREP();
		int SendMOB_MSHOREQ();
		int SendMOB_HOIND(uint8_t);
        int SendMR_GenericACK();
        void SendRS_BR_header();
        void SendMR_CodeREP_header(uint8_t,uint8_t,uint8_t,uint16_t);



        RS_BR_header *generate_RS_BR_header(int,int);

        // 16j add
		uint8_t selectDIUC(double);
		uint8_t selectUIUC(double);
        double  getMCSweight(uint8_t);
        void    saveDLAccessSymbols(int);
        void    saveULAccessSymbols(int);
        void    saveULsymbols(int);
        void    saveDLMAP(char *, int);
        uint8_t alloc_count;
        vector <ULMAP_IE_u> relay_AL;
        vector <ULMAP_IE_u> relay_ULie_others;

        vector < msInfo > mslist;



	public:
		mac802_16j_NT_PMPRS(uint32_t type, uint32_t id, struct plist *pl, const char *name);
		~mac802_16j_NT_PMPRS();

        mac802_16j_NT *access_station;
        bool    NE_success;         // network entry success
        bool    rs_br_hdr_rsp;
        bool    mr_coderep_rsp;

		vector<ManagementConnection *>   ULRelayMnConnections;    // Basic, Primary Connections
		vector<DataConnection *>         ULRelayDtConnections;
		vector<ConnectionReceiver *>     ULRelayReceiverList;

		vector<ManagementConnection *>   DLRelayMnConnections;    // Basic, Primary Connections
		vector<DataConnection *>         DLRelayDtConnections;
		vector<ConnectionReceiver *>     DLRelayReceiverList;

        BR_Connection                    *brConnection;

        list<RangingObject *>   RangingCodeList;

		int init();
		int recv(ePacket_ *);
		int send(ePacket_ *);

		inline int frameDuCode() const { return *_frameDuCode; }
		inline uint8_t getDIUC() { return myDIUC; }

        void PushPDUintoConnection(uint16_t cid, ifmgmt * pdu);
        void handleRangingCode();
        //inline void SetRDIUC(uint8_t diuc) { myRDIUC = diuc; }
        //inline void SetRUIUC(uint8_t uiuc) { myRUIUC = uiuc; }


        void ClearRNGCodeList(list<RangingObject *> *RNGCodeList)
        {
            while(!RNGCodeList->empty())
            {
                RangingObject *rngObg = RNGCodeList->front();
                delete rngObg;
                RNGCodeList->pop_front();
            }
        }

        mac802_16j_NT_PMPMS     *getMS(uint8_t,uint8_t);
		ManagementConnection    *getULRelayMnConn(uint16_t);
		ManagementConnection    *getDLRelayMnConn(uint16_t);
        DataConnection          *getULRelayDtConn(uint16_t);
        DataConnection          *getDLRelayDtConn(uint16_t);
        msInfo                  getMSinfo(uint16_t);

        friend class RS_16j_NT_Scheduler;
};

#endif                /* __NCTUNS_80216J_NT_MAC_80216J_NT_PMPRS_H__ */
