


#include "SegGraph.h"
#include "Utility.h"
#include <string.h>
#include <math.h>

CSegGraph::CSegGraph()
{
	m_segGraph.SetRowFirst();	

}
CSegGraph::~CSegGraph()
{

}

/*********************************************************************
 *
 *  Func Name  : GenerateWordNet
 *
 *  Description: Gernerate the word net from the sLine, that's list all the possible word
 *              
 *
 *  Parameters : sSentence :the sentence 
 *               dictCore: the core dictionary
 *               bOriginalFreq: judge wether the frequency is original or not
 *
 *  Returns    : success or fail
 *********************************************************************/
bool CSegGraph::GenerateWordNet(char *sSentence,CDictionary &dictCore,bool	bOriginalFreq)
{

	unsigned int i=0,j,nLen=strlen(sSentence);
	char sWord[WORD_MAXLENGTH]="",sTempWord[WORD_MAXLENGTH]="",sWordMatch[WORD_MAXLENGTH];
	int nWordIndex=0,nHandleTemp,k,nPOS;
	int nMatchFreq[20],nMatchHandle[20],nTotalFreq,nMatchCount;
	double dValue=0;
	m_nAtomCount=0;
	m_segGraph.SetEmpty();//Set segmentation graph empty

	AtomSegment(sSentence);

    for(i=0;i<m_nAtomCount;i++)//Init the cost array
    {
		if(m_nAtomPOS[i]==CT_CHINESE)//The atom is a Chinese Char
		{
			if(!bOriginalFreq)//Not original frequency
				m_segGraph.SetElement(i,i+1,log((double)MAX_FREQUENCE),0);//init the link with the maximum value
			else
				m_segGraph.SetElement(i,i+1,0,0,m_sAtom[i]);//init the link with the maximum value
		}
		else//Other atom
		{
			strcpy(sWord,m_sAtom[i]);//init the word 
			dValue=MAX_FREQUENCE;
			switch(m_nAtomPOS[i])
			{
			case CT_INDEX:
			case CT_NUM:
				nPOS=-27904;//'m'*256
				strcpy(sWord,"未##数");
				dValue=0;
				break;
			case CT_DELIMITER:
				nPOS=30464;//'w'*256;
				break;
			case CT_LETTER:
				nPOS=-'n'*256-'x';//
				dValue=0;
				strcpy(sWord,"未##串");
				break;
			case CT_SINGLE://12021-2129-3121
				if(GetCharCount("+-1234567890",m_sAtom[i])==(int)strlen(m_sAtom[i]))
				{
					nPOS=-27904;//'m'*256
					strcpy(sWord,"未##数");
				}
				else
				{
					nPOS=-'n'*256-'x';//
					strcpy(sWord,"未##串");
				}
				dValue=0;
				break;
			default:
				nPOS=m_nAtomPOS[i];//'?'*256;
				break;
			}
			if(!bOriginalFreq)//Not original frequency
				m_segGraph.SetElement(i,i+1,0,nPOS);//init the link with minimum
			else
				m_segGraph.SetElement(i,i+1,dValue,nPOS,sWord);//init the link with minimum
		}
    }
	i=0;
	while(i<m_nAtomCount)//All the word
	{
	  strcpy(sWord,m_sAtom[i]);//Get the current atom
	  j=i+1;
	  if(strcmp(sWord,"月")==0&&strcmp(m_sAtom[i+1],"份")==0)//Don't split 月份
		  j+=1;
	  while(j<=m_nAtomCount&&dictCore.GetMaxMatch(sWord,sWordMatch,&nHandleTemp))
	  {//Add a condition to control the end of string
	   //retrieve the dictionary with the word
          if(strcmp(sWordMatch,sWord)==0)//find the current word
		  {
			  nTotalFreq=0;
			  dictCore.GetHandle(sWord,&nMatchCount,nMatchHandle,nMatchFreq);
			  for(k=0;k<nMatchCount;k++)//Add the frequency
			  {
				 nTotalFreq+=nMatchFreq[k];
			  }
			  //Adding a rule to exclude some words to be formed.
			  if(strlen(sWord)==4&&i>=1&&(IsAllNum((unsigned char *)m_sAtom[i-1])||IsAllChineseNum(m_sAtom[i-1]))&&(strncmp(sWord,"年",2)==0||strncmp(sWord,"月",2)==0))
			  {//1年内、1999年末
			     if(CC_Find("末内中底前间初",sWord+2))
				     break;
			  }
			  if(nMatchCount==1)//The possible word has only one POS, store it
			  {
				if(!bOriginalFreq)//Not original frequency
					m_segGraph.SetElement(i,j,-log((double)(nTotalFreq+1))+log((double)MAX_FREQUENCE),nMatchHandle[0]);
				else
					m_segGraph.SetElement(i,j,nTotalFreq,nMatchHandle[0],sWord);
			  }
			  else 
			  {
					if(!bOriginalFreq)//Not original frequency
						m_segGraph.SetElement(i,j,-log((double)(nTotalFreq+1))+log((double)MAX_FREQUENCE),0);
					else
						m_segGraph.SetElement(i,j,nTotalFreq,0,sWord);
			  }
		  }
		  strcat(sWord,m_sAtom[j++]);
	  }
	  i+=1;//Start from i++;
	}
	return true;
}
/*********************************************************************
 *
 *  Func Name  : AtomSegment
 *
 *  Description: Segment the sentence into a atom
 *              
 *
 *  Parameters : sSentence :the sentence which segment into atom
 *
 *  Returns    : success or fail
 *********************************************************************/
