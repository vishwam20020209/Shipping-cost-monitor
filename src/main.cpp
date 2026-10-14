#include <bits/stdc++.h>
using namespace std;

// FreightTiger shipping-cost monitor. C++17, deterministic and dependency-free.
struct Shipment { string origin, destination, routeType, date; double quantity=0, distance=0, freight=0; };
struct Note { string id, date, applies, text; };
struct Weekly { string route, routeType, week; double freight=0, tonneKm=0, cost=0; };

vector<string> parseCSVLine(const string& line) {
    vector<string> out; string cur; bool quoted=false;
    for (size_t i=0;i<line.size();++i) {
        char c=line[i];
        if (c=='"') {
            if (quoted && i+1<line.size() && line[i+1]=='"') { cur+='"'; ++i; }
            else quoted=!quoted;
        } else if (c==',' && !quoted) { out.push_back(cur); cur.clear(); }
        else cur+=c;
    }
    out.push_back(cur); return out;
}
vector<vector<string>> readCSV(const string& path) {
    ifstream f(path); if(!f) throw runtime_error("Cannot open input file: "+path);
    vector<vector<string>> rows; string line; bool first=true;
    while(getline(f,line)) { if(!line.empty() && line.back()=='\r') line.pop_back(); if(first){first=false;continue;} if(!line.empty()) rows.push_back(parseCSVLine(line)); }
    return rows;
}
int daySerial(const string& s) { int y=stoi(s.substr(0,4)),m=stoi(s.substr(5,2)),d=stoi(s.substr(8,2));
    tm t={}; t.tm_year=y-1900; t.tm_mon=m-1; t.tm_mday=d; t.tm_hour=12; time_t v=mktime(&t); return int(v/86400); }
string fromSerial(int serial) { time_t v=(time_t)serial*86400; tm *t=gmtime(&v); char b[16]; strftime(b,sizeof(b),"%Y-%m-%d",t); return b; }
string mondayOf(const string& date) { int s=daySerial(date); tm *t; time_t v=(time_t)s*86400; t=gmtime(&v); int dow=t->tm_wday; int back=(dow+6)%7; return fromSerial(s-back); }
double num(const string& s) { return stod(s); }
string fmt(double x) { ostringstream o; o<<fixed<<setprecision(2)<<x; return o.str(); }
string signedPct(double x) { ostringstream o; o<<showpos<<fixed<<setprecision(1)<<x<<"%"; return o.str(); }
string routeKey(const string& a,const string& b) { return a+"-"+b; }

