#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include <iostream>
#include <fstream>
#include <string>
#include <list>
#include <map>

using namespace std;


class jsonData;

class jsonData {
  public:
    string s;
    double d;
static bool debugjson;
static bool impartial;
static int nberr;

    jsonData (void) : d(0.0) {}
    jsonData (double d) : d(d) {}
    jsonData (string s) : s(s), d(0) {}

    virtual void out (ostream &cout) const = 0;

    bool operator< (jsonData const & a) const {
	if (a.d == d)
	    return (a.s < s);
	else
	    return (a.d < d);
    }
};

int ugglyglobaltab = 0;
void tabulate (ostream &cout) {
    int i;
    for (i=0 ; i<ugglyglobaltab ; i++) cout << "  ";
}

ostream& operator<< (ostream& cout, const jsonData& j) {
    j.out (cout);
    return cout;
}

class PjsonData {
    public:
	jsonData *pj;
	PjsonData(jsonData *pj) : pj(pj) {}
    bool operator< (PjsonData const & a) const {
	return (*(a.pj) < *pj);
    }
};

class jsonNumber : public jsonData {
  public:
    jsonNumber (double d) : jsonData(d) {
	if (debugjson) cerr << "jsonNumber: " << d << endl;
    }
    virtual void out (ostream &cout) const {
	cout.precision(20);
	cout << d;
    }
};

class jsonString : public jsonData {
  public:
    jsonString (string &s) : jsonData(s) {
	if (debugjson) cerr << "jsonString: " << s << endl;
    }
    virtual void out (ostream &cout) const {
	cout << '"' << s << '"';
    }
};

class jsonNull : public jsonData {
  public:
    jsonNull () {
	if (debugjson) cerr << "jsonNull" << endl;
    }
    virtual void out (ostream &cout) const {
	cout << "null";
    }
};

class jsonBoolean : public jsonData {
  public:
    bool b;
    jsonBoolean (bool b) : b(b) {
	if (debugjson) cerr << "jsonBoolean: " << (b ? "true" : "false") << endl;
    }
    virtual void out (ostream &cout) const {
	if (b)
	    cout << "true";
	else
	    cout << "false";
    }
};

class jsonBrace : public jsonData, map<PjsonData,int> {
  public:
    jsonBrace () {
	if (debugjson) cerr << "jsonBrace" << endl;
    }
    virtual void out (ostream &cout) const {
	const_iterator mi;
	cout << "{" << endl;
ugglyglobaltab++;
	for (mi=begin() ; mi!=end() ; mi++) {
tabulate(cout);
	    cout  << *(mi->first.pj);
	    const_iterator mj = mi;
	    mj++;
	    if (mj != end())
		cout << "," << endl;
	    else
		cout << endl;
	}
ugglyglobaltab--;
tabulate(cout);
	cout << "}";
    }
    void push (jsonData &j, int i) {
	insert (pair<PjsonData,int>( PjsonData(&j) , i));
    }
};

class jsonSticker  : public jsonData {
  public:
    const jsonData* pj;
    jsonSticker (string s, jsonData const& j) : jsonData(s), pj(&j) {
	if (debugjson) cerr << "jsonSticker: " << s << endl;
    }
    virtual void out (ostream &cout) const {
	cout << '"' << s << "\": " << *pj;
    }
};


class jsonList : public jsonData, list<jsonData *> {
  public:
    jsonList () {
	if (debugjson) cerr << "jsonList" << endl;
    }
    virtual void out (ostream &cout) const {
	const_iterator li;
	cout << "[" << endl;
ugglyglobaltab++;
	for (li=begin() ; li!=end() ; li++) {
tabulate(cout);
	    cout << **li;
	    const_iterator lj = li;
	    lj++;
	    if (lj != end())
		cout << "," << endl;
	    else
		cout << endl;
	}
ugglyglobaltab--;
tabulate(cout);
	cout << "]";
    }
    void push (jsonData &j) {
	push_back (&j);
    }
};