bool CSegGraph::AtomSegment(char *sSentence)
{
    unsigned int i=0,j=0,nCurType,nNextType;	
	//i is the pointer of sentence string
	//j is the pointer of pAtoms
	char sChar[3];
	sChar[2]=0;//Set the char ending
	m_sAtom[j][0]=0;//Set the first word as null
	m_nAtomLength[j]=0;
	if(strncmp(sSentence,SENTENCE_BEGIN,strlen(SENTENCE_BEGIN))==0)
	{
		strcpy(m_sAtom[j],SENTENCE_BEGIN);//Set the first word as sentence begining
		m_nAtomLength[j]=strlen(SENTENCE_BEGIN);
		m_nAtomPOS[j]=CT_SENTENCE_BEGIN;//init
		i+=m_nAtomLength[j];
		j+=1;
		m_sAtom[j][0]=0;//Set the first word as null
		m_nAtomLength[j]=0;
	}
	while(i<strlen(sSentence))
	{
		if(strncmp(sSentence+i,SENTENCE_END,strlen(SENTENCE_END))==0)
		{
			strcpy(m_sAtom[j],SENTENCE_END);//Set the first word as null
			m_nAtomLength[j]=strlen(SENTENCE_END);
			m_nAtomPOS[j]=CT_SENTENCE_END;//init
			i+=m_nAtomLength[j];
			j+=1;
			m_sAtom[j][0]=0;//Set the first word as null
			m_nAtomLength[j]=0;
			continue;
		}
		sChar[0]=*(sSentence+i);//Get the char with first byte
		sChar[1]=0;//
		i+=1;
		if(sChar[0]<0)//Two byte char
		{
			sChar[1]=*(sSentence+i);//Get the char with second byte
			i+=1;//i increased by 1
		}
		strcat(m_sAtom[j],sChar);
		nCurType=charType((unsigned char *)sChar);
		if(sChar[0]=='.'&&(charType((unsigned char *)sSentence+i)==CT_NUM||(*(sSentence+i)>='0'&&*(sSentence+i)<='9')))
			nCurType=CT_NUM;//Digit after . indicate . as a point in the numeric
		m_nAtomPOS[j]=nCurType;
		//Record its property, just convience for continuous processing
		
		if(nCurType==CT_CHINESE||nCurType==CT_INDEX||nCurType==CT_DELIMITER||nCurType==CT_OTHER)
		{//Chinese char, index number,delimiter and other is treated as atom
			m_nAtomLength[j]=strlen(m_sAtom[j]);//Save its length
			j+=1;//Skip to next atom
			m_sAtom[j][0]=0;//init
		}
		else 
		{//Number,single char, letter
			nNextType=255;
			if(i<strlen(sSentence))
				nNextType=charType((unsigned char *)(sSentence+i));
			if(nNextType!=nCurType||i==strlen(sSentence))
			//Reaching end or next char type is different from current char
			{
				m_nAtomLength[j]=strlen(m_sAtom[j]);//Save its length	
				j+=1;
				m_sAtom[j][0]=0;//init
			}
		}
	}
	m_nAtomCount=j;//The count of segmentation atoms
	return true;
}
