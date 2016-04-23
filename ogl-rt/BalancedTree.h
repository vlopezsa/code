#ifndef BALANCEDTREE_H
#define BALANCEDTREE_H

#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(object) do{if (object){delete[] object;object=NULL;}}while(false)
#endif

#define VERTEXSTRIDE 4
#define INDEXSTRIDE 3

#ifndef max
#define max(a,b) (a<b?b:a)

#endif

#ifndef min
#define min(a,b) (a<b?a:b)

#endif

class TBalancedTree
{
public:
	struct TVertex
	{
		float x,y,z;
	};

	struct TBoundingBox
	{
		TVertex minCorner,maxCorner;
	};

	struct TNode
	{
		int PrimPos,PrimCountCut;
		TBoundingBox BoundingBox;
	};

	struct TTriangleInfo
	{
		TVertex Center;
		float len;
	};

private:
	TNode *Nodes;
	TTriangleInfo *PrimitiveCenter;
	const float *_vertexinfo;
	int _numprim,numnodes;
	const int *index;
	int *_sortedprims;
	int *_sortedprimsaux;

	void sortprimsbycenterx(int ini,int end,float min,float max)
	{
		if (max<=min)
			return;
		float cut=0.5f*(max+min);
		float maxleft=min;
		float minright=max;
		int a=ini;
		int b=end;
		while (a<b)
		{
			while ((a<b)&&(PrimitiveCenter[_sortedprims[a]].Center.x<=cut)){
				if (maxleft<PrimitiveCenter[_sortedprims[a]].Center.x)
				{
					maxleft=PrimitiveCenter[_sortedprims[a]].Center.x;
				}
				a++;
			}
			while ((a<b)&&(PrimitiveCenter[_sortedprims[b]].Center.x>cut))
			{
				if (minright>PrimitiveCenter[_sortedprims[b]].Center.x)
				{
					minright=PrimitiveCenter[_sortedprims[b]].Center.x;
				}
				b--;
			}
			if (a<b)
			{
				int aux=_sortedprims[a];
				_sortedprims[a]=_sortedprims[b];
				_sortedprims[b]=aux;
				if (maxleft<PrimitiveCenter[_sortedprims[a]].Center.x)
				{
					maxleft=PrimitiveCenter[_sortedprims[a]].Center.x;
				}
				if (minright>PrimitiveCenter[_sortedprims[b]].Center.x)
				{
					minright=PrimitiveCenter[_sortedprims[b]].Center.x;
				}
				a++;
			}
		}
		sortprimsbycenterx(ini,a-1,min,maxleft);
		sortprimsbycenterx(a,end,minright,max);
	}

	void sortprimsbycentery(int ini,int end,float min,float max)
	{
		if (max<=min)
			return;
		float cut=0.5f*(max+min);
		float maxleft=min;
		float minright=max;
		int a=ini;
		int b=end;
		while (a<b)
		{
			while ((a<b)&&(PrimitiveCenter[_sortedprims[a]].Center.y<=cut)){
				if (maxleft<PrimitiveCenter[_sortedprims[a]].Center.y)
				{
					maxleft=PrimitiveCenter[_sortedprims[a]].Center.y;
				}
				a++;
			}
			while ((a<b)&&(PrimitiveCenter[_sortedprims[b]].Center.y>cut))
			{
				if (minright>PrimitiveCenter[_sortedprims[b]].Center.y)
				{
					minright=PrimitiveCenter[_sortedprims[b]].Center.y;
				}
				b--;
			}
			if (a<b)
			{
				int aux=_sortedprims[a];
				_sortedprims[a]=_sortedprims[b];
				_sortedprims[b]=aux;
				if (maxleft<PrimitiveCenter[_sortedprims[a]].Center.y)
				{
					maxleft=PrimitiveCenter[_sortedprims[a]].Center.y;
				}
				if (minright>PrimitiveCenter[_sortedprims[b]].Center.y)
				{
					minright=PrimitiveCenter[_sortedprims[b]].Center.y;
				}
				a++;
			}
		}
		sortprimsbycentery(ini,a-1,min,maxleft);
		sortprimsbycentery(a,end,minright,max);
	}