jsonData * json_parse (istream &cin) {
    jsonData *pj = NULL;
    char c;
    while (cin) {
	c = cin.get();
	if (isspace (c)) continue;
	switch (c) {
	    case '"': {
		    string s;
		    while (cin) {
// cerr << "s=" << s << endl;
			c = cin.get();
			if (c == '"') break;
			if ((c == '\\') && cin) {
			    c = cin.get();
			    s += '\\';
			    s += c;
			    continue;
			}
			s += c;
		    }
// cerr << "s=" << s << endl;
		    while (cin) {
			c = cin.get();
			if (isspace (c)) continue;
			break;
		    }
		    if (!cin) {
			pj = new jsonString (s);
cerr << "short file after string s=" << s <<endl;
jsonData::nberr ++;
			break;
		    }
		    switch (c) {
			case ':': 
			    pj = new jsonSticker(s, *json_parse (cin));
			    break;
			case '}':
			case ']':
			case ',':
			    pj = new jsonString (s);
			    cin.unget();
			    break;
			default:
			    pj = new jsonString (s);
			    cin.unget();
cerr << "dubious character \"" << c << "\" after string s=" << s << endl;
jsonData::nberr ++;
			    break;
		    }
		    
		  }
		break;
	    case '{': {
		    int i = 0;
		    jsonBrace *pjb = new jsonBrace;
		    while (cin) {
			jsonData *pj = json_parse (cin);
			if (pj != NULL) {
			    pjb->push (*pj, i++);
			}
			while (cin) {
			    c = cin.get();
			    if (!isspace (c))
				break;
			}
			if (!cin) {
cerr << "short file while in brace ({)" << endl;
jsonData::nberr ++;
			break;
			}
			if (c == ',') continue;
			if (c == '}') break;
cerr << "dubious character \"" << c << "\" while in brace ({)" << endl;
jsonData::nberr ++;
		    }
		    pj = pjb;;
		  }
		break;
	    case '[': {
		    jsonList *pjb = new jsonList;
		    while (cin) {
			jsonData *pj = json_parse (cin);
			if (pj != NULL) {
			    pjb->push (*pj);
			}
			while (cin) {
			    c = cin.get();
			    if (!isspace (c))
				break;
			}
			if (!cin) {
cerr << "short file while in brace ({)" << endl;
jsonData::nberr ++;
			    break;
			}
			if (c == ',') continue;
			if (c == ']') break;
cerr << "dubious character \"" << c << "\" while in bracket ([)" << endl;
jsonData::nberr ++;
		    }
		    pj = pjb;;
		  }
		break;

	    case 'n':	// null ?
		if (cin) {
		    c = cin.get();
		    if ((c == 'u') && cin) {
			c = cin.get();
			if ((c == 'l') && cin) {
			    c = cin.get();
			    if ((c == 'l') && cin) {
				pj = new jsonNull ();
			    }
			}
		    }
		}
		if (pj == NULL) {
cerr << "dubious char \"" << c << "\" while trying to read boolean 'true'" << endl;
jsonData::nberr ++;
		}
		break;
	    case 't':	// true ?
		if (cin) {
		    c = cin.get();
		    if ((c == 'r') && cin) {
			c = cin.get();
			if ((c == 'u') && cin) {
			    c = cin.get();
			    if ((c == 'e') && cin) {
				pj = new jsonBoolean (true);
			    }
			}
		    }
		}
		if (pj == NULL) {
cerr << "dubious char \"" << c << "\" while trying to read boolean 'true'" << endl;
jsonData::nberr ++;
		}
		break;
	    case 'f':	// false ?
		if (cin) {
		    c = cin.get();
		    if ((c == 'a') && cin) {
			c = cin.get();
			if ((c == 'l') && cin) {
			    c = cin.get();
			    if ((c == 's') && cin) {
			    c = cin.get();
				if ((c == 'e') && cin) {
				    pj = new jsonBoolean (false);
				}
			    }
			}
		    }
		}
		if (pj == NULL) {
cerr << "dubious char \"" << c << "\" while trying to read boolean 'false'" << endl;
jsonData::nberr ++;
		}
		break;
	    case ']':
	    case '}':
		cin.unget();
		return pj;
		break;
		
	    default:
		if (isdigit (c) || (c=='-') || (c=='+') || (c=='.')) {
		    string s;
		    s += c;
		    while (cin) {
			c = cin.get();
			if (isdigit (c) ||  (c=='-') || (c=='+') || (c=='.') || (c=='e')) {
			    s += c;
			    continue;
			}
			if (isspace (c)) {
			    pj = new jsonNumber (atof(s.c_str()));
			    break;
			}
			pj = new jsonNumber (atof(s.c_str()));
			switch (c) {
			    case ',':
			    case '}':
			    case ']':
				cin.unget();
				break;
			    default:
				cin.unget();
cerr << "dubious char \"" << c << "\" while trying to read number " << s << endl;
jsonData::nberr ++;
				break;
			}
			break;
		    }
		} else {
cerr << "dubious char \"" << c << "\" in the middle of nowhere" << endl;
jsonData::nberr ++;
		}
		break;
	} // switch (c)
	if (pj != NULL)
	    return pj;
    }
    return pj;
}

