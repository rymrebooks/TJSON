/*
Target design:
	Utf8 only.
	Fast, stablity
	Long time excutation for server side.
	General/flexible purpose in use.
	Only string storage for all type values.
	Float point, number, boolean are used as realm/context.
Author: Trinhdc
Email:  rymrebooks@gmail.com
Donate  @ paypal: rymrebooks@gmail.com 
License: BSD
Version: 1.0.0.0
RoadMap:
	More test to get bugs.
	Listen to users to improve
Bugs:
	+
	+
*/
#ifndef _Use_TJson
#define _Use_TJson
//#define _DebugOnFlying
#define IsResvrJsonToken(x) (x == '"' || x == '\'' || x == ':' || x == ',' || x == '[' || x == ']' || x == '{' || x == '}' )
#define IsStartArrOrObjToken(x) (x == '{' || x == '[') 
#define IsEndArrOrObjToken(x) (x == '}' || x == ']') 
#define IsNewItemToken(x) (x == ',' || x == '{' || x == '[') 
#define IsOpenStrTK(x) (x == '"' || x == '\'')
#define IsSpliters(x) (x == ':' || x == ',')

const int m_iInitMemorySizeForAllocator = 64 * 1024;// 1000000; 64*1024
const int m_iInitMemSizeForArrayObjects = 5;
enum TJsonTag : char {
	JVL_ISKEY = 0,
	JVL_NUMBER,
	JVL_STRING,
	JVL_ARRAY,
	JVL_OBJECT,
	JVL_TRUE,
	JVL_FALSE,
	JVL_NUMBERORBOOL,
	JVL_NUMBERORBOOLORNULL,
	JVL_NULL
};

char *TJsonTag_s[] =
{
	" Tag: JVL_ISKEY", //JVL_ISKEY
	" Tag: JVL_NUMBER", //JVL_NUMBER
	" Tag: JVL_STRING", //JVL_STRING
	" Tag: JVL_ARRAY", //JVL_ARRAY
	" Tag: JVL_OBJECT", //JVL_OBJECT
	" Tag: JVL_TRUE", //JVL_TRUE
	" Tag: JVL_FALSE", //JVL_FALSE
	" Tag: JVL_NUMBERORBOOL", //JVL_NUMBERORBOOL
	" Tag: JVL_NUMBERORBOOLORNULL", //JVL_NUMBERORBOOLORNULL
	" Tag: JVL_NULL", //JVL_NULL
};

void PrintTagName(TJsonTag oTag) {
	cout << TJsonTag_s[oTag];
	/*switch (oTag)
	{
	case JVL_ISKEY:
		cout << " Tag: JVL_ISKEY";
		break;
	case JVL_NUMBER:
		cout << " Tag: JVL_NUMBER";
		break;
	case JVL_STRING:
		cout << " Tag: JVL_STRING";
		break;
	case JVL_ARRAY:
		cout << " Tag: JVL_ARRAY";
		break;
	case JVL_OBJECT:
		cout << " Tag: JVL_OBJECT";
		break;
	case JVL_TRUE:
		cout << " Tag: JVL_TRUE";
		break;
	case JVL_FALSE:
		cout << " Tag: JVL_FALSE";
		break;
	case JVL_NUMBERORBOOL:
		cout << " Tag: JVL_NUMBERORBOOL";
		break;
	case JVL_NUMBERORBOOLORNULL:
		cout << " Tag: JVL_NUMBERORBOOLORNULL";
		break;
	case JVL_NULL:
		cout << " Tag: JVL_NULL";
		break;
	default:
		cout << " Tag: TJsonTag UNKNOW!";
		break;
	}*/
}

enum TJsonParseRet : char {
	JSP_SUCC = 0,        // Parse is successful. Orthers are as belows
	JSP_MemPa,            // Mem for parsing
	JSP_FALSE_,            // General parsing
	JSP_FALSE_FM,        // Format issue
	JSP_PARSE_ER        // Parsing fires issue
};


struct TAllocator {
	int nRetry;
	int iBuffResizing;
	int iAllocations, iDeallocations;
	int Count, Size, m_sgSize; //sgSize is suggestion size - when parse without enoung memory it is store new suggested memory.
	char *cBuffer;

	void AllocCtor() {
		m_sgSize = 0;
		iBuffResizing = 0;
		iAllocations = 0;
		iDeallocations = 0;
		Count = 0;
		Size = 0;
		nRetry = 10;
		cBuffer = nullptr;
	}

	TAllocator(int iInitializedSize) {
		AllocCtor();
		IsValidMem(iInitializedSize);
	}

	TAllocator() {
		AllocCtor();
		IsValidMem(0);
	}

	~TAllocator() {
		if (cBuffer != nullptr) {
			delete[]cBuffer;
		}
	}

	void Prints() {
		cout << "\n@TAllocator, iBuffResizing: " << iBuffResizing << " iAllocations: " << iAllocations << " iDeallocations: " << iDeallocations << " Count: " << Count << " Size: " << Size;
	}

	void DoAllMemBySuggestion() {
		if (m_sgSize > 0) {
			if (cBuffer != nullptr)
				delete[]cBuffer;
			int _sgSize = m_sgSize;
			AllocCtor();
			m_sgSize = _sgSize;
			cBuffer = new char[m_sgSize];
			Size = m_sgSize;
			nRetry--;
		}
	}

	void DoAllMemBySuggestion(int iSuggestedSize) {
		m_sgSize = iSuggestedSize;
		DoAllMemBySuggestion();
	};

	bool IsValiadMemByPlusMoreContentInson(const char *chStringJson, int &iSuggestedSize) {
		int istrLen = strlen(chStringJson);				// Returns the length of the C string str.
		int iCountKeys = CountKeysIn(chStringJson, istrLen);
		int iExpectNewMem = istrLen + (iCountKeys * 64); // 64 managed bytes per node.
		int iNeedMems = Count + iExpectNewMem;
		iSuggestedSize = iNeedMems + iNeedMems / 2;
		if (iNeedMems < Size) {
			return true;
		}
		else
			return false;
	}

	int CountKeysIn(const char *chStringJson, int istrLen) {
		int i = 0, iKeyCount = 0;
		char clastchar = 0;
		for (i = 1; i < istrLen; i++) {
			if (clastchar == '"' && chStringJson[i] == ':')
				iKeyCount++;
			clastchar = chStringJson[i];
		}
		return iKeyCount;
	}

	void Reset() {
		Count = 0;
	}