	void sortprimsbycenterz(int ini,int end,float min,float max)
	{
		if (max<=min)
			return;
		float cut=0.5f*(max+min);
		float maxleft=min;
		float minright=max;
		int a=ini;
		int b=end;
		while (a<b)
		{
			while ((a<b)&&(PrimitiveCenter[_sortedprims[a]].Center.z<=cut)){
				if (maxleft<PrimitiveCenter[_sortedprims[a]].Center.z)
				{
					maxleft=PrimitiveCenter[_sortedprims[a]].Center.z;
				}
				a++;
			}
			while ((a<b)&&(PrimitiveCenter[_sortedprims[b]].Center.z>cut))
			{
				if (minright>PrimitiveCenter[_sortedprims[b]].Center.z)
				{
					minright=PrimitiveCenter[_sortedprims[b]].Center.z;
				}
				b--;
			}
			if (a<b)
			{
				int aux=_sortedprims[a];
				_sortedprims[a]=_sortedprims[b];
				_sortedprims[b]=aux;
				if (maxleft<PrimitiveCenter[_sortedprims[a]].Center.z)
				{
					maxleft=PrimitiveCenter[_sortedprims[a]].Center.z;
				}
				if (minright>PrimitiveCenter[_sortedprims[b]].Center.z)
				{
					minright=PrimitiveCenter[_sortedprims[b]].Center.z;
				}
				a++;
			}
		}
		sortprimsbycenterz(ini,a-1,min,maxleft);
		sortprimsbycenterz(a,end,minright,max);
	}

	int finMSBposition(const int x)
	{
		unsigned long index;
		_BitScanReverse(&index,x);
		return index;
		//__asm  bsr  eax, x
	}

	int finLSBposition(const int x)
	{
		unsigned long index;
		_BitScanForward(&index,x);
		return index;
		//__asm  bsf  eax, x
	}

