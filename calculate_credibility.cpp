#include <bits/stdc++.h>
using namespace std;
struct Order {
    string user_id;
    double price;
    double energy_amount;
    uint64_t timestamp;
    bool operator<(const Order& other) const {
        if (price == other.price) return timestamp > other.timestamp;
        return price < other.price; 
    }
};
struct AskComparator {
    bool operator()(const Order& a, const Order& b) const {
        if (a.price == b.price) return a.timestamp > b.timestamp;
        return a.price > b.price;
    }
};
class EnergyExchange {
private:
    priority_queue<Order> ev_bids; 
    priority_queue<Order, vector<Order>, AskComparator> solar_asks;
    unordered_map<string, double> token_balances;    
    unordered_map<string, int> successful_contracts;
    unordered_map<string, int> total_contracts;    
    uint64_t current_time = 0;
    const double MIN_SAFETY_MARGIN = 2.0;
    const double DEFAULT_FLOOR_PRICE = 0.10;

public:
    double total_pooled_kw = 100.0; 
    void SubmitOrder(string user_id, double price, double amount, bool is_ev_bid) {
        current_time++;
        Order new_order = {user_id, price, amount, current_time};        
        if (is_ev_bid) {
            ev_bids.push(new_order);
        } else {
            solar_asks.push(new_order);
        }
    }
    void ProcessTelemetry(string user_id, double equity_share, double current_load) {
        double virtual_allocation = total_pooled_kw * equity_share;
        double net_surplus = virtual_allocation - current_load;        
        if (net_surplus > MIN_SAFETY_MARGIN) {
            SubmitOrder(user_id, DEFAULT_FLOOR_PRICE, net_surplus, false);
        }
    }
    vector<string> MatchOrders() {
        vector<string> execution_log;        
        while (!ev_bids.empty() && !solar_asks.empty()) {
            Order top_bid = ev_bids.top();
            Order top_ask = solar_asks.top();
            if (top_bid.price >= top_ask.price) {
                double matched_volume = min(top_bid.energy_amount, top_ask.energy_amount);
                double execution_price = (top_bid.price + top_ask.price) / 2.0;
                token_balances[top_ask.user_id] += (matched_volume * execution_price);
                token_balances[top_bid.user_id] -= (matched_volume * execution_price);
                execution_log.push_back(top_bid.user_id + " bought " + to_string(matched_volume) + 
                                        " kWh from " + top_ask.user_id + " at mid-market $" + to_string(execution_price));
                ev_bids.pop();
                solar_asks.pop();                
                if (top_bid.energy_amount > matched_volume) {
                    top_bid.energy_amount -= matched_volume;
                    ev_bids.push(top_bid);
                } else if (top_ask.energy_amount > matched_volume) {
                    top_ask.energy_amount -= matched_volume;
                    solar_asks.push(top_ask);
                }
            } else {
                break; 
            }
        }
        return execution_log;
    }
    void RecordContractHistory(string user_id, bool success) {
        total_contracts[user_id]++;
        if (success) {
            successful_contracts[user_id]++;
        }
    }
    double CalculateCredibility(string user_id) {
        int successful_deliveries = successful_contracts[user_id];
        int total_initiated = total_contracts[user_id];        
        if (total_initiated == 0) return 0.0;        
        return (static_cast<double>(successful_deliveries) / total_initiated) * 100.0;
    }
};
//main fn.
int main() {
    EnergyExchange exchange;
    exchange.RecordContractHistory("Solar_User_A", true);
    exchange.RecordContractHistory("Solar_User_A", true);
    exchange.RecordContractHistory("Solar_User_A", false);
    exchange.RecordContractHistory("Solar_User_A", true);
    exchange.RecordContractHistory("Solar_User_B", true);
    exchange.RecordContractHistory("Solar_User_B", false);
    exchange.RecordContractHistory("Solar_User_B", false);
    exchange.SubmitOrder("EV_User_1", 0.18, 20.0, true);
    exchange.SubmitOrder("EV_User_2", 0.14, 10.0, true);
    exchange.ProcessTelemetry("Solar_User_A", 0.25, 5.0);
    exchange.ProcessTelemetry("Solar_User_B", 0.10, 12.0);
    vector<string> results = exchange.MatchOrders();    
    cout << "--- TRADE EXECUTION LOG ---\n";
    for (const string& trade : results) {
        cout << trade << "\n";
    }
    cout << "\n--- CREDIBILITY SCORES ---\n";
    cout << "Solar_User_A Score: " << exchange.CalculateCredibility("Solar_User_A") << "%\n";
    cout << "Solar_User_B Score: " << exchange.CalculateCredibility("Solar_User_B") << "%\n";
    cout << "New_User Score: " << exchange.CalculateCredibility("New_User") << "%\n";
    return 0;
}