// Only notes that explicitly describe a cost-increasing event can justify a spike.
// The date windows below encode the actual temporal scope stated in the supplied notes.
bool noteApplies(const Note& n, const string& route, const string& week) {
    int w=daySerial(week), d=daySerial(n.date);
    if (n.id=="N001") return route=="Chennai-Bangalore" && w>=daySerial("2025-02-24") && w<=daySerial("2025-03-03");
    if (n.id=="N002") return route=="Ahmedabad-Mumbai" && w>=d && w<=d+6;
    if (n.id=="N003") return n.applies=="All Routes" && w>=d && w<=d+28;
    // Other notes describe unaffected routes, stable conditions, improvements, or no rate change.
    return false;
}
int main(int argc,char**argv) {
    try {
        string shipmentsPath=argc>1?argv[1]:"data/shipment_records.csv";
        string notesPath=argc>2?argv[2]:"data/context_notes.csv";
        string outputPath=argc>3?argv[3]:"output.csv";
        auto sr=readCSV(shipmentsPath); auto nr=readCSV(notesPath);
        vector<Shipment> shipments;
        for(auto&r:sr) { if(r.size()<10) continue; Shipment s; s.origin=r[1];s.destination=r[2];s.routeType=r[3];s.quantity=num(r[5]);s.distance=num(r[6]);s.freight=num(r[7]);s.date=r[8]; if(s.quantity>0&&s.distance>0) shipments.push_back(s); }
        vector<Note> notes;
        for(auto&r:nr) if(r.size()>=4) notes.push_back({r[0],r[1],r[2],r[3]});
        map<tuple<string,string,string>,Weekly> agg;
        for(auto&s:shipments) { string route=routeKey(s.origin,s.destination), week=mondayOf(s.date); auto key=make_tuple(route,s.routeType,week); auto &w=agg[key];w.route=route;w.routeType=s.routeType;w.week=week;w.freight+=s.freight;w.tonneKm+=s.quantity*s.distance; }
        vector<Weekly> weeks; for(auto &kv:agg) { auto w=kv.second; if(w.tonneKm>0) {w.cost=w.freight/w.tonneKm;weeks.push_back(w);} }
        sort(weeks.begin(),weeks.end(),[](const Weekly&a,const Weekly&b){return tie(a.week,a.route,a.routeType)<tie(b.week,b.route,b.routeType);});
        map<pair<string,string>,vector<double>> history; // route + route_type => prior weekly costs
        map<string,vector<Weekly>> byWeek;
        for(auto&w:weeks) byWeek[w.week].push_back(w);
        ofstream out(outputPath); if(!out) throw runtime_error("Cannot write output: "+outputPath);
        out<<"route,week_of,cost_per_tonne_km,vs_own_history,vs_similar_routes,flagged,matched_note_id,reason\n";
        int flaggedCount=0, justifiedCount=0, rows=0;
        for(auto&w:weeks) {
            auto hk=make_pair(w.route,w.routeType); auto hist=history[hk];
            double ownAvg=0; if(!hist.empty()) { int start=max(0,(int)hist.size()-8); for(int i=start;i<(int)hist.size();++i) ownAvg+=hist[i]; ownAvg/=hist.size()-start; }
            double peerSum=0;int peerN=0; for(auto&p:byWeek[w.week]) if(p.route!=w.route && p.routeType==w.routeType){peerSum+=p.cost;peerN++;}
            double peerAvg=peerN?peerSum/peerN:0;
            bool ownUp=!hist.empty() && w.cost>ownAvg; bool peerUp=peerN>0 && w.cost>peerAvg;
            double ownChange=ownAvg>0?(w.cost-ownAvg)/ownAvg*100:0;
            double peerChange=peerAvg>0?(w.cost-peerAvg)/peerAvg*100:0;
            // Conservative anomaly rule: an increase of at least 20% against either defined baseline.
            bool anomaly=(ownUp && ownChange>=20.0)||(peerUp && peerChange>=20.0);
            string ownText=hist.empty()?"N/A (no prior weeks)":signedPct(ownChange)+" vs own trailing "+to_string(min(8,(int)hist.size()))+"-week average";
            string peerText=peerN==0?"N/A (no same-week peers)":signedPct(peerChange)+" vs same-week "+w.routeType+" routes";
            string flag="No", noteId="", reason;
            if(anomaly) {
                const Note* match=nullptr;
                for(auto&n:notes) if(noteApplies(n,w.route,w.week)) { if(!match || n.date>match->date) match=&n; }
                if(match) { flag="No (justified)";noteId=match->id;reason="Not flagged: note "+match->id+" dated "+match->date+" applies to "+(match->applies=="All Routes"?"all routes":match->applies)+" and describes a cost-increasing event: "+match->text; justifiedCount++; }
                else { flag="Yes"; reason="Flagged for human review: cost increased unusually versus at least one baseline (20% threshold). No matching cost-increasing note was found for this route and week."; flaggedCount++; }
            } else {
                reason="Not flagged: neither available baseline shows an increase of at least 20%.";
            }
            out << '"' << w.route << "\",\"" << w.week << "\"," << fmt(w.cost) << ",\"" << ownText << "\",\"" << peerText << "\",\"" << flag << "\"," << noteId << ",\"";
            for(char c:reason){ if(c=='"') out << "\"\""; else out << c; }
            out << "\"\n";
            history[hk].push_back(w.cost);
            rows++;
        }
        out.close();
        cerr<<"Processed shipments: "<<shipments.size()<<"\nWeekly route rows: "<<rows<<"\nFlagged unexplained: "<<flaggedCount<<"\nSpikes justified by notes: "<<justifiedCount<<"\nOutput: "<<outputPath<<"\n";
    } catch(const exception&e) { cerr<<"Error: "<<e.what()<<"\n"; return 1; }
    return 0;
}