	void BuildTreeInternal(int PrimPos,int PrimCount,int nodeID)
	{
		if (((nodeID<<1)>=numnodes)||(PrimCount==1))
		{
			//it is a leaf, no more nodes to cut
			int PrimID = _sortedprims[PrimPos];
			Nodes[nodeID].BoundingBox.maxCorner.x = max(_vertexinfo[index[PrimID*INDEXSTRIDE+0]*VERTEXSTRIDE+0],max(_vertexinfo[index[PrimID*INDEXSTRIDE+1]*VERTEXSTRIDE+0],_vertexinfo[index[PrimID*INDEXSTRIDE+2]*VERTEXSTRIDE+0]));
			Nodes[nodeID].BoundingBox.maxCorner.y = max(_vertexinfo[index[PrimID*INDEXSTRIDE+0]*VERTEXSTRIDE+1],max(_vertexinfo[index[PrimID*INDEXSTRIDE+1]*VERTEXSTRIDE+1],_vertexinfo[index[PrimID*INDEXSTRIDE+2]*VERTEXSTRIDE+1]));
			Nodes[nodeID].BoundingBox.maxCorner.z = max(_vertexinfo[index[PrimID*INDEXSTRIDE+0]*VERTEXSTRIDE+2],max(_vertexinfo[index[PrimID*INDEXSTRIDE+1]*VERTEXSTRIDE+2],_vertexinfo[index[PrimID*INDEXSTRIDE+2]*VERTEXSTRIDE+2]));
			Nodes[nodeID].BoundingBox.minCorner.x = min(_vertexinfo[index[PrimID*INDEXSTRIDE+0]*VERTEXSTRIDE+0],min(_vertexinfo[index[PrimID*INDEXSTRIDE+1]*VERTEXSTRIDE+0],_vertexinfo[index[PrimID*INDEXSTRIDE+2]*VERTEXSTRIDE+0]));
			Nodes[nodeID].BoundingBox.minCorner.y = min(_vertexinfo[index[PrimID*INDEXSTRIDE+0]*VERTEXSTRIDE+1],min(_vertexinfo[index[PrimID*INDEXSTRIDE+1]*VERTEXSTRIDE+1],_vertexinfo[index[PrimID*INDEXSTRIDE+2]*VERTEXSTRIDE+1]));
			Nodes[nodeID].BoundingBox.minCorner.z = min(_vertexinfo[index[PrimID*INDEXSTRIDE+0]*VERTEXSTRIDE+2],min(_vertexinfo[index[PrimID*INDEXSTRIDE+1]*VERTEXSTRIDE+2],_vertexinfo[index[PrimID*INDEXSTRIDE+2]*VERTEXSTRIDE+2]));
			Nodes[nodeID].PrimCountCut = (PrimCount<<2)|3;//leaf
			Nodes[nodeID].PrimPos = PrimPos;
			for (int i=1;i<PrimCount;i++)
			{
				PrimID = _sortedprims[PrimPos+i];
				for (int j=0;j<3;j++)
				{
					Nodes[nodeID].BoundingBox.maxCorner.x = max(_vertexinfo[index[PrimID*INDEXSTRIDE+j]*VERTEXSTRIDE+0],Nodes[nodeID].BoundingBox.maxCorner.x);
					Nodes[nodeID].BoundingBox.maxCorner.y = max(_vertexinfo[index[PrimID*INDEXSTRIDE+j]*VERTEXSTRIDE+1],Nodes[nodeID].BoundingBox.maxCorner.y);
					Nodes[nodeID].BoundingBox.maxCorner.z = max(_vertexinfo[index[PrimID*INDEXSTRIDE+j]*VERTEXSTRIDE+2],Nodes[nodeID].BoundingBox.maxCorner.z);
					Nodes[nodeID].BoundingBox.minCorner.x = min(_vertexinfo[index[PrimID*INDEXSTRIDE+j]*VERTEXSTRIDE+0],Nodes[nodeID].BoundingBox.minCorner.x);
					Nodes[nodeID].BoundingBox.minCorner.y = min(_vertexinfo[index[PrimID*INDEXSTRIDE+j]*VERTEXSTRIDE+1],Nodes[nodeID].BoundingBox.minCorner.y);
					Nodes[nodeID].BoundingBox.minCorner.z = min(_vertexinfo[index[PrimID*INDEXSTRIDE+j]*VERTEXSTRIDE+2],Nodes[nodeID].BoundingBox.minCorner.z);
				}
			}
			return;
		}
		// Find bounding box by center
		int PrimID = _sortedprims[PrimPos];
		float rangemax[3],rangemin[3];
		rangemax[0]=rangemin[0]=PrimitiveCenter[PrimID].Center.x;
		rangemax[1]=rangemin[1]=PrimitiveCenter[PrimID].Center.y;
		rangemax[2]=rangemin[2]=PrimitiveCenter[PrimID].Center.z;
//		rangemax[3]=rangemin[3]=PrimitiveCenter[PrimID].len;
		Nodes[nodeID].BoundingBox.minCorner = PrimitiveCenter[PrimID].Center;
		for (int i=1;i<PrimCount;i++)
		{
			PrimID = _sortedprims[PrimPos+i];
			rangemax[0]=max(rangemax[0],PrimitiveCenter[PrimID].Center.x);
			rangemax[1]=max(rangemax[1],PrimitiveCenter[PrimID].Center.y);
			rangemax[2]=max(rangemax[2],PrimitiveCenter[PrimID].Center.z);
//			rangemax[3]=max(rangemax[3],PrimitiveCenter[PrimID].len);
			rangemin[0]=min(rangemin[0],PrimitiveCenter[PrimID].Center.x);
			rangemin[1]=min(rangemin[1],PrimitiveCenter[PrimID].Center.y);
			rangemin[2]=min(rangemin[2],PrimitiveCenter[PrimID].Center.z);
//			rangemin[3]=min(rangemin[3],PrimitiveCenter[PrimID].len);
		}
		// Take longest axis for sorting
		int maxaxis = 0;
		for (int i=1;i<3;i++)
		{
			if ((rangemax[i]-rangemin[i])>(rangemax[maxaxis]-rangemin[maxaxis]))
			{
				maxaxis=i;
			}
		}
		//find how many primitives should be removed in this node
		int bigPrim = 0;
		/*
		for (int i = 0;i<PrimCount;i++)
		{
			PrimID = _sortedprims[PrimPos+i];
			if (PrimitiveCenter[PrimID].len>=PrimitiveCenter[PrimPos].len)
			{
				//swap and add counter
				bigPrim=i;
			}
		}		
		int aux=_sortedprims[PrimPos+bigPrim];
		_sortedprims[PrimPos+bigPrim]=_sortedprims[PrimPos];
		_sortedprims[PrimPos]=aux;
		PrimID = _sortedprims[PrimPos];
		if (PrimitiveCenter[PrimID].len>=(0.5*(rangemax[maxaxis]-rangemin[maxaxis])))
			bigPrim=1;
		else
			bigPrim=0;
		if ((PrimCount-bigPrim)<2)
		{
			//almost all are big, build tree as normal
			bigPrim=0;
		}
		//update maxaxis in case it is needed
		if (bigPrim>0)
		{
			PrimCount-=bigPrim;
			PrimPos+=bigPrim;
			PrimID = _sortedprims[PrimPos];		
			rangemax[0]=rangemin[0]=PrimitiveCenter[PrimID].Center.x;
			rangemax[1]=rangemin[1]=PrimitiveCenter[PrimID].Center.y;
			rangemax[2]=rangemin[2]=PrimitiveCenter[PrimID].Center.z;
			Nodes[nodeID].BoundingBox.minCorner = PrimitiveCenter[PrimID].Center;
			for (int i=1;i<PrimCount;i++)
			{
				PrimID = _sortedprims[PrimPos+i];
				rangemax[0]=max(rangemax[0],PrimitiveCenter[PrimID].Center.x);
				rangemax[1]=max(rangemax[1],PrimitiveCenter[PrimID].Center.y);
				rangemax[2]=max(rangemax[2],PrimitiveCenter[PrimID].Center.z);
				rangemin[0]=min(rangemin[0],PrimitiveCenter[PrimID].Center.x);
				rangemin[1]=min(rangemin[1],PrimitiveCenter[PrimID].Center.y);
				rangemin[2]=min(rangemin[2],PrimitiveCenter[PrimID].Center.z);
			}
			for (int i=0;i<3;i++)
			{
				if ((rangemax[i]-rangemin[i])>(rangemax[maxaxis]-rangemin[maxaxis]))
				{
					maxaxis=i;
				}
			}
		}
		*/
		// find how many primitives should be send to the left
		int MSB = finMSBposition(PrimCount);
		int mask = (PrimCount>>(MSB-1))&3;
		//int left = PrimCount/2;
		int left;
		if (mask==2)
		{
			//send one third and rest to left amd one thir to right
			left = PrimCount-(1<<(MSB-1));
		}
		else //mask==3
		{
			//send two thirds to left and 1 third and rest to right
			left = 1<<MSB;
		}
		int left_save = left;
		int currentPos = PrimPos;
		int remaining = PrimCount;
		float maxcurrent = rangemax[maxaxis];
		float mincurrent = rangemin[maxaxis];
		// Move primitives to its corresponding side
		/*
		#define NUMBUCKETS 5
		int bucketcount[NUMBUCKETS+1];
		if (maxcurrent>mincurrent)
		{
			//generate bucket counters
			memset(bucketcount,0,sizeof(bucketcount));
			for (int i=0;i<PrimCount;i++)
			{
				PrimID = _sortedprims[PrimPos+i];
				float value = *((&PrimitiveCenter[PrimID].Center.x)+maxaxis);
				int bucket = float(NUMBUCKETS)*(value-mincurrent)/(maxcurrent-mincurrent);
				if (bucket>=NUMBUCKETS)
				{
					bucket=NUMBUCKETS-1;
				}
				bucketcount[bucket+1]++;
			}
			for (int i=1;i<=NUMBUCKETS;i++)
			{
				bucketcount[i]+=bucketcount[i-1];
			}
			//move primitives to its position
			for (int i=0;i<PrimCount;i++)
			{
				PrimID = _sortedprims[PrimPos+i];
				float value = *((&PrimitiveCenter[PrimID].Center.x)+maxaxis);
				int bucket = float(NUMBUCKETS)*(value-mincurrent)/(maxcurrent-mincurrent);
				if (bucket>=NUMBUCKETS)
				{
					bucket=NUMBUCKETS-1;
				}
				_sortedprimsaux[PrimPos+bucketcount[bucket]]=PrimID;
				bucketcount[bucket]++;
			}
			//refine bucket where the cut is
			//copy back to sorted prims
			for (int i=0;i<PrimCount;i++)
			{
				_sortedprims[PrimPos+i]=_sortedprimsaux[PrimPos+i];
			}
		}
		*/

		while ((left>0)&&(left<remaining))
		{
			if (mincurrent==maxcurrent)
			{
				break;
			}
			float cut = mincurrent+((maxcurrent-mincurrent)*float(left)/float(remaining));
			//generate bucket counters
			/*
			memset(bucketcount,0,sizeof(bucketcount));
			for (int i=0;i<remaining;i++)
			{
				PrimID = _sortedprims[currentPos+i];
				float value = *((&PrimitiveCenter[PrimID].Center.x)+maxaxis);
				int bucket = float(NUMBUCKETS)*(value-mincurrent)/(maxcurrent-mincurrent);
				if (bucket>=NUMBUCKETS)
				{
					bucket=NUMBUCKETS-1;
				}
				bucketcount[bucket]++;
			}
			int bucketcut=0;
			int totalcount = bucketcount[bucketcut];
			while (totalcount<left)
			{
				bucketcut++;
				totalcount+=bucketcount[bucketcut];
			}
			cut=mincurrent+(float(bucketcut)+(float(bucketcount[bucketcut]+left-totalcount)/float(bucketcount[bucketcut])))*(maxcurrent-mincurrent)/float(NUMBUCKETS);
			*/
			//move all the lower or equal to cut to the left
			int a = currentPos;
			int b = currentPos+remaining-1;
			float minright = maxcurrent;
			float maxleft = mincurrent;
			while (a<b)
			{
				while ((*((&PrimitiveCenter[_sortedprims[a]].Center.x)+maxaxis))<=cut)
				{
					if (maxleft<(*((&PrimitiveCenter[_sortedprims[a]].Center.x)+maxaxis)))
					{
						maxleft = (*((&PrimitiveCenter[_sortedprims[a]].Center.x)+maxaxis));
					}
					a++;
				}
				while ((*((&PrimitiveCenter[_sortedprims[b]].Center.x)+maxaxis))>cut)
				{
					if (minright>(*((&PrimitiveCenter[_sortedprims[b]].Center.x)+maxaxis)))
					{
						minright = (*((&PrimitiveCenter[_sortedprims[b]].Center.x)+maxaxis));
					}
					b--;
				}
				if (a<b)
				{
					//swap
					int aux = _sortedprims[a];
					_sortedprims[a]=_sortedprims[b];
					_sortedprims[b]=aux;
				}
			}
			// update variables
			int onleft = (b-currentPos)+1;
			if (onleft<=left)
			{
				//take the rest from the right
				left-=onleft;
				currentPos += onleft;
				remaining-=onleft;
				mincurrent=minright;
			}
			else
			{
				//remove some from the left
				if (remaining == onleft)
					break;//all the same value, finish
				remaining = onleft;
				maxcurrent = maxleft;
			}
		}
		left = left_save;
		BuildTreeInternal(PrimPos,left,nodeID<<1);
		BuildTreeInternal(PrimPos+left,PrimCount-left,(nodeID<<1)+1);
		// Update bounding boxes
		Nodes[nodeID].BoundingBox.maxCorner.x=max(Nodes[nodeID<<1].BoundingBox.maxCorner.x,Nodes[(nodeID<<1)+1].BoundingBox.maxCorner.x);
		Nodes[nodeID].BoundingBox.maxCorner.y=max(Nodes[nodeID<<1].BoundingBox.maxCorner.y,Nodes[(nodeID<<1)+1].BoundingBox.maxCorner.y);
		Nodes[nodeID].BoundingBox.maxCorner.z=max(Nodes[nodeID<<1].BoundingBox.maxCorner.z,Nodes[(nodeID<<1)+1].BoundingBox.maxCorner.z);
		Nodes[nodeID].BoundingBox.minCorner.x=min(Nodes[nodeID<<1].BoundingBox.minCorner.x,Nodes[(nodeID<<1)+1].BoundingBox.minCorner.x);
		Nodes[nodeID].BoundingBox.minCorner.y=min(Nodes[nodeID<<1].BoundingBox.minCorner.y,Nodes[(nodeID<<1)+1].BoundingBox.minCorner.y);
		Nodes[nodeID].BoundingBox.minCorner.z=min(Nodes[nodeID<<1].BoundingBox.minCorner.z,Nodes[(nodeID<<1)+1].BoundingBox.minCorner.z);
		//Nodes[nodeID].Cut = maxaxis;
		Nodes[nodeID].PrimCountCut = (bigPrim<<2)|maxaxis;
		Nodes[nodeID].PrimPos = PrimPos-bigPrim;
		for (int i=Nodes[nodeID].PrimPos;i<PrimPos;i++)
		{
			PrimID = _sortedprims[i];
			for (int j=0;j<3;j++)
			{
				Nodes[nodeID].BoundingBox.maxCorner.x = max(_vertexinfo[index[PrimID*INDEXSTRIDE+j]*VERTEXSTRIDE+0],Nodes[nodeID].BoundingBox.maxCorner.x);
				Nodes[nodeID].BoundingBox.maxCorner.y = max(_vertexinfo[index[PrimID*INDEXSTRIDE+j]*VERTEXSTRIDE+1],Nodes[nodeID].BoundingBox.maxCorner.y);
				Nodes[nodeID].BoundingBox.maxCorner.z = max(_vertexinfo[index[PrimID*INDEXSTRIDE+j]*VERTEXSTRIDE+2],Nodes[nodeID].BoundingBox.maxCorner.z);
				Nodes[nodeID].BoundingBox.minCorner.x = min(_vertexinfo[index[PrimID*INDEXSTRIDE+j]*VERTEXSTRIDE+0],Nodes[nodeID].BoundingBox.minCorner.x);
				Nodes[nodeID].BoundingBox.minCorner.y = min(_vertexinfo[index[PrimID*INDEXSTRIDE+j]*VERTEXSTRIDE+1],Nodes[nodeID].BoundingBox.minCorner.y);
				Nodes[nodeID].BoundingBox.minCorner.z = min(_vertexinfo[index[PrimID*INDEXSTRIDE+j]*VERTEXSTRIDE+2],Nodes[nodeID].BoundingBox.minCorner.z);
			}
		}
	}
	
public:
	TBalancedTree()
	{
		Nodes=NULL;
		PrimitiveCenter=NULL;
	}