	char *Allocate(int iAllSize) {
		if (IsValidMem(iAllSize)) {
			iAllocations++;
			char *cOut = &cBuffer[Count];
			Count += iAllSize;
			return cOut;
		}
		else {
			return nullptr;
		}
	}

	void DeAllocate() {
		iDeallocations++;
	}

	bool IsValidMem(int iAllSize) {
		if ((Count + iAllSize) < Size)
			return true;
		else {
			if (cBuffer != nullptr) {
				m_sgSize = 2 * Size;
				return false;
			}
			else {
				int iNewSize = Size > 0 ? (Size + Size / 2) + iAllSize : m_iInitMemorySizeForAllocator + iAllSize;
				char *pBuffTmp = new char[iNewSize];
				if (pBuffTmp == nullptr)
					return false;
				if (cBuffer != nullptr) {
					for (int i = 0; i < Count; i++)
						pBuffTmp[i] = cBuffer[i];
					delete[]cBuffer;
				}
				cBuffer = pBuffTmp;
				Size = iNewSize;
				iBuffResizing++;
			}
		}
		return true;
	}

};

struct TNodeData {
	TJsonTag Tag;
	char *Key, *Value;
	int ItemIDCnt, Size;
};

#ifndef _Use_MyQsortUtils
#define _Use_MyQsortUtils
inline bool FirstIsLessThanLast(const char *strFirst, const char *strLast) { // a < b
	int i = 0;
	if (strFirst != nullptr && strLast != nullptr) {
		while ((strFirst[i] == strLast[i]) && (strFirst[i] != 0) && (strLast[i] != 0)) {
			i++;
		}

		if ((strFirst[i] == 0) || (strLast[i] == 0)) {
			if ((strFirst[i] == 0) && (strLast[i] != 0)) {
				return true;
			}
			return false;
		}
		else {
			return strFirst[i] < strLast[i];	// > 
		}
	}
	return false;
}

inline bool FirstIsGreaterThanLast(const char *strFirst, const char *strLast) {// a > b
	int i = 0;
	if (strFirst != nullptr && strLast != nullptr) {
		while ((strFirst[i] == strLast[i]) && (strFirst[i] != 0) && (strLast[i] != 0)) {
			i++;
		}

		if ((strFirst[i] == 0) || (strLast[i] == 0)) {
			if ((strFirst[i] != 0) && (strLast[i] == 0)) {
				return true;
			}
			return false;
		}
		else {
			return strFirst[i] > strLast[i];		// >
		}
	}
	return false;
}

inline bool IsEqualStr(const char *strSrc, const char *strDes) {
	int i = 0;
	if (strSrc != nullptr && strDes != nullptr) {
		while ((strSrc[i] == strDes[i]) && (strSrc[i] != 0) && (strDes[i] != 0)) {
			i++;
		}

		if ((strSrc[i] == 0) && (strDes[i] == 0))
			return true;
		else
			return false;
	}
	return false;
}

int Compare(const void* a, const void* b) {
	const TNodeData* arg1 = static_cast<const TNodeData*>(a);
	const TNodeData* arg2 = static_cast<const TNodeData*>(b);

	if (FirstIsLessThanLast((const char *)arg1->Key, (const char *)arg2->Key))    return -1;// if (arg1 < arg2) return -1; a < b
	if (FirstIsGreaterThanLast((const char *)arg1->Key, (const char *)arg2->Key)) return 1; // if (arg1 > arg2) return 1;  a > b
	return 0;
}

// https://stackoverflow.com/questions/22504837/how-to-implement-quick-sort-algorithm-in-c
template<typename T, typename compare = std::less<T>>
void q_sort(T **input, int l_idx, int r_idx, compare comp = compare()) {

	if (l_idx >= r_idx)
		return;

	// The below is the partition block (can be made a sub function)

	int left = l_idx;
	int right = r_idx;
	{
		int pivot_idx = l_idx;
		T *pivot = input[pivot_idx];

		while (left < right) {
			while (FirstIsLessThanLast(input[left]->Key, pivot->Key))	// Expect < from comp(input[left], pivot) @ use struct, must implement operator < 
				left++;
			while (FirstIsLessThanLast(pivot->Key, input[right]->Key)) // Expect < from comp(input[left], pivot)
				right--;
			swap(input[left], input[right]);
		}

		swap(pivot, input[left]);
	}

	q_sort(input, l_idx, left, comp);
	q_sort(input, left + 1, r_idx, comp);

}

template<typename T, typename compare = std::less<T>>
void quick_sort(T **array, int N, compare comp = compare()) {
	q_sort(array, 0, N - 1, comp);

}

int SearchNode(const char *cKey, TNodeData **arr, int n) {
	int first, last, middle;
	first = 0;
	last = n - 1;
	middle = (first + last) / 2;
	while (first <= last)
	{
		if (FirstIsLessThanLast(arr[middle]->Key, cKey))
		{
			first = middle + 1;

		}
		else if (IsEqualStr(arr[middle]->Key, cKey))
		{
			//cout << search << " found at location " << middle + 1 << "\n";
			break;
		}
		else
		{
			last = middle - 1;
		}
		middle = (first + last) / 2;
	}

	if (first > last)
	{
		return -1;
	}
	return middle;
}

#endif

struct TJSNode : public TNodeData {

	TJSNode **Nodes;

	TJSNode() {
		Ctor();
	}
	
	TJSNode &operator[](const char *cKey) {
		return *GetNodeByKey(cKey);
	}

	void Ctor() {
		ItemIDCnt = 0;
		Size = 0;
		Tag = JVL_NULL;
		Key = nullptr;
		Value = nullptr;
		Nodes = nullptr;
	}

	TJSNode &operator=(const TJSNode &oTJSNode) {
		CopyFrom(oTJSNode);
		return *this;
	}

	void CopyFrom(const TJSNode &oTJSNode) {
		ItemIDCnt = oTJSNode.ItemIDCnt;
		Size = oTJSNode.Size;
		Tag = oTJSNode.Tag;
		Key = oTJSNode.Key;
		Value = oTJSNode.Value;
		Nodes = oTJSNode.Nodes;
	}

