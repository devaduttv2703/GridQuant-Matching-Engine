#include <bits/stdc++.h>
using namespace std;
struct Order{
    string user_id;
    double price;
    double energy_amount;
    uint64_t timestamp;
    bool operator<(const Order &other) const{    
        if (price == other.price) return timestamp > other.timestamp;
        return price < other.price;
    }
};
struct AskComparator
{
    bool operator()(const Order &a, const Order &b) const{    
        if (a.price == b.price) return a.timestamp > b.timestamp;
        return a.price > b.price;
    }
};
class Solution
{
public:
    vector<string> matchOrders(vector<Order> &bids, vector<Order> &asks){    
        priority_queue<Order> bid_queue;
        priority_queue<Order, vector<Order>, AskComparator> askqueue;
        for (const auto &bid : bids)
            bid_queue.push(bid);
        for (const auto &ask : asks)
            askqueue.push(ask);
        vector<string> execution_log;
        
        while (!bid_queue.empty() && !askqueue.empty()){        
            Order best_bid = bid_queue.top();
            Order best_ask = askqueue.top();           
            if (best_bid.price < best_ask.price){            
                break;
            }            
            bid_queue.pop();
            askqueue.pop();        
            double traded_volume = min(best_bid.energy_amount, best_ask.energy_amount);           
            execution_log.push_back(best_bid.user_id + " bought " + to_string(traded_volume) +
                                    " kWh from " + best_ask.user_id + " at $" + to_string(best_ask.price));           
            best_bid.energy_amount -= traded_volume;
            best_ask.energy_amount -= traded_volume;            
            if (best_bid.energy_amount > 0) bid_queue.push(best_bid);            
            if (best_ask.energy_amount > 0) askqueue.push(best_ask);            
        }
        return execution_log;
    }
};

// main fn.
int main()
{
    vector<Order> bids = {
        {"EV_User_1", 0.15, 10.0, 100},
        {"EV_User_2", 0.18, 5.0, 101},
        {"EV_User_3", 0.15, 8.0, 102}};
    vector<Order> asks = {
        {"Solar_User_A", 0.12, 15.0, 100},
        {"Solar_User_B", 0.14, 5.0, 101},
        {"Solar_User_C", 0.12, 10.0, 102}};
    Solution sol;
    vector<string> results = sol.matchOrders(bids, asks);
    cout << "--- TRADE EXECUTION LOG ---\n";
    for (const string &trade : results){    
        cout << trade << "\n";
    }

    return 0;
}