	void clean(void)
	{
		SAFE_DELETE_ARRAY(Nodes);
		SAFE_DELETE_ARRAY(PrimitiveCenter);
	}


	void BuildTree(const float *vertexinfo,const int *indices,int numprim,int *sortedprim)
	{
		//reserve memory
		if (numprim!=_numprim)
		{
			clean();
			numnodes = numprim*2;
			Nodes = new TNode[numnodes];
			PrimitiveCenter = new TTriangleInfo[numprim];
		}
		_numprim=numprim;
		_vertexinfo=vertexinfo;
		_sortedprims = sortedprim;
		_sortedprimsaux = new int[numprim];
		//compute bounding info for every primitive
		const float factor = 1.0f/3.0f;
		index=indices;
		float minx,maxx,miny,maxy,minz,maxz;
		for (int i=0;i<numprim;i++)
		{
			sortedprim[i]=i;
			TVertex va,vb,vc;
			va.x = vertexinfo[indices[i*INDEXSTRIDE+0]*VERTEXSTRIDE+0];
			va.y = vertexinfo[indices[i*INDEXSTRIDE+0]*VERTEXSTRIDE+1];
			va.z = vertexinfo[indices[i*INDEXSTRIDE+0]*VERTEXSTRIDE+2];
			vb.x = vertexinfo[indices[i*INDEXSTRIDE+1]*VERTEXSTRIDE+0];
			vb.y = vertexinfo[indices[i*INDEXSTRIDE+1]*VERTEXSTRIDE+1];
			vb.z = vertexinfo[indices[i*INDEXSTRIDE+1]*VERTEXSTRIDE+2];
			vc.x = vertexinfo[indices[i*INDEXSTRIDE+2]*VERTEXSTRIDE+0];
			vc.y = vertexinfo[indices[i*INDEXSTRIDE+2]*VERTEXSTRIDE+1];
			vc.z = vertexinfo[indices[i*INDEXSTRIDE+2]*VERTEXSTRIDE+2];

			PrimitiveCenter[i].Center.x=factor*(va.x+vb.x+vc.x);
			PrimitiveCenter[i].Center.y=factor*(va.y+vb.y+vc.y);
			PrimitiveCenter[i].Center.z=factor*(va.z+vb.z+vc.z);
			PrimitiveCenter[i].len = max(fabs(va.x-vb.x),max(fabs(va.y-vb.y),fabs(va.z-vb.z)));
			PrimitiveCenter[i].len = max(PrimitiveCenter[i].len,max(fabs(va.x-vc.x),max(fabs(va.y-vc.y),fabs(va.z-vc.z))));
			PrimitiveCenter[i].len = max(PrimitiveCenter[i].len,max(fabs(vc.x-vb.x),max(fabs(vc.y-vb.y),fabs(vc.z-vb.z))));
		}
		minx=maxx=PrimitiveCenter[0].Center.x;
		miny=maxy=PrimitiveCenter[0].Center.y;
		minz=maxz=PrimitiveCenter[0].Center.z;
		for (int i=1;i<numprim;i++)
		{
			if (minx>PrimitiveCenter[i].Center.x)
				minx=PrimitiveCenter[i].Center.x;
			if (maxx<PrimitiveCenter[i].Center.x)
				maxx=PrimitiveCenter[i].Center.x;
			if (miny>PrimitiveCenter[i].Center.y)
				miny=PrimitiveCenter[i].Center.y;
			if (maxy<PrimitiveCenter[i].Center.y)
				maxy=PrimitiveCenter[i].Center.y;
			if (minz>PrimitiveCenter[i].Center.z)
				minz=PrimitiveCenter[i].Center.z;
			if (maxz<PrimitiveCenter[i].Center.z)
				maxz=PrimitiveCenter[i].Center.z;
		}
		/*
		sortprimsbycenterx(0,numprim-1,minx,maxx);
		sortprimsbycentery(0,numprim-1,miny,maxy);
		sortprimsbycenterz(0,numprim-1,minz,maxz);
		*/
		BuildTreeInternal(0,numprim,1);
		delete[] _sortedprimsaux;
	}

	int getnumnodes(void)
	{
		return numnodes;
	}

	const TNode *getNodes(void)
	{
		return Nodes;
	}

	~TBalancedTree()
	{
		clean();
	}
};

#endif