	bool ValidatMem(TAllocator &oTAllocator) {
		if (ItemIDCnt + 1 >= Size) {
			//cout << " Cnt: " << ItemIDCnt << "/" << Size << "|" << oTAllocator.Count << "|" << oTAllocator.Size;
			int iNewSize;
			TJSNode **NodesTmp = 0;
			if (oTAllocator.Count + 100 >= oTAllocator.Size)
				iNewSize = oTAllocator.Count;
			if (Size == 0) {
				iNewSize = m_iInitMemSizeForArrayObjects;
				NodesTmp = (TJSNode**)oTAllocator.Allocate(iNewSize * sizeof(void*));
				if (NodesTmp == nullptr)
					return false;
			}
			else {
				iNewSize = Size + Size / 2;
				NodesTmp = (TJSNode**)oTAllocator.Allocate(iNewSize * sizeof(void*));
				if (NodesTmp == nullptr)
					return false;
			}

			if (NodesTmp != nullptr) {
				for (int i = 0; i < iNewSize; i++) {
					NodesTmp[i] = (TJSNode*)oTAllocator.Allocate(sizeof(TJSNode));
					if (NodesTmp[i] == nullptr)
						return JSP_MemPa;
					NodesTmp[i]->Ctor();
				}

				if (Nodes != nullptr) {
					for (int i = 0; i < ItemIDCnt; i++) {
						NodesTmp[i]->CopyFrom(*Nodes[i]);
					}
					//delete[]Nodes;
				}

				Nodes = NodesTmp;
				Size = iNewSize;
				//Return false to resize?
			}
			else
				return false;
		}
		return ItemIDCnt < Size;
	}

	// Numbers, boolean or null value
	inline bool IsNotString(const char *cInput, int s, int e) {
		while (cInput[s] == ' ') s++;
		//while (cInput[e] == ' ') e--;
		if (e - s >= 1)
			return !IsNumberOrBooleanOrNull(&cInput[s]);
		else
			return false;
	}

	inline bool IsNumberOrBooleanOrNull(const char *cInput) {
		//cout << "\ncInput: " << cInput;
		int s = 0, iDot = 0, ic; while (cInput[s] == ' ') s++;

		// Check for null
		if ((cInput[s] == 'n' && cInput[s + 1] == 'u' && cInput[s + 2] == 'l' && cInput[s + 3] == 'l' && (cInput[s + 4] == 0 || cInput[s + 4] == ' ')))
			return true;

		// Check for true
		if ((cInput[s] == 't' && cInput[s + 1] == 'r' && cInput[s + 2] == 'u' && cInput[s + 3] == 'e' && (cInput[s + 4] == 0 || cInput[s + 4] == ' ')))
			return true;

		// Check for false
		if ((cInput[s] == 'f' && cInput[s + 1] == 'a' && cInput[s + 2] == 'l' && cInput[s + 3] == 's' && cInput[s + 4] == 'e' && (cInput[s + 5] == 0 || cInput[s + 5] == ' ')))
			return true;

		// Check for number
		if (cInput[s] == '-') {
			iDot = 0; ic = s + 1;
		}
		else {
			iDot = 0; ic = s;
		}

		//int js = s; while (cInput[js] != ' ' && cInput[js] != 0 && !IsResvrJsonToken(cInput[js])) js++;
		while (cInput[ic] != 0) {
			if ((cInput[ic] == ' ' || cInput[ic] == '\t' || cInput[ic] == '\n' || cInput[ic] == '\r')) {
				ic++;
				continue;
			}
			if (((cInput[ic] < '0') || (cInput[ic] > '9')) && (cInput[ic] != '.'))
				return false;
			iDot += cInput[ic] == '.' ? 1 : 0;
			if (iDot > 1)
				return false;
			ic++;
		}
		return true;
	}

