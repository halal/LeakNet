//=========== (C) Copyright 1999 Valve, L.L.C. All rights reserved. ===========
//
// The copyright to the contents herein is the property of Valve, L.L.C.
// The contents may be used and/or copied only with the written permission of
// Valve, L.L.C., or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Purpose: Implementation of CAllPlayersStats
//
// $Workfile:     $
// $Date:         $
//
//------------------------------------------------------------------------------------------------------
// $Log: $
//
// $NoKeywords: $
//=============================================================================
#include "AllPlayersStats.h"
#include "PlayerReport.h"
#include "TextFile.h"

//------------------------------------------------------------------------------------------------------
// Function:	CAllPlayersStats::init
// Purpose:	 intializes the object
//------------------------------------------------------------------------------------------------------
void CAllPlayersStats::init()
{
}

//------------------------------------------------------------------------------------------------------
// Function:	CAllPlayersStats::generate
// Purpose:	generates intermediate data from match info
//------------------------------------------------------------------------------------------------------
void CAllPlayersStats::generate()
{
}

//------------------------------------------------------------------------------------------------------
// Function:	CAllPlayersStats::writeHTML
// Purpose:	writes out html based on the intermediate data generated by generate()
// Input:	html - the html file to output to
//------------------------------------------------------------------------------------------------------
void CAllPlayersStats::writeHTML(CHTMLFile& html)
{
	string filename;
	bool result=g_pApp->os->findfirstfile("*.tfs",filename);

	if (!result)
		return;
	
	multimap<double,CPlrPersist,greater<double> > ranksort;

	html.write("<table cols=1 cellspacing=0 border=0 cellpadding=10 bordercolor=black>\n");
	while(1)
	{
		CTextFile f(filename);
		pair<double,CPlrPersist> insertme;
		insertme.second.read(f);
		insertme.first=insertme.second.rank();
		
		ranksort.insert(insertme);

		if (!g_pApp->os->findnextfile(filename))
			break;
		

	}
	
	g_pApp->os->findfileclose();
	
	multimap<double,CPlrPersist,greater<double> >::iterator rankit=ranksort.begin();
	
	for (rankit;rankit!=ranksort.end();++rankit)
	{
		bool rowstarted=false;
		//double rank=rankit->first;
		CPlrPersist* pcpp=&(rankit->second);

		time_t cutoff=g_pMatchInfo->logOpenTime() - g_pApp->getCutoffSeconds();
		
		if (pcpp->lastplayed >= cutoff || !g_pApp->eliminateOldPlayers)
		{
			if (!rowstarted)
			{
				rowstarted=true;
				html.write("<tr>\n");
			}

			html.write("<td width=300 valign=top>");	
			CPlayerReport pr(pcpp);
			pr.writeHTML(html);
			html.write("</td>\n");
		}
		if (++rankit==ranksort.end())
		{
			if (rowstarted)
				html.write("</tr>\n");
			break;
		}


		//double rank=rankit->first;
		CPlrPersist* pcpp2=&(rankit->second);
		if (pcpp->lastplayed >= cutoff ||  !g_pApp->eliminateOldPlayers)
		{
			if (!rowstarted)
			{
				rowstarted=true;
				html.write("<tr>\n");
			}

			html.write("<td width=300 valign=top>");	
			CPlayerReport pr2(pcpp2);
			pr2.writeHTML(html);
			html.write("</td>\n");
		}
		if (rowstarted)
			html.write("</tr>\n");
	}

	html.write("</table>");

}