bool jsonData::debugjson = false;
bool jsonData::impartial = false;
bool nooutput = false;
int jsonData::nberr = 0;

int main (int nb, char ** cmde) {
    int i;
    bool nomorearg = false;

    for (i=1 ; i<nb ; i++) {
	if (cmde[i][0]!='-') continue;	// it'll be read as a file later on
	if (strcmp (cmde[i], "--") == 0) {
	    break;
	} else if ( (strcmp (cmde[i],	"-impartial") == 0) || 
		    (strcmp (cmde[i]+1,	"-impartial") == 0)
		  ) {
	    jsonData::impartial = true;
	} else if ( (strcmp (cmde[i],	"-nooutput") == 0) || 
		    (strcmp (cmde[i]+1,	"-nooutput") == 0)
		  ) {
	    nooutput = true;
	} else if ( (strcmp (cmde[i],	"-debug") == 0) || 
		    (strcmp (cmde[i]+1,	"-debug") == 0)
		  ) {
	    jsonData::debugjson = true;
	} else if ( (strcmp (cmde[i],	"-version") == 0) || 
		    (strcmp (cmde[i]+1,	"-version") == 0)
		  ) {
	    cout << "json-order 0.0.1 - (c)2013 Jean-Daniel Pauget" << endl;
	} else if ( (strcmp (cmde[i],	"-help") == 0) || 
		    (strcmp (cmde[i]+1,	"-help") == 0)
		  ) {
	    cout << "json-order [-debug] [-impartial] [-nooutput] [--] [... filenames ...]" << endl
		 << endl;
	    exit (0);
	} else {
	    cerr << "unkown option : " << cmde[i] << endl;
	}
    }

    int nbglobalerr = 0;
    int nbfile = 0;

    for (i=1 ; i<nb ; i++) {
	if (nomorearg || (cmde[i][0]!='-')) {
	    nbfile ++;
	    ifstream fin(cmde[i]);
	    if (!fin) {
		int e = errno;
		cerr << "could not open " << cmde[i] << " : " << strerror(e) << endl;
		continue;
	    }
	    jsonData::nberr = 0;
	    jsonData *pj = json_parse (fin);
	    if (pj != NULL) {
		if (!nooutput)
		    cout << *pj << endl;
	    } else {
		cerr << "could not parse anything from " << cmde[i] << endl;
		jsonData::nberr ++;
	    }
	    if (jsonData::impartial && (jsonData::nberr != 0)) {
		cerr << jsonData::nberr << " errors parsing " << cmde[i] << endl;
		nbglobalerr ++;
	    }
	} else {
	    if (strcmp (cmde[i], "--") == 0) {
		nomorearg = true;
	    }
	}
    }
    if (nbfile == 0) {
	jsonData::nberr = 0;
	jsonData *pj = json_parse (cin);
	if (pj != NULL) {
	    if (!nooutput)
		cout << *pj << endl;
	} else {
	    cerr << "could not parse anything from stdin" << endl;
	    jsonData::nberr ++;
	}
	if (jsonData::impartial && (jsonData::nberr != 0)) {
	    cerr << jsonData::nberr << " errors parsing stdin" << endl;
	    nbglobalerr ++;
	}
    }
    if (jsonData::impartial && (nbglobalerr != 0)) {
	return 1;
    }
    return 0;
}