	inline bool IsEmpty(const char *cInput, int s, int e) {
		while (s < e) {
			if (cInput[s] != ' ')
				return false;
			s++;
		}
		return true;
	}

#ifdef _DebugOnFlying
	inline bool ObtainKey(TAllocator &oTAllocator, const char *chStringJson, int iLastTokenPos, int iCrrPos, string &strSpace) {
#else
	inline bool ObtainKey(TAllocator &oTAllocator, const char *chStringJson, int iLastTokenPos, int iCrrPos) {
#endif
		int j = 0;
		if (!ValidatMem(oTAllocator))
			return false;
		char *cKeyTmp = oTAllocator.Allocate(iCrrPos - iLastTokenPos);
		if (cKeyTmp != nullptr) {
			for (int i = iLastTokenPos + 1; i < iCrrPos; i++)
				cKeyTmp[j++] = chStringJson[i];
			cKeyTmp[j++] = 0;
			Nodes[ItemIDCnt]->Key = cKeyTmp;
			Nodes[ItemIDCnt]->Tag = JVL_ISKEY;
#ifdef _DebugOnFlying
			cout << "\n    " << strSpace << "@" << Nodes[ItemIDCnt]->Key;
#endif
			return true;
		}
		else
			return false;
	}

#ifdef _DebugOnFlying
	inline bool ObtainValue(TAllocator &oTAllocator, const char *chStringJson, int iLastTokenPos, int iCrrPos, string strPrefix) {
#else
	inline bool ObtainValue(TAllocator &oTAllocator, const char *chStringJson, int iLastTokenPos, int iCrrPos) {
#endif
		if (!ValidatMem(oTAllocator))
			return JSP_MemPa;
		char *cValueTmp = oTAllocator.Allocate(iCrrPos - iLastTokenPos);
		if (cValueTmp != nullptr) {
			int j = 0;

			int st = iLastTokenPos + 1; while (st < iCrrPos &&
				(chStringJson[st] == ' ' || chStringJson[st] == '\n' || chStringJson[st] == '\r' || chStringJson[st] == '\t'))
				st++;
			int en = iCrrPos;			while (en > iLastTokenPos &&
				(chStringJson[en] == ' ' || chStringJson[en] == '\n' || chStringJson[en] == '\r' || chStringJson[en] == '\t'))
				en--;

			for (int i = st; i < en; i++)
				cValueTmp[j++] = chStringJson[i];
			cValueTmp[j++] = 0;
			Nodes[ItemIDCnt]->Value = cValueTmp;
			Nodes[ItemIDCnt]->Tag = IsNumberOrBooleanOrNull(cValueTmp) ? JVL_NUMBERORBOOLORNULL : JVL_STRING;
#ifdef _DebugOnFlying
			cout << strPrefix << Nodes[ItemIDCnt]->Value;
#endif
			return true;
		}
		else
			return false;
	}

#ifdef _DebugOnFlying
	inline TJsonParseRet RecursiveParse(int &iPos, int &iLastTokenPos, const char *chStringJson, TAllocator &oTAllocator, char cCrrContainer, string strSpace) {
#else
	inline TJsonParseRet RecursiveParse(int &iPos, int &iLastTokenPos, const char *chStringJson, TAllocator &oTAllocator, char cCrrContainer) {
#endif
		ItemIDCnt = 0;
		int j;// , iLastTokenPos = iPos - 1;
		char cStrOpen, cLastChar = 0, cLastSplitter = 0, cNextJsonToken;
		while (chStringJson[iPos] != 0) {
			if (cLastChar != '\\' && IsResvrJsonToken(chStringJson[iPos])) {
				if (IsStartArrOrObjToken(chStringJson[iPos])) {
					if (!ValidatMem(oTAllocator))
						return JSP_MemPa;
					cCrrContainer = chStringJson[iPos];
					cLastSplitter = 0;
#ifdef _DebugOnFlying
					cout << "\n" << strSpace + "    !" << chStringJson[iPos];
					iLastTokenPos = iPos++;
					Nodes[ItemIDCnt]->Tag = cCrrContainer == '{' ? JVL_OBJECT : JVL_ARRAY;
					if (Nodes[ItemIDCnt++]->RecursiveParse(iPos, iLastTokenPos, chStringJson, oTAllocator, cCrrContainer, strSpace + "    ") != JSP_SUCC)
#else
					iLastTokenPos = iPos++;
					Nodes[ItemIDCnt]->Tag = cCrrContainer == '{' ? JVL_OBJECT : JVL_ARRAY;
					if (Nodes[ItemIDCnt++]->RecursiveParse(iPos, iLastTokenPos, chStringJson, oTAllocator, cCrrContainer) != JSP_SUCC)
#endif
						return JSP_PARSE_ER;
					cCrrContainer = 0;
				}
				else if (IsOpenStrTK(chStringJson[iPos])) {
					cStrOpen = chStringJson[iPos];
					iLastTokenPos = iPos++;
					while (chStringJson[iPos] != 0 && (chStringJson[iPos] != cStrOpen || (chStringJson[iPos] == cStrOpen && chStringJson[iPos - 1] == '\\'))) iPos++;
					j = iPos + 1;
					while (chStringJson[j] != 0 && !IsResvrJsonToken(chStringJson[j])) j++;

					cNextJsonToken = chStringJson[j];
#ifdef _DebugOnFlying
					if (cNextJsonToken == ':') {
						if (!ObtainKey(oTAllocator, chStringJson, iLastTokenPos, iPos, strSpace))
							return JSP_MemPa;
					}
					else {
						if (!ObtainValue(oTAllocator, chStringJson, iLastTokenPos, iPos, "#"))
							return JSP_MemPa;
						ItemIDCnt++;
					}
#else
					if (cNextJsonToken == ':') {
						if (!ObtainKey(oTAllocator, chStringJson, iLastTokenPos, iPos))
							return JSP_MemPa;
					}
					else {
						if (!ObtainValue(oTAllocator, chStringJson, iLastTokenPos, iPos))
							return JSP_MemPa;
						ItemIDCnt++;
					}
#endif
					iLastTokenPos = iPos;
					cLastSplitter = 0;
				}
				else if (IsEndArrOrObjToken(chStringJson[iPos])) {
					if (cLastSplitter == ':') {
#ifdef _DebugOnFlying
						if (!ObtainValue(oTAllocator, chStringJson, iLastTokenPos, iPos, "$"))
#else
						if (!ObtainValue(oTAllocator, chStringJson, iLastTokenPos, iPos))
#endif
							return JSP_MemPa;
						ItemIDCnt++;
					}
					else {
						//*
						if (cCrrContainer == '[') {
							if (IsNotString(chStringJson, iLastTokenPos + 1, iPos)) {
#ifdef _DebugOnFlying
								if (!ObtainValue(oTAllocator, chStringJson, iLastTokenPos, iPos, "$"))
#else
								if (!ObtainValue(oTAllocator, chStringJson, iLastTokenPos, iPos))
#endif
									return JSP_MemPa;
								ItemIDCnt++;
							}
						}
						//*/
					}
#ifdef _DebugOnFlying
					cout << "\n" << strSpace << chStringJson[iPos] << "<-" << iPos << "," << oTAllocator.Count << "," << oTAllocator.Size;
#endif
					iLastTokenPos = iPos;
					cLastSplitter = 0;
					return JSP_SUCC;
				}
				else if (chStringJson[iPos] == ',' && cLastSplitter == ':') {
#ifdef _DebugOnFlying
					if (!ObtainValue(oTAllocator, chStringJson, iLastTokenPos, iPos, "&"))
#else
					if (!ObtainValue(oTAllocator, chStringJson, iLastTokenPos, iPos))
#endif
						return JSP_MemPa;
#ifdef _DebugOnFlying
					cout << chStringJson[iPos] << " ";
#endif
					ItemIDCnt++;
				}
				else if (IsSpliters(chStringJson[iPos])) {
#ifdef _DebugOnFlying
					cout << chStringJson[iPos] << " ";
#endif
					//*
					if (cCrrContainer == '[') {
						if (IsNotString(chStringJson, iLastTokenPos + 1, iPos)) {
#ifdef _DebugOnFlying
							if (!ObtainValue(oTAllocator, chStringJson, iLastTokenPos, iPos, "$"))
#else
							if (!ObtainValue(oTAllocator, chStringJson, iLastTokenPos, iPos))
#endif
								return JSP_MemPa;
							ItemIDCnt++;
						}
					}
					//*/
					cLastSplitter = chStringJson[iPos];
					iLastTokenPos = iPos;
				}
			}
			cLastChar = chStringJson[iPos];
			iPos++;
		}
		return JSP_SUCC;
	}

	void Print(string strDeepth) {
		string strMore = "";
		if (Value != nullptr || (Tag == JVL_OBJECT || Tag == JVL_ARRAY)) strMore = ": ";
		if (Key != nullptr) cout << "\n" << strDeepth << '"' << Key << '"' << strMore;
		if (Value != nullptr) {
			if (Tag == JVL_NUMBERORBOOL || Tag == JVL_NUMBERORBOOLORNULL) {
				cout << Value;
			}
			else {
				cout << '"' << Value << '"';
			}
		}

		if (Tag == JVL_OBJECT) cout << "\n" << strDeepth << "{";
		if (Tag == JVL_ARRAY) cout << "\n" << strDeepth << "[";

		if (Tag == JVL_OBJECT || Tag == JVL_ARRAY) {
			for (int i = 0; i < ItemIDCnt; i++) {
				Nodes[i]->Print(strDeepth + "    ");
				if (i < ItemIDCnt - 1)
					cout << ", ";
			}
		}

		if (Tag == JVL_ARRAY)  cout << "\n" << strDeepth << "]";
		if (Tag == JVL_OBJECT) cout << "\n" << strDeepth << "}";

	}

	void Stringify(char *cOutputBuff, int &iOutSize) {
		if (Key != nullptr) {
			cOutputBuff[iOutSize++] = '"';
			int i = 0;
			while (Key[i] != 0)
				cOutputBuff[iOutSize++] = Key[i++];
			cOutputBuff[iOutSize++] = '"';
			if (Value != nullptr || (Tag == JVL_OBJECT || Tag == JVL_ARRAY)) {
				cOutputBuff[iOutSize++] = ':';
				cOutputBuff[iOutSize++] = ' ';
			}
		}

		if (Tag == JVL_OBJECT) cOutputBuff[iOutSize++] = '{';
		if (Tag == JVL_ARRAY) cOutputBuff[iOutSize++] = '[';

		if (Value != nullptr) {
			int i = 0;
			if (Tag != JVL_NUMBERORBOOL && Tag != JVL_NUMBERORBOOLORNULL)
				cOutputBuff[iOutSize++] = '"';
			while (Value[i] != 0)
				cOutputBuff[iOutSize++] = Value[i++];
			if (Tag != JVL_NUMBERORBOOL && Tag != JVL_NUMBERORBOOLORNULL)
				cOutputBuff[iOutSize++] = '"';
		}

		if (Tag == JVL_OBJECT || Tag == JVL_ARRAY) {
			for (int i = 0; i < ItemIDCnt; i++) {
				Nodes[i]->Stringify(cOutputBuff, iOutSize);
				if (i < ItemIDCnt - 1) {
					cOutputBuff[iOutSize++] = ',';
					cOutputBuff[iOutSize++] = ' ';
				}
			}
		}

		if (Tag == JVL_OBJECT) cOutputBuff[iOutSize++] = '}';
		if (Tag == JVL_ARRAY) cOutputBuff[iOutSize++] = ']';
	}

	inline bool IsExistKey(const char *cKey, int &iLocation) {
		/*
		for (int i = 0; i < ItemIDCnt; i++) {
		if (IsEqualStr(Nodes[i]->Key, cKey)) {
		iLocation = i;
		return true;
		}
		}
		//*/

		iLocation = SearchNode(cKey, (TNodeData **)Nodes, ItemIDCnt);
		if (iLocation >= 0)
			return true;
		return false;
	}

	bool IsExistKey(const char *cKey) {
		int iLocation = 0;
		return IsExistKey(cKey, iLocation);
	}

	char *GetStrValueOf(const char *cKey) {
		int iLocation = 0;
		if (IsExistKey(cKey, iLocation)) {
			return Nodes[iLocation]->Value;
		}
		else {
			return nullptr;
		}
	}

	TJSNode *GetNodeByKey(const char *cKey) {
		int iLocation = 0;
		if (IsExistKey(cKey, iLocation)) {
			return Nodes[iLocation];
		}
		return nullptr;
	}

	bool AddKey(TAllocator &oTAllocator, const char *cKey) {
		if (ValidatMem(oTAllocator)) {
			int i, iSz = 0; while (cKey[iSz++] != 0);
			char *cKeyTmp = oTAllocator.Allocate(iSz);
			if (cKeyTmp != nullptr) {
				for (i = 0; i < iSz; i++)
					cKeyTmp[i] = cKey[i];
				cKeyTmp[i] = 0;
				Nodes[ItemIDCnt]->Key = cKeyTmp;
				Nodes[ItemIDCnt]->Tag = JVL_ISKEY;
				return true;
			}
		}
		return false;
	}

	bool AddValue(TAllocator &oTAllocator, const char *cValue) {
		if (ValidatMem(oTAllocator)) {
			int i, iSz = 0; while (cValue[iSz++] != 0);
			char *ccValueTmp = oTAllocator.Allocate(iSz);
			if (ccValueTmp != nullptr) {
				for (i = 0; i < iSz; i++)
					ccValueTmp[i] = cValue[i];
				ccValueTmp[i] = 0;
				Nodes[ItemIDCnt]->Value = ccValueTmp;
				Nodes[ItemIDCnt]->Tag = JVL_STRING;
				return true;
			}
		}
		return false;
	}

	bool UpdateValue(TAllocator &oTAllocator, const char *cValue) {
		if (ValidatMem(oTAllocator)) {
			int i, iSz = 0; while (cValue[iSz++] != 0);
			char *ccValueTmp = oTAllocator.Allocate(iSz);
			if (ccValueTmp != nullptr) {
				for (i = 0; i < iSz; i++)
					ccValueTmp[i] = cValue[i];
				ccValueTmp[i] = 0;
				Value = ccValueTmp;
				Tag = JVL_STRING;
				return true;
			}
		}
		return false;
	}

	bool AddArray(TAllocator &oTAllocator, const char *cKey) {
		if (AddKey(oTAllocator, cKey)) {
			Nodes[ItemIDCnt]->Tag = JVL_ARRAY;
			ItemIDCnt++;
			return true;
		}
		else
			return false;
	}

	bool AddObject(TAllocator &oTAllocator, const char *cKey) {
		if (AddKey(oTAllocator, cKey)) {
			Nodes[ItemIDCnt]->Tag = JVL_OBJECT;
			ItemIDCnt++;
			return true;
		}
		else
			return false;
	}

	bool AddItem(TAllocator &oTAllocator, const char *cKey, const char *cValue) {
		if (AddKey(oTAllocator, cKey)) {
			AddValue(oTAllocator, cValue);
			ItemIDCnt++;
			return true;
		}
		else
			return false;
	}

	bool AddArrItem(TAllocator &oTAllocator, const char *cKey) {
		if (AddKey(oTAllocator, cKey)) {
			ItemIDCnt++;
			return true;
		}
		else
			return false;
	}

	void RemoveItem(const char *cKey) {
		int iLocation = 0;
		if (IsExistKey(cKey, iLocation)) {
			for (int i = iLocation; i < ItemIDCnt; i++) {
				Nodes[i] = Nodes[i + 1];
			}
			ItemIDCnt--;
		}
	}

	void QSortKeys() {//TJSNode **Nodes
		if (Tag == JVL_OBJECT && ItemIDCnt > 0) {
			int low = 0, high = ItemIDCnt - 1;
			quick_sort<TNodeData>((TNodeData**)Nodes, ItemIDCnt);
			for (int i = 0; i < ItemIDCnt; i++) {
				if (Nodes[i]->Tag == JVL_OBJECT) {
					Nodes[i]->QSortKeys();
				}
			}
		}
	}

	};

struct TJSON {
	TJSNode *Root;
	TJSNode *m_pTJSNode;
	TAllocator m_TAllocator;

	TJSNode *operator[](const char *cKey) {
		return nullptr;// m_pTJSNode[cKey];
	}

	TJSON() {
		m_CurrNodeDepthID = 0;
		m_pTJSNode = new TJSNode();
		Root = m_pTJSNode;
		m_pTJSNode->Ctor();
	}

	void BackToRoot() {
		m_CurrNodeDepthID = 0;
		m_pTJSNode = Root->Nodes[0];
	}


	TJsonParseRet RootParse(const char *chStringJson) {
		//First parse 
		int iSuggestedSize;
		TryMemorySpace(chStringJson, iSuggestedSize);

		int iPos = 0, iLastTokenPos = 0;
		char cCrrContainer = 0;
		if(m_pTJSNode != Root)
			m_pTJSNode = Root;

		m_pTJSNode->Ctor();
		m_pTJSNode->Tag = JVL_OBJECT;
		m_TAllocator.Reset();
		m_CurrNodeDepthID = 0;


#ifdef _DebugOnFlying
		string strSpace = "";
		TJsonParseRet oRet = m_pTJSNode->RecursiveParse(iPos, chStringJson, m_TAllocator, cCrrContainer, strSpace);
		if (m_pTJSNode->ItemIDCnt > 0) {
			m_pTJSNode = m_pTJSNode->Nodes[0];
			return oRet;
		}
		else
			return JSP_PARSE_ER;
#else
		while (m_TAllocator.nRetry > 0) {
			TJsonParseRet oRet = m_pTJSNode->RecursiveParse(iPos, iLastTokenPos, chStringJson, m_TAllocator, cCrrContainer);
			if (oRet == JSP_SUCC) {
				m_pTJSNode = m_pTJSNode->Nodes[0];
				m_pTJSNode->QSortKeys();
				return oRet;
			}
			else {
				m_TAllocator.Reset();
				iPos = 0;
				iLastTokenPos = 0;
				cCrrContainer = 0;
				m_pTJSNode->Ctor();
				m_pTJSNode->Tag = JVL_OBJECT;
				m_TAllocator.DoAllMemBySuggestion();
			}
		}
		return JSP_PARSE_ER;
#endif
	}

	TJsonParseRet TryMemorySpace(const char *chStringJson, int &iSuggestedSize) {
		if (!m_TAllocator.IsValiadMemByPlusMoreContentInson(chStringJson, iSuggestedSize)) {
			char *cOutputBuff = new char[m_TAllocator.Count * 2]; // m_TAllocator.Count recently content buffera
			if (cOutputBuff != nullptr) {
				int iOutSize = 0;
				int iCurrNodeDepthIDBk = m_CurrNodeDepthID;
				if (m_CurrNodeDepthID > 0) {
					BackToRoot();
					Stringify(cOutputBuff, iOutSize);
				}

				m_TAllocator.DoAllMemBySuggestion(iSuggestedSize);

				if (iCurrNodeDepthIDBk > 0)
				{
					RootParse(cOutputBuff);
					//Restore current node;
					while (m_CurrNodeDepthID < iCurrNodeDepthIDBk) {
						if (strlen(m_TrackCrrNodes[m_CurrNodeDepthID]) > 0) {
							TJSON *TmpNode = GetNodeByKey(m_TrackCrrNodes[m_CurrNodeDepthID]);
							//cout << "Back to: " << m_TrackCrrNodes[m_CurrNodeDepthID];
							m_CurrNodeDepthID++;
						}
					}
				}
				delete[]cOutputBuff;
			}
			else
				return JSP_PARSE_ER;
		}
		return JSP_SUCC;
	}

	//for current node inserts new tree node .... from chStringJson
	TJsonParseRet CrrNodeParse(const char *cKey, const char *chStringJson) {
		m_TAllocator.Prints();
		int iSuggestedSize = 0;
		TryMemorySpace(chStringJson, iSuggestedSize);

		if (m_TAllocator.IsValiadMemByPlusMoreContentInson(chStringJson, iSuggestedSize)) {
			int iPos = 0, iLastTokenPos = 0;
			char cCrrContainer = 0;
			TJSNode *pCrrNode = m_pTJSNode;
			m_pTJSNode = AddObject(cKey);
			TJsonParseRet oRet = m_pTJSNode->RecursiveParse(iPos, iLastTokenPos, chStringJson, m_TAllocator, cCrrContainer);
			m_pTJSNode = pCrrNode;
			m_TAllocator.Prints();
			return oRet;
		}
		return JSP_PARSE_ER;
	}

	void Stringify(char *cOutputBuff, int &iOutSize) {
		iOutSize = 0;
		m_pTJSNode->Stringify(cOutputBuff, iOutSize);
		cOutputBuff[iOutSize++] = 0;
	}

	
	TJSON &operator=(TJSON &oValue) {
		m_pTJSNode = oValue.m_pTJSNode;
		return *this;
	}

	char *GetStrValueOf(const char *cKey) {
		int iLocation = 0;
		if (m_pTJSNode->IsExistKey(cKey, iLocation)) {
			return m_pTJSNode->Nodes[iLocation]->Value;
		}
		else {
			return nullptr;
		}
	}

	int GetItemCount() {
		return m_pTJSNode->ItemIDCnt;
	}

	char *GetStrKeyOfItemID(int iItemid) {
		if (iItemid < m_pTJSNode->ItemIDCnt) {
			return m_pTJSNode->Nodes[iItemid]->Key;
		}
		else {
			return nullptr;
		}
	}

	char *GetStrValueOfItemID(int iItemid) {
		if (iItemid < m_pTJSNode->ItemIDCnt) {
			return m_pTJSNode->Nodes[iItemid]->Value;
		}
		else {
			return nullptr;
		}
	}

	TJsonTag *GetTypeValueOfItemID(int iItemid) {
		if (iItemid < m_pTJSNode->ItemIDCnt) {
			return &m_pTJSNode->Nodes[iItemid]->Tag;
		}
		else {
			return nullptr;
		}
	}

	TJsonTag *GetTypeValueOf(const char *cKey) {
		int iLocation = 0;
		if (m_pTJSNode->IsExistKey(cKey, iLocation)) {
			return &m_pTJSNode->Nodes[iLocation]->Tag;
		}
		else {
			return nullptr;
		}
	}

	void SetStrValueForItemID(int iItemid, const char *cNewValue) {
		if (iItemid < m_pTJSNode->ItemIDCnt) {
			m_pTJSNode->Nodes[iItemid]->UpdateValue(m_TAllocator, cNewValue); //cNewValue
		}
	}

	// SetStrValueForItemID SetStrValueForKey
	void SetStrValueForKey(const char *cKey, const char *cNewValue) {
		int iLocation = 0;
		if (m_pTJSNode->IsExistKey(cKey, iLocation)) {
			m_pTJSNode->Nodes[iLocation]->UpdateValue(m_TAllocator, cNewValue); //cNewValue
		}
	}

	int m_CurrNodeDepthID;
	char m_TrackCrrNodes[64][64];//
	TJSON *GetNodeByKey(const char *cKey) {
		int iLocation = 0;
		if (m_pTJSNode->IsExistKey(cKey, iLocation)) {
			TrackTheNodeByKey(m_CurrNodeDepthID++, cKey);
			m_pTJSNode = m_pTJSNode->Nodes[iLocation];
			return this;
		}
		return nullptr;
	}

	void TrackTheNodeByKey(int iCurrNodeDepthID, const char *cKey) {
		int i = 0;
		while (cKey[i] != 0 && i < 64) {
			m_TrackCrrNodes[iCurrNodeDepthID][i] = cKey[i];
			i++;
		}
		m_TrackCrrNodes[iCurrNodeDepthID][i] = 0;

	}

	bool IsExistKey(const char *cKey) {
		return m_pTJSNode->IsExistKey(cKey);
	}

	void Prints() {
		cout << "\n\nTJSNode: \n";
		m_pTJSNode->Print("");
		cout << "\n\nm_TAllocator: \n";
		m_TAllocator.Prints();
	}

	void Print() {
		m_pTJSNode->Print("");
	}

	void AddArray(const char *cKey) {
		m_pTJSNode->AddArray(m_TAllocator, cKey);
	}

	TJSNode * AddObject(const char *cKey) {
		m_pTJSNode->AddObject(m_TAllocator, cKey);
		return m_pTJSNode->Nodes[m_pTJSNode->ItemIDCnt - 1];
	}

	void AddArrItem(const char *cKey) {
		m_pTJSNode->AddArrItem(m_TAllocator, cKey);
	}

	void AddItem(const char *cKey, const char *cValue) {
		m_pTJSNode->AddItem(m_TAllocator, cKey, cValue);
	}

	void RemoveItem(const char *cKey) {
		m_pTJSNode->RemoveItem(cKey);
	}
};

#ifndef _GetJsonFromFile
#define _GetJsonFromFile
std::string GetJsonFromFile(std::string strfile, bool isShow = false) {
	std::string strJson = "", strJsonTmp = "";
	std::ifstream myfile(strfile);
	std::string line;
	if (myfile.is_open())
	{
		std::cout << "\n\nTest file, wait for reading in file: " << strfile;
		while (std::getline(myfile, line))
		{
			strJson += line;
		}
		myfile.close();
		char cLast = 0;
		strJsonTmp = "";
		for (size_t i = 0; i < strJson.length(); i++) {
			if (strJson[i] != ' ' || cLast != ' ') {
				strJsonTmp += strJson[i];
			}
			cLast = strJson[i];
		}
		strJson = strJsonTmp;
		std::cout << "\nstring json size: " << strlen(strJson.c_str());
		if (isShow) {
			std::cout << "\nInput json:\n" << strJson.c_str() << "\n";
		}
	}
	else {
		std::cout << "Unable to open file: " << strfile;
	}
	return strJson;
}
#endif

//char cOutPut[1000000];

void Test_TJS_Node_JS(string &strSortJson, bool isManual = true) {
	//std::string strSortJson = GetJsonFromFile("e:\\SortJson.js", false);

	//std::string strSortJson = GetJsonFromFile("c:/canada.json", false);
	//std::string strSortJson = GetJsonFromFile("c:/citm_catalog.json", false); 


	//std::string strSortJson = GetJsonFromFile("c:/IssueParser.js", true);

	//std::string strSortJson = GetJsonFromFile("e:\\Special.js", false);
	//std::string strSortJson = GetJsonFromFile("e:\\SortJson_tmp.js", false);
	//std::string strSortJson = GetJsonFromFile("e:\\SortJson.js", false);



	int iSize = 0;
	TJSON oTJSON;
	TJsonParseRet oParseRs = oTJSON.RootParse(strSortJson.c_str());
	//if (!isManual)
	//	return;
	auto *cOutPut = new char[8000000];
	string strOut, strOut2;
	TJSON oTJSON21;
	if (oParseRs == JSP_SUCC) {
		std::cout << "\n\nParsing is sucessful. -----------------------------------------------------------------------------------------------------";
		/*
		std::cout << "\nOutput:\n"; oTJSON.Prints();
		/*/
		int iOutSize = 0;
		oTJSON.Stringify(cOutPut, iOutSize);
		strOut = cOutPut;
		oTJSON21.RootParse(strOut.c_str());
		oTJSON21.Stringify(cOutPut, iOutSize);
		strOut2 = cOutPut;
		if (strOut2 != strOut) {
			ofstream myfile;
			myfile.open("strOut.js", ios::out | ios::trunc | ios::binary);
			myfile.write(strOut.c_str(), strOut.length());
			myfile.close();

			myfile.open("strOut2.js", ios::out | ios::trunc | ios::binary);
			myfile.write(strOut2.c_str(), strOut2.length());
			myfile.close();
			cout << " Recompare is FALSE;";
		}
		else {
			cout << " Recompare is TRUE;";
			goto clean;

		}
		//cout << "\ncOutPut: \n" << cOutPut << "\niOutSize: " << iOutSize;
		cout << "\ncOutPut: ...\n" << "\niOutSize: " << iOutSize;
		oTJSON.m_TAllocator.Prints();
		//*/
		string strInput, strKey, strValue;
		TJSON *oCrrNode = &oTJSON;
		//char strViewBuff[10000];
		while (1) {
			cout << "\n\n[q] -> quit";
			cout << " [p] -> prints";
			cout << " [e] -> check exit key of crr node";
			cout << " [v] -> value of a key";
			cout << "\n[t] -> type of node by key";
			cout << " [c] -> count of item in crr node";
			cout << " [u] -> value of a element.";
			cout << " [d] -> update value of key or id";
			cout << "\n[y] -> plus more json for crr node";
			cout << " [g] -> get node with key";
			cout << " [s] -> show stringify";
			cout << " [f] -> save to file";
			cout << "\n[r] -> Restore Root";
			cout << " [a] -> add array";
			cout << " [o] -> add object";
			cout << " [i] -> add key/value";
			cout << "\n[m] -> remove item.";
			cout << " Enter str: ";
			cin >> strInput;
			if (strInput == "[q]") {
				break;
			}
			if (strInput == "[p]") {
				oCrrNode->Print();
			}
			else if (strInput == "[e]") {
				cout << "\n            Enter key: "; cin >> strInput;
				if (oCrrNode->IsExistKey(strInput.c_str())) {
					cout << "\n            The key is existing.";
				}
				else {
					cout << "\n            The key dont existing.";
				}
			}
			else if (strInput == "[v]") {
				cout << "\n            Enter key: "; cin >> strInput;
				if (oCrrNode->IsExistKey(strInput.c_str())) {
					char *cValue = oCrrNode->GetStrValueOf(strInput.c_str());
					TJsonTag *myType = oCrrNode->GetTypeValueOf(strInput.c_str());
					if (cValue != nullptr) {
						cout << "\n            -> " << cValue; PrintTagName(*myType);
					}
					else {
						cout << "\n            The key's cValue is null."; PrintTagName(*myType);
					}
				}
				else {
					cout << "\n            The key dont existing.";
				}
			}
			else if (strInput == "[d]") { // SetStrValueForKey
				cout << "\n            Update value by key(y/n): "; cin >> strInput;
				if (strInput == "y") {
					cout << "\n            Enter key: "; cin >> strKey;
					cout << "\n            Enter value: "; cin >> strValue;
					oCrrNode->SetStrValueForKey(strKey.c_str(), strValue.c_str());
				}
				else { // SetStrValueForItemID 
					int iItemPos = 0;
					cout << "\n            Items Count: " << oCrrNode->GetItemCount();
					cout << "\n            Enter item pos: "; cin >> iItemPos;
					cout << "\n            Enter value: "; cin >> strValue;
					oCrrNode->SetStrValueForItemID(iItemPos, strValue.c_str());
				}
			}
			else if (strInput == "[t]") {
				cout << "\n            Enter key: "; cin >> strInput;
				if (oCrrNode->IsExistKey(strInput.c_str())) {
					TJsonTag *myType = oCrrNode->GetTypeValueOf(strInput.c_str());
					if (myType != nullptr) {
						cout << "\n            -> "; PrintTagName(*myType);
					}
					else {
						cout << "\n            The key's cValue's type is null."; PrintTagName(*myType);
					}
				}
				else {
					cout << "\n            The key dont existing.";
				}
			}
			else if (strInput == "[c]") {
				cout << "\n            Items Count: " << oCrrNode->m_pTJSNode->ItemIDCnt;
			}
			else if (strInput == "[u]") {
				int iItemPos = 0;
				cout << "\n            Items Count: " << oCrrNode->GetItemCount();
				cout << "\n            Enter item pos: "; cin >> iItemPos;
				if (iItemPos < oCrrNode->GetItemCount()) {
					char *ckey = oCrrNode->GetStrKeyOfItemID(iItemPos);
					char *cValue = oCrrNode->GetStrValueOfItemID(iItemPos);
					TJsonTag *myType = oCrrNode->GetTypeValueOfItemID(iItemPos);
					string strkey = ""; if (ckey) strkey = ckey;
					string strvalue = cValue; if (cValue) strvalue = cValue;
					cout << "\n            The Key: '" << strkey << "' Value: '" << strvalue << "'"; PrintTagName(*myType);
				}
				else {
					cout << "\n            Item pos must < Items Count.";
				}

			}
			else if (strInput == "[g]") {
				cout << "\n            Enter key: "; cin >> strInput;
				auto xNode = oCrrNode->GetNodeByKey(strInput.c_str());
				if (xNode != nullptr) {
					cout << "\n            The key is existing. Now your working node is changed";
					oCrrNode = xNode;
				}
				else {
					cout << "\n            The key dont existing. Plz try-again.";
				}
			}
			else if (strInput == "[s]") {
				oCrrNode->Stringify(cOutPut, iSize);
				cout << "\nStringify, iSize: " << iSize << ":\n" << cOutPut;
			}
			else if (strInput == "[r]") {
				//oCrrNode->m_pTJSNode = oCrrNode->Root->Nodes[0];
				oCrrNode->BackToRoot();
			}
			else if (strInput == "[a]") {
				cout << "\n            Enter key: "; cin >> strKey;
				oCrrNode->AddArray(strKey.c_str());
			}
			else if (strInput == "[o]") {
				cout << "\n            Enter key: "; cin >> strKey;
				oCrrNode->AddObject(strKey.c_str());
			}
			else if (strInput == "[t]") {
				cout << "\n            Enter key: "; cin >> strKey;
				oCrrNode->AddArrItem(strKey.c_str());
			}
			else if (strInput == "[i]") {
				cout << "\n            Enter key:   "; cin >> strKey;
				cout << "\n            Enter value: "; cin >> strValue;
				oCrrNode->AddItem(strKey.c_str(), strValue.c_str());
			}
			else if (strInput == "[m]") {
				cout << "\n            Enter key: "; cin >> strKey;
				oCrrNode->RemoveItem(strKey.c_str());
			}
			else if (strInput == "[f]") {
				cout << "\n            Enter file name: "; cin >> strKey;
				oCrrNode->Stringify(cOutPut, iSize);
				ofstream myfile;
				myfile.open(strKey.c_str(), ios::out | ios::trunc | ios::binary);
				myfile.write(cOutPut, iSize);
				myfile.close();
			}
			else if (strInput == "[y]") { // CrrNodeParse
										  //In reality usage, user should check mem before insert new object
				cout << "\n            Enter key:   ";			cin >> strKey;
				//cout << "\n            Enter json object: ";	cin >> strValue;
				strValue = "{\"type\": \"number\", \"description\": \"Product identifier\"}";
				oCrrNode->CrrNodeParse(strKey.c_str(), strValue.c_str());

				//m_iInitMemorySizeForAllocator == 1000000 m_iInitMemSizeForArrayObjects
				//@TAllocator, iBuffResizing: 1 iAllocations: 136 iDeallocations: 0 Count: 2540 Size: 1000000
				//@TAllocator, iBuffResizing: 1 iAllocations : 153 iDeallocations : 0 Count : 2871 Size : 1000000
			}
		}
	}
	else {
		std::cout << "\nParsing is falsed.";
	}
clean:
	delete[]cOutPut;
};

void Test_TJS_Node(string strFile, bool isManual = true) {
	std::string strSortJson = GetJsonFromFile(strFile);
	Test_TJS_Node_JS(strSortJson, isManual);
}

#endif