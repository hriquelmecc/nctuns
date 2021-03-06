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

#ifndef __NCTUNS_80216E_OFDMA_MAP_IE_H__
#define __NCTUNS_80216E_OFDMA_MAP_IE_H__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

/* Except for DIUC 14 or 15 */  // (44 + numCid * 16) bits
struct OFDMA_DLMAP_IE_other {
	uint8_t diuc;
	uint8_t numCid;
	uint16_t *cid;
	uint8_t symOff;
	uint8_t chOff;
	uint8_t boosting;
	uint8_t numSym;
	uint8_t numCh;
	uint8_t repeCode;
};

/* Extended-2 DIUC IE */    // (16 + length * 8) bits
struct OFDMA_DLMAP_IE_14 {
	uint8_t diuc;
	uint8_t ext2_diuc;
	uint8_t length;
	void *data;
};

/* Extended DIUC IE */      // (12 + length * 8) bits
struct OFDMA_DLMAP_IE_15 {
	uint8_t diuc;
	uint8_t ext_diuc;
	uint8_t length;
	void *data;
};

/* Except for UIUC 0, 11~15 */  // 32 bits
struct OFDMA_ULMAP_IE_other {
	uint16_t cid;
	uint8_t uiuc;
	uint16_t duration; // in slots
	uint8_t repeCode;
};

/* FAST-FEEDBACK Allocation IE */   // 52 bits
struct OFDMA_ULMAP_IE_0 {
	uint16_t cid;
	uint8_t uiuc;
	uint8_t symOff;
	uint8_t chOff;
	uint8_t numSym;
	uint8_t numCh;
	uint8_t rsv;
};

/* Extended-2 UIUC IE */    // (32 + length * 8) bits
struct OFDMA_ULMAP_IE_11 {
	uint16_t cid;
	uint8_t uiuc;
	uint8_t ext2_uiuc;
	uint8_t length;
	void *data;
};

/* CDMA Bandwidth-Request, CDMA Ranging IE */  // 52 bits
struct OFDMA_ULMAP_IE_12 {
	uint16_t cid;
	uint8_t uiuc;
	uint8_t symOff;
	uint8_t chOff;
	uint8_t numSym;
	uint8_t numCh;
	uint8_t rangMethod;
	uint8_t rangIndicator;
};

/* PAPR Reduction and Safety Zone Sllocation IE */  // 52 bits
struct OFDMA_ULMAP_IE_13 {
	uint16_t cid;
	uint8_t uiuc;
	uint8_t symOff;
	uint8_t chOff;
	uint8_t numSym;
	uint8_t numCh;
	uint8_t papr_or_safety;
	uint8_t sounding;
	uint8_t rsv;
};

/* CDMA Allocation IE */    // 60 bits
struct OFDMA_ULMAP_IE_14 {
	uint16_t cid;
	uint8_t uiuc;
	uint8_t duration;
	uint8_t uiuc_trans;
	uint8_t repeCode;
	uint8_t frameIndex;
	uint8_t rangCode;
	uint8_t rangSym;
	uint8_t rangCh;
	uint8_t bwReq;
};

/* Extended UIUC IE */  // (28 + length * 8) bits
struct OFDMA_ULMAP_IE_15 {
	uint16_t cid;
	uint8_t uiuc;
	uint8_t ext_uiuc;
	uint8_t length;
	void *data;
};

/* Union of DLMAP_ie and ULMAP_ie*/
typedef union DLMAP_IE_union{
	struct OFDMA_DLMAP_IE_other ie_other;
	struct OFDMA_DLMAP_IE_14    ie_14;
	struct OFDMA_DLMAP_IE_15    ie_15;
}DLMAP_IE_u;

typedef union ULMAP_IE_union{
	struct OFDMA_ULMAP_IE_other ie_other;
	struct OFDMA_ULMAP_IE_0     ie_0;
	struct OFDMA_ULMAP_IE_11    ie_11;
	struct OFDMA_ULMAP_IE_12    ie_12;
	struct OFDMA_ULMAP_IE_13    ie_13;
	struct OFDMA_ULMAP_IE_14    ie_14;
	struct OFDMA_ULMAP_IE_15    ie_15;
}ULMAP_IE_u;

namespace mobileOFDMAMapIE {
	class OFDMA_DLMAP_IE {
		public:
			uint8_t *_ie_data;
			int     _ie_bits;
			int     _offset;
			uint8_t _diuc;

		public:
			OFDMA_DLMAP_IE();
			OFDMA_DLMAP_IE(int);
			OFDMA_DLMAP_IE(uint8_t *, int);
			~OFDMA_DLMAP_IE();

			void        mallocIE        (int);
			void        appendBitField  (int, uint32_t);
			void        appendBitField  (int, uint8_t *);
			void        extractField    (int, uint8_t *);
			inline      int     getBits ()          { return _ie_bits; }
			inline      uint8_t *getData()          { return _ie_data; }
			inline      uint8_t getDIUC ()          { return _diuc; }
			inline      void    setDIUC (uint8_t d) { _diuc = d; }
	};

	class OFDMA_ULMAP_IE {
		public:
			uint8_t     *_ie_data;
			int         _ie_bits;
			int         _offset;
			uint16_t    _cid;
			uint8_t     _uiuc;

		public:
			OFDMA_ULMAP_IE();
			OFDMA_ULMAP_IE(uint16_t, int);
			OFDMA_ULMAP_IE(uint8_t *, int);
			~OFDMA_ULMAP_IE();

			void        mallocIE        (int);
			void        appendBitField  (int, uint32_t);
			void        appendBitField  (int, uint8_t *);
			void        extractField    (int, uint8_t *);
			inline      int     getBits     ()              { return _ie_bits; }
			inline      uint8_t *getData    ()              { return _ie_data; }
			inline      uint8_t getUIUC     ()              { return _uiuc; }
			inline      void    setUIUC     (uint8_t u)     { _uiuc = u; }
			inline      uint8_t getCID      ()              { return _cid; }
			inline      void    setCID      (uint16_t c)    { _cid = c; }
			inline      void    resetOffset ()              { _offset = 0; }
	};
}

#endif          /*  __NCTUNS_80216E_OFDMA_MAP_IE_H__  */ 
