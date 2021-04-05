#include <random>
#include <iostream>
#include <vector>
#include <tuple>
#include <set>
#include <cassert>
#include <fstream>
#include <sstream>
#include <iomanip>
using namespace std;

#define NUM_NODES 100
#define NUM_NEIGHBOURS 5

struct node {
  static int lastid;
  int id;
  node(int numn):max_nn(numn), id(++lastid) {
  }

  int max_nn;

  void recv(node* src, int ev, vector<tuple<int,node*,node*>>& net) {
	++counter_msg;
	if (seen.find(ev)!=seen.end()) {
		++counter_dup;
		return;
	}
	seen.emplace(ev);
assert(!n.empty());
	for (auto&i: n) {
		if (i==src) continue;
		net.emplace_back(make_tuple(ev, this, i));
	}
  }

  int counter_msg{0};
  int counter_dup{0};
  set<int> seen;
  vector<node*> n;

  void link(node& other) {
	if (n.size()>=max_nn) return;
	if (other.n.size()>=max_nn) return;
	n.emplace_back(&other);
	other.n.emplace_back(this);
  }

  void dump(ostream&os) const {
      os << "node #" << id << ' ' << n.size() << " n; msg " << counter_msg << " dup " << counter_dup << ".\n";
  }

};

int node::lastid{0};

struct w_t: vector<node> {
	w_t(int numnodes, int numneighbours): vector<node>(numnodes, node(numneighbours)) {

	    std::default_random_engine g;
	    std::uniform_int_distribution<size_t> d(0, size()-1);

		int i=0;
		int edges=numnodes*numneighbours;
		for (int i=0; i<edges; ++i) {
			auto n=d(g);
			auto m=d(g);
			(*this)[n].link((*this)[m]);
		}
	}

	int max_num_neighbours() const { return begin()->max_nn; }

	struct tick_data {
		int node_arrivals;
		int tot_msgs;
		int tot_dup;
		int on_wire;
		int hop;
	};
	struct tick_tm: vector<tick_data> {
		void dump(size_t sz, ostream& os) const {
			int n=0;
			os << "#hop spread transmitted dup on_wire\n";
			for (auto& i:*this) {
				++n;
				//os << "hop #" << n << ", spread " << fixed << setprecision(8) << ((i.node_arrivals*100.0)/(sz*1.0)) << "% msg transmitted: " << i.tot_msgs << " dup: " << i.tot_dup << " on wire: " << i.on_wire << "\n";
				os << n << ' ' << fixed << setprecision(8) << ((i.node_arrivals*100.0)/(sz*1.0)) << ' ' << i.tot_msgs << ' ' << i.tot_dup << ' ' << i.on_wire << "\n";
			}
//os << rbegin()->node_arrivals << ' ' << sz << endl;
//			assert(rbegin()->node_arrivals==sz);
		}
	};
	tick_tm tm;
	void dump(ostream&os) const {
		size_t totmsg=0;
		for (auto& o:*this) {
			//o.dump(os);
			totmsg+=o.counter_msg;
		}
		os << "nodes: " << size() << "; messages: " << totmsg << "; msg/node: " << (((totmsg*100.0)/size())/100.0) << "; max hops: " << hops << endl;
		ostringstream name;
		name << "hops__nodes_" << size() << "-max_neigh_" << max_num_neighbours() << ".dat";
		ofstream ofs(name.str());
		tm.dump(size(), ofs);
	}

	bool tick(int hop) {
		if (net.empty()) return false;
		tick_data td;
		td.hop=hop;
		auto net0=net;
		net.clear();
		for (auto&i:net0) {
			get<2>(i)->recv(get<1>(i), get<0>(i), net);
		}
		net0.clear();
		td.node_arrivals=0;
		td.tot_msgs=0;
		td.tot_dup=0;
		for (auto& o:*this) {
			if (!o.seen.empty()) ++td.node_arrivals;
			td.tot_msgs+=o.counter_msg;
			td.tot_dup+=o.counter_dup;
		}
		td.on_wire=net.size();
		tm.push_back(td);
		return true;
	}

	void relay(int ev) {
		tm.clear();
		net.emplace_back(make_tuple(ev, nullptr, &*begin()));
		hops=1;
		while (tick(hops)) {
			++hops;
//			cout << "tick " << i++ << endl;
		}
	}

	int hops{0};
	vector<tuple<int,node*,node*>> net;
};


void run(int nodes, int neighbours) {
	w_t w(nodes, neighbours);
	w.relay(1);
	w.dump(cout);
}


int main() {
	int n=8;
//	run(100,n);
//	run(1000,n);
	run(10000,4);
	run(10000,8);
	run(10000,16);
//	run(100000,n);
	run(1000000,4);
	run(1000000,8);
	run(1000000,16);
}
