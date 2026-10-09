#include <bits/stdc++.h>
using namespace std;
struct Order{
    string user_id;
    double price;
    double energy_amount;
    uint64_t timestamp;
    bool operator<(const Order &other) const{
        if (price == other.price)
            return timestamp > other.timestamp;
        return price < other.price;
    }
};
struct AskComparator{
    bool operator()(const Order &a, const Order &b) const{
        if (a.price == b.price)
            return a.timestamp > b.timestamp;
        return a.price > b.price;
    }
};
class EnergyExchange{
private:
    priority_queue<Order> ev_bids;
    priority_queue<Order, vector<Order>, AskComparator> solar_asks;
    unordered_map<string, double> token_balances;
    uint64_t current_time = 0;
    const double MIN_SAFETY_MARGIN = 2.0;
    const double DEFAULT_FLOOR_PRICE = 0.10;

public:
    double total_pooled_kw = 100.0;
    void SubmitOrder(string user_id, double price, double amount, bool is_ev_bid){
        current_time++;
        Order new_order = {user_id, price, amount, current_time};
        if (is_ev_bid){
            ev_bids.push(new_order);
        }
        else{
            solar_asks.push(new_order);
        }
    }
    void ProcessTelemetry(string user_id, double equity_share, double current_load){
        double virtual_allocation = total_pooled_kw * equity_share;
        double net_surplus = virtual_allocation - current_load;
        if (net_surplus > MIN_SAFETY_MARGIN){
            SubmitOrder(user_id, DEFAULT_FLOOR_PRICE, net_surplus, false);
        }
    }
    vector<string> MatchOrders(){
        vector<string> execution_log;
        while (!ev_bids.empty() && !solar_asks.empty()){
            Order best_bid = ev_bids.top();
            Order best_ask = solar_asks.top();
            if (best_bid.price < best_ask.price){
                break;
            }
            ev_bids.pop();
            solar_asks.pop();
            double traded_volume = min(best_bid.energy_amount, best_ask.energy_amount);
            double total_cost = traded_volume * best_ask.price;
            token_balances[best_bid.user_id] -= total_cost;
            token_balances[best_ask.user_id] += total_cost;
            execution_log.push_back(best_bid.user_id + " bought " + to_string(traded_volume) +
                                    " kWh from " + best_ask.user_id + " for $" + to_string(total_cost));
            best_bid.energy_amount -= traded_volume;
            best_ask.energy_amount -= traded_volume;
            if (best_bid.energy_amount > 0) ev_bids.push(best_bid);
            if (best_ask.energy_amount > 0) solar_asks.push(best_ask);
        }
        return execution_log;
    }
};
// main fn.

int main()
{
    EnergyExchange exchange;
    exchange.SubmitOrder("EV_User_1", 0.15, 20.0, true);
    exchange.SubmitOrder("EV_User_2", 0.12, 10.0, true);
    exchange.ProcessTelemetry("Solar_User_A", 0.25, 5.0);
    exchange.ProcessTelemetry("Solar_User_B", 0.10, 12.0);
    exchange.ProcessTelemetry("Solar_User_C", 0.40, 15.0);
    vector<string> results = exchange.MatchOrders();
    for (const string &trade : results){
        cout << trade << "\n";
    }
    return 0;
}

