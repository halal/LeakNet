//=========== (C) Copyright 1999 Valve, L.L.C. All rights reserved. ===========
//
// The copyright to the contents herein is the property of Valve, L.L.C.
// The contents may be used and/or copied only with the written permission of
// Valve, L.L.C., or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Purpose: 
//
// $Workfile:     $
// $Date:         $
//
//-----------------------------------------------------------------------------
// $Log: $
//
// $NoKeywords: $
//=============================================================================

#ifndef LIGHTMAP_H
#define LIGHTMAP_H
#pragma once

#include "bumpvects.h"
#include "bsplib.h"

typedef struct
{
	dface_t		*faces[2];
	Vector		interface_normal;
	qboolean	coplanar;
} edgeshare_t;

extern edgeshare_t	edgeshare[MAX_MAP_EDGES];


//==============================================

// This is incremented each time BuildFaceLights and FinalLightFace
// are called. It's used for a status bar in WorldCraft.
extern int g_iCurFace;

extern int vertexref[MAX_MAP_VERTS];
extern int *vertexface[MAX_MAP_VERTS];

struct faceneighbor_t
{
	int		numneighbors;			// neighboring faces that share vertices
	int		*neighbor;				// neighboring face list (max of 64)

	Vector	*normal;				// adjusted normal per vertex
	Vector	facenormal;				// face normal

	bool	bHasDisp;				// is this surface a displacement surface???
};

extern faceneighbor_t faceneighbor[MAX_MAP_FACES];

//==============================================


struct sample_t
{
	// in local luxel space
	winding_t	*w;
	int			s, t;
	Vector2D	coord;	
	Vector2D	mins;
	Vector2D	maxs;
	// in world units
	Vector		pos;
	Vector		normal;
	float		area;	
};

struct facelight_t
{
	// irregularly shaped light sample data, clipped by face and luxel grid
	int			numsamples;
	sample_t	*sample;			
	Vector		*light[MAXLIGHTMAPS][NUM_BUMP_VECTS+1];	// result of direct illumination, indexed by sample

	// regularly spaced lightmap grid
	int			numluxels;			
	Vector		*luxel;				// world space position of luxel
	Vector		*luxelNormals;		// world space normal of luxel
	float		worldAreaPerLuxel;
};

extern directlight_t	*activelights;
extern directlight_t	*freelights;

extern facelight_t		facelight[MAX_MAP_FACES];
extern int				numdlights;


//==============================================

struct lightinfo_t
{
	vec_t	facedist;
	Vector	facenormal;

	Vector	facemid;		// world coordinates of center

	Vector	modelorg;		// for origined bmodels

	Vector	textureOrigin;
	Vector	worldToTextureSpace[2];	// s = (world - texorg) . worldtotex[0]
	Vector	textureToWorldSpace[2];	// world = texorg + s * textoworld[0]

	Vector	luxelOrigin;
	Vector	worldToLuxelSpace[2]; // s = (world - texorg) . worldtotex[0]
	Vector	luxelToWorldSpace[2]; // world = texorg + s * textoworld[0]
	
	//	vec_t	exactmins[2], exactmaxs[2]; not used anymore.
	
	int		lightmapTextureMinsInLuxels[2];
	int		lightmapTextureSizeInLuxels[2];

	int		facenum;
	dface_t	*face;

	int		isflat;
	int		hasbumpmap;
};

struct SampleInfo_t
{
	int		m_FaceNum;
	int		m_WarnFace;
	dface_t	*m_pFace;
	facelight_t	*m_pFaceLight;
	int		m_LightmapWidth;
	int		m_LightmapHeight;
	int		m_LightmapSize;
	int		m_NormalCount;
	int		m_Cluster;
	int		m_iThread;
	texinfo_t	*m_pTexInfo;
	bool	m_IsDispFace;
	Vector	m_Point;
	Vector	m_PointNormal[ NUM_BUMP_VECTS + 1 ];
};


extern void InitLightinfo( lightinfo_t *l, int facenum );

void FreeDLights();


#endif // LIGHTMAP_H